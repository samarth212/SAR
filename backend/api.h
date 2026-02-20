#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include "anomaly_detector.h"
#include "shared_state.h"
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
handle_request(const http::request<http::string_body> &req);

static void do_session(tcp::socket socket);

void run_http_server(unsigned short port);