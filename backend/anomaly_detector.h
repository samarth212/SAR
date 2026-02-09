#pragma once

#include "data_parser.h"
#include <iostream>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <deque>
#include <optional>
#include <cmath>
#include <numeric>

/*

Maybe keep the last 20-50 data points so you can calculate averages.
Step 3: Calculate What's "Normal"
For each new piece of data, calculate:

Average price of recent trades 
Average volume of recent trades
Standard deviation (how spread out the prices usually are)

Think of standard deviation like this: if stock prices are usually between $100-$102, that's low deviation. If they bounce between $95-$110, that's high deviation.
Step 4: Detect Anomalies
Compare the new data point to your calculated "normal":
Example for a price surge:

If new price > (average price + 2×standard deviation), that's unusual!
Or simpler: if new price is 5% higher than the recent average

Example for volume surge:

If new volume is 3× the average volume, something's happening!

Step 5: Decide What to Do
When you detect an anomaly:

Print an alert to the console
Log it to a file
Maybe track how long the anomaly lasts


*/

enum class AnomalyType {
    Price,
    Volume,
    Spread,
    Volatility,
    Range,
    Gap,
    Liquidity,
    StaleData,
    ParseError
};

enum class SourceType { Trade, Quote, Bar };
enum class Direction { Up, Down, None };

struct Anomaly{
    AnomalyType type = AnomalyType::Price;
    SourceType source = SourceType::Trade;
    Direction direction = Direction::None;

    std::string symbol;
    std::string timestamp;     
    //std::int64_t ts_ns = 0;   

    // for why it triggered can be used later
    double value = 0.0;        // observed value (price, volume, spread, etc.)
    double mean = 0.0;         // baseline average
    double stdev = 0.0;        // baseline std dev
    double zscore = 0.0;       // (value - mean) / stdev when stdev > 0

    double lower = 0.0;        // mean - k*stdev
    double upper = 0.0;        // mean + k*stdev
    double k = 0.0;            // how many std devs you used

    std::string note;          // message
};

double averagePriceOfRecentTrades(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol);

std::int64_t averageVolumeOfRecentTrades(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol);

std::optional<Anomaly> detectPriceAnomaly(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol, double k);

std::optional<Anomaly> detectVolumeAnomaly(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol, double k);

std::optional<Anomaly> detectSpreadAnomaly(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol, double k);
