#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "anomaly_detector.h"
#include "data_parser.h"

extern std::unordered_map<std::string, SymbolState> bySymbol;
extern std::deque<Anomaly> recentAnomalies;
extern std::mutex stateMutex;
extern std::unordered_set<std::string> trackedSymbols;
extern std::mutex subscriptionMutex;
extern std::condition_variable subscriptionCv;
