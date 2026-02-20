#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "anomaly.h"
#include "api.h"
#include "data_parser.h"
#include "socket.h"

std::unordered_map<std::string, SymbolState> bySymbol;
std::deque<Anomaly> recentAnomalies;
std::mutex stateMutex;