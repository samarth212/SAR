#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "anomaly_detector.h"
#include "api.h"
#include "data_parser.h"
#include "socket.h"

std::unordered_map<std::string, SymbolState> bySymbol;
std::deque<Anomaly> recentAnomalies;
std::mutex stateMutex;

int main() {
    std::thread apiThread([] { run_http_server(8080); });
    apiThread.detach();

    run_socket();

    return 0;
}