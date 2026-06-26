#include "api.h"

#include <algorithm>
#include <cctype>
#include <vector>

static std::string normalize_symbol(std::string symbol) {
    symbol.erase(std::remove_if(symbol.begin(), symbol.end(),
                                [](unsigned char ch) { return std::isspace(ch); }),
                 symbol.end());

    std::transform(symbol.begin(), symbol.end(), symbol.begin(),
                   [](unsigned char ch) { return std::toupper(ch); });

    return symbol;
}

static bool is_valid_symbol(const std::string &symbol) {
    if (symbol.empty() || symbol.size() > 10)
        return false;

    return std::all_of(symbol.begin(), symbol.end(), [](unsigned char ch) {
        return std::isalnum(ch) || ch == '.' || ch == '-';
    });
}

static json symbols_json(const std::unordered_set<std::string> &symbols) {
    std::vector<std::string> sorted(symbols.begin(), symbols.end());
    std::sort(sorted.begin(), sorted.end());

    json out = json::array();
    for (const auto &symbol : sorted)
        out.push_back(symbol);

    return out;
}

static http::response<http::string_body>
handle_request(const http::request<http::string_body> &req) {
    // CORS so React can call
    auto make_json = [&](http::status st, json body) {
        http::response<http::string_body> res{st, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set("Access-Control-Allow-Origin", "*");
        res.set("Access-Control-Allow-Headers", "Content-Type");
        res.set("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
        res.body() = body.dump();
        res.prepare_payload();
        return res;
    };

    if (req.method() == http::verb::options) {
        return make_json(http::status::ok, json{{"ok", true}});
    }

    std::string path = std::string(req.target());

    if (path == "/api/health" && req.method() == http::verb::get) {
        return make_json(http::status::ok, json{{"ok", true}});
    }

    if (path == "/api/tickers" && req.method() == http::verb::get) {
        json out = json::array();
        {
            std::lock_guard<std::mutex> lock(stateMutex); // acquire lock
            for (auto const &pair : bySymbol)
                out.push_back(pair.first);
        }
        return make_json(http::status::ok, out);
    }

    if (path == "/api/tickers/tracked" && req.method() == http::verb::put) {
        json body;
        try {
            body = json::parse(req.body());
        } catch (const json::parse_error &) {
            return make_json(http::status::bad_request, json{{"error", "invalid JSON"}});
        }

        if (!body.is_array()) {
            return make_json(http::status::bad_request, json{{"error", "expected ticker array"}});
        }

        std::unordered_set<std::string> nextTrackedSymbols;
        for (const auto &item : body) {
            if (!item.is_string()) {
                return make_json(http::status::bad_request,
                                 json{{"error", "ticker symbols must be strings"}});
            }

            std::string symbol = normalize_symbol(item.get<std::string>());
            if (!is_valid_symbol(symbol)) {
                return make_json(http::status::bad_request,
                                 json{{"error", "invalid ticker symbol"}, {"symbol", symbol}});
            }

            nextTrackedSymbols.insert(symbol);
        }

        {
            std::lock_guard<std::mutex> lock(subscriptionMutex);
            trackedSymbols = nextTrackedSymbols;
        }
        subscriptionCv.notify_all();

        return make_json(http::status::ok, json{{"tracked", symbols_json(nextTrackedSymbols)}});
    }

    if (path == "/api/anomalies" && req.method() == http::verb::get) {
        json out = json::array();
        {
            std::lock_guard<std::mutex> lock(stateMutex); // acquire locks
            for (auto const &a : recentAnomalies) {
                out.push_back({{"type", static_cast<int>(a.type)},
                               {"source", static_cast<int>(a.source)},
                               {"direction", static_cast<int>(a.direction)},
                               {"symbol", a.symbol},
                               {"timestamp", a.timestamp},
                               {"value", a.value},
                               {"mean", a.mean},
                               {"stdev", a.stdev},
                               {"zscore", a.zscore},
                               {"lower", a.lower},
                               {"upper", a.upper},
                               {"k", a.k},
                               {"note", a.note}});
            }
        }
        return make_json(http::status::ok, out);
    }

    if (req.method() != http::verb::get) {
        return make_json(http::status::method_not_allowed, json{{"error", "method not allowed"}});
    }

    return make_json(http::status::not_found, json{{"error", "not found"}});
}

static void do_session(tcp::socket socket) {
    beast::flat_buffer buffer;
    http::request<http::string_body> req;

    beast::error_code ec;
    http::read(socket, buffer, req, ec);
    if (ec)
        return;

    auto res = handle_request(req);
    http::write(socket, res, ec);

    socket.shutdown(tcp::socket::shutdown_send, ec);
}

void run_http_server(unsigned short port) {
    net::io_context ioc;
    tcp::acceptor acceptor{ioc, {tcp::v4(), port}};

    for (;;) {
        tcp::socket socket{ioc};
        acceptor.accept(socket);
        do_session(std::move(socket));
    }
}
