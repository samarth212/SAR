#include "socket.h"

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

static void record_anomaly(const Anomaly &anomaly) {
    constexpr std::size_t MAX_RECENT_ANOMALIES = 100;

    std::cout << anomaly.note << "\n";
    recentAnomalies.push_back(anomaly);
    if (recentAnomalies.size() > MAX_RECENT_ANOMALIES) {
        recentAnomalies.pop_front();
    }
}

// helper method to handle env vars
static std::string getenv_or_throw(const char *name) {
    const char *v = std::getenv(name);
    if (!v || !*v)
        throw std::runtime_error(std::string("Missing env var: ") + name);
    return std::string(v);
}

static std::unordered_set<std::string>
symbols_difference(const std::unordered_set<std::string> &left,
                   const std::unordered_set<std::string> &right) {
    std::unordered_set<std::string> result;
    for (const auto &symbol : left) {
        if (!right.contains(symbol))
            result.insert(symbol);
    }
    return result;
}

template <typename WebSocket>
static void write_subscription_message(WebSocket &ws, std::mutex &writeMutex,
                                       const std::string &action,
                                       const std::unordered_set<std::string> &symbols) {
    if (symbols.empty())
        return;

    std::vector<std::string> sorted(symbols.begin(), symbols.end());
    std::sort(sorted.begin(), sorted.end());

    json symbolArray = json::array();
    for (const auto &symbol : sorted)
        symbolArray.push_back(symbol);

    const std::string message = json{{"action", action},
                                     {"trades", symbolArray},
                                     {"quotes", symbolArray},
                                     {"bars", symbolArray}}
                                    .dump();

    std::lock_guard<std::mutex> lock(writeMutex);
    ws.write(net::buffer(message));
}

int run_socket() {
    try {

        const std::string key = getenv_or_throw("APCA_API_KEY_ID");
        const std::string secret = getenv_or_throw("APCA_API_SECRET_KEY");

        // test stream
        const std::string host = "stream.data.alpaca.markets"; // alpaca server we are connecting to
        const std::string port = "443"; // the secure port used for encrypted connections
        const std::string target = "/v2/iex";

        // object that runs all network work for this program
        net::io_context ioc;
        // object holds the rules for making a secure connection as a client
        ssl::context ctx{ssl::context::tls_client};

        // This tells the program to trust the normal certificate list on the computer
        ctx.set_default_verify_paths();

        // This tells the program to check the server identity before sending secrets
        ctx.set_verify_mode(ssl::verify_peer);

        // create socket
        websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

        // This object helps find the server address to connect to
        tcp::resolver resolver{ioc};
        auto const results =
            resolver.resolve(host, port); // find the server address which matches the host and port

        // connect TCP first so the SSL handshake has a valid socket
        net::connect(ws.next_layer().next_layer(), results.begin(), results.end());

        // This sets the server name so the secure connection is made to the right site.
        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str())) {
            // This builds an error code if setting the server name fails.
            beast::error_code ec{static_cast<int>(::ERR_get_error()),
                                 net::error::get_ssl_category()};
            // This stops with a readable error.
            throw beast::system_error{ec};
        }

        // start TLS handshake with server
        ws.next_layer().handshake(ssl::stream_base::client);

        // start the WebSocket session on top of the encrypted connection
        ws.handshake(host, target);

        // buffer holds incoming messages from the server
        beast::flat_buffer buffer;

        // reads the first server message that says we are connected
        ws.read(buffer);

        // print to terminal
        // std::cout << beast::make_printable(buffer.data()) << "\n";

        // This clears the buffer so we can reuse it.
        buffer.consume(buffer.size());

        // logs in over the WebSocket using the key and secret
        std::string auth_msg =
            std::string(R"({"action":"auth","key":")") + key + R"(","secret":")" + secret + R"("})";
        // sends the login message to Alpaca
        ws.write(net::buffer(auth_msg));

        // reads Alpaca’s response to the login
        ws.read(buffer);

        // std::cout << beast::make_printable(buffer.data()) << "\n";

        buffer.consume(buffer.size());

        std::atomic_bool subscriptionsRunning{true};
        std::mutex writeMutex;

        std::thread subscriptionThread([&] {
            std::unordered_set<std::string> subscribedSymbols;

            while (subscriptionsRunning.load()) {
                std::unordered_set<std::string> desiredSymbols;
                {
                    std::unique_lock<std::mutex> lock(subscriptionMutex);
                    subscriptionCv.wait(lock, [&] {
                        return !subscriptionsRunning.load() || trackedSymbols != subscribedSymbols;
                    });

                    if (!subscriptionsRunning.load())
                        return;

                    desiredSymbols = trackedSymbols;
                }

                const auto symbolsToSubscribe =
                    symbols_difference(desiredSymbols, subscribedSymbols);
                const auto symbolsToUnsubscribe =
                    symbols_difference(subscribedSymbols, desiredSymbols);

                try {
                    write_subscription_message(ws, writeMutex, "subscribe", symbolsToSubscribe);
                    write_subscription_message(ws, writeMutex, "unsubscribe",
                                               symbolsToUnsubscribe);
                    subscribedSymbols = desiredSymbols;
                } catch (const std::exception &e) {
                    std::cerr << "Subscription error: " << e.what() << "\n";
                    subscriptionsRunning.store(false);
                    subscriptionCv.notify_all();
                    return;
                }
            }
        });

        subscriptionCv.notify_all();

        auto stop_subscription_thread = [&] {
            subscriptionsRunning.store(false);
            subscriptionCv.notify_all();
            if (subscriptionThread.joinable())
                subscriptionThread.join();
        };

        // keep reading updates forever

        try {
            for (;;) {
                // read next message from the stream
                ws.read(buffer);

                // Convert to string
                std::string message = beast::buffers_to_string(buffer.data());
                buffer.consume(buffer.size());

                auto events = parseMessage(message);

                {
                    std::lock_guard<std::mutex> lock(stateMutex);
                    updateState(bySymbol, events);

                    std::unordered_set<std::string> changed;
                    changed.reserve(events.size());
                    for (const auto &ev : events)
                        changed.insert(ev.symbol);

                    for (const auto &symbol : changed) {
                        if (auto a = detectPriceAnomaly(symbol, bySymbol, 2.0)) {
                            record_anomaly(*a);
                        }
                        if (auto a = detectSpreadAnomaly(symbol, bySymbol, 2.0)) {
                            record_anomaly(*a);
                        }
                        if (auto a = detectVolumeAnomaly(symbol, bySymbol, 2.0)) {
                            record_anomaly(*a);
                        }
                    }
                }
            }
        } catch (...) {
            stop_subscription_thread();
            throw;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
