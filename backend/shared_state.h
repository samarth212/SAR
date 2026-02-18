#pragma once
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

#include "anomaly_detector.h"
#include "data_parser.h"

extern std::unordered_map<std::string, SymbolState> bySymbol;
extern std::deque<Anomaly> recentAnomalies;
extern std::mutex stateMutex;
