#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include "anomaly_detector.h"
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

extern std::unordered_map<std::string, SymbolState> bySymbol;
extern std::deque<Anomaly> recentAnomalies; // keep last N anomalies
extern std::mutex stateMutex;

static http::response<http::string_body>
handle_request(const http::request<http::string_body> &req) {
    // CORS so React can call
    auto make_json = [&](http::status st, json body) {
        http::response<http::string_body> res{st, req.version()};
        res.set(http::field::content_type, "application/json");
        res.set("Access-Control-Allow-Origin", "*");
        res.set("Access-Control-Allow-Headers", "Content-Type");
        res.set("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.body() = body.dump();
        res.prepare_payload();
        return res;
    };

    if (req.method() == http::verb::options) {
        return make_json(http::status::ok, json{{"ok", true}});
    }

    if (req.method() != http::verb::get) {
        return make_json(http::status::method_not_allowed, json{{"error", "GET only"}});
    }

    std::string path = std::string(req.target());

    if (path == "/api/health") {
        return make_json(http::status::ok, json{{"ok", true}});
    }

    if (path == "/api/symbols") {
        json out = json::array();
        {
            std::lock_guard<std::mutex> lock(stateMutex); // acquire lock
            for (auto const &pair : bySymbol)
                out.push_back(pair.first);
        }
        return make_json(http::status::ok, out);
    }

    if (path == "/api/anomalies") {
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
