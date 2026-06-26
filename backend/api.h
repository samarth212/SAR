#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include <condition_variable>
#include "anomaly_detector.h"
#include "shared_state.h"
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

extern std::unordered_map<std::string, SymbolState> bySymbol;
extern std::deque<Anomaly> recentAnomalies; // keep last N anomalies
extern std::mutex stateMutex;
extern std::unordered_set<std::string> trackedSymbols;
extern std::mutex subscriptionMutex;
extern std::condition_variable subscriptionCv;

void run_http_server(unsigned short port);
