#include "anomaly_detector.h"

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

double averagePriceOfRecentTrades(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol){

    if (symbol.empty() || !bySymbol.contains(symbol)){
        return 0.0;
    }

    const auto& state  = bySymbol.at(symbol);
    const auto& prices = state.prices;

    if (prices.empty()) return 0.0;

    double sum = 0.0;

    for (double price: prices){
        sum += price;
    }

    return sum / static_cast<double>(prices.size());

}

std::int64_t averageVolumeOfRecentTrades(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol){

    if (symbol.empty() || !bySymbol.contains(symbol)){
        return 0.0;
    }
    const auto& state  = bySymbol.at(symbol);
    const auto& volumes = state.barVolumes;

    if (volumes.empty()) return 0.0;

    std::int64_t sum = 0.0;

    for (std::int64_t volume: volumes){
        sum += volume;
    }

    return sum / static_cast<std::int64_t>(volumes.size());

}

template <typename T>
T calcSTDEV(const std::deque<T>& data){
    if (data.empty()) return 0.0;
    T sum = std::accumulate(data.begin(), data.end(), 0.0);
    T mean = sum / data.size();
    
    T sq_sum = std::inner_product(data.begin(), data.end(), data.begin(), 0.0,
        [](T a, T b) { return a + b; },
        [mean](T a, T b) { return (a - mean) * (b - mean); });
        
    return std::sqrt(sq_sum / data.size());
}


std::optional<Anomaly> detectPriceAnomaly(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol, double k){
    if (symbol.empty() || !bySymbol.contains(symbol)){
        return std::nullopt;
    }

    const auto& state  = bySymbol.at(symbol);
    if (!state.lastTrade.has_value()) {
        return std::nullopt;
    }
    const double newPrice = state.lastTrade.value().price;
    const auto& prices = state.prices;


    double avgPrice = averagePriceOfRecentTrades(symbol, bySymbol);
    double stdev = calcSTDEV<double>(prices);

    constexpr double EPS = 1e-9;
    if (stdev <= EPS) {
        return std::nullopt;
    }

    if(newPrice > avgPrice + (k * stdev)){
        Anomaly newAnomaly;
        newAnomaly.type = AnomalyType::Price;
        newAnomaly.source = SourceType::Trade;
        newAnomaly.direction = Direction::Up;

        newAnomaly.symbol = symbol;
        newAnomaly.timestamp = state.lastTradeTs;
        
        newAnomaly.value = newPrice;
        newAnomaly.mean = avgPrice;
        newAnomaly.stdev = stdev;
        newAnomaly.zscore = (newPrice - avgPrice)/stdev;

        newAnomaly.lower = avgPrice - (k * stdev);
        newAnomaly.upper = avgPrice + (k * stdev);
        newAnomaly.k = k;

        newAnomaly.note =
        "Upward price anomaly: " + symbol +
        " traded at " + std::to_string(newPrice) +
        ", which is above the recent average " + std::to_string(avgPrice) +
        " by " + std::to_string(newPrice - avgPrice) +
        " (" + std::to_string(newAnomaly.zscore) + " standard deviations). "
        "This suggests an unusually strong move compared to the stock's recent behavior, "
        "which can happen when new information hits the market or when short-term buying pressure spikes.";

        return newAnomaly;
    }
    else if(newPrice < avgPrice - (k * stdev)){
        Anomaly newAnomaly;
        newAnomaly.type = AnomalyType::Price;
        newAnomaly.source = SourceType::Trade;
        newAnomaly.direction = Direction::Down;

        newAnomaly.symbol = symbol;
        newAnomaly.timestamp = state.lastTradeTs;
        
        newAnomaly.value = newPrice;
        newAnomaly.mean = avgPrice;
        newAnomaly.stdev = stdev;
        newAnomaly.zscore = (newPrice - avgPrice)/stdev;

        newAnomaly.lower = avgPrice - (k * stdev);
        newAnomaly.upper = avgPrice + (k * stdev);
        newAnomaly.k = k;

        newAnomaly.note =
        "Downward price anomaly: " + symbol +
        " traded at " + std::to_string(newPrice) +
        ", which is below the recent average " + std::to_string(avgPrice) +
        " by " + std::to_string(avgPrice - newPrice) +
        " (" + std::to_string(-newAnomaly.zscore) + " standard deviations, threshold < " +
        std::to_string(newAnomaly.lower) + "). "
        "This suggests an unusually sharp drop compared to recent behavior, which can occur when negative news hits or short-term selling pressure increases.";

        return newAnomaly;
    }
    return std::nullopt;
}

std::optional<Anomaly> detectVolumeAnomaly(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol, double k) {
    if (symbol.empty() || !bySymbol.contains(symbol)) {
        return std::nullopt;
    }

    const auto& state = bySymbol.at(symbol);
    if (!state.lastBar.has_value()) {
        return std::nullopt;
    }

    const std::int64_t newVolume = state.lastBar.value().volume;
    const auto& volumes = state.barVolumes;

    const double avgVolume = averageVolumeOfRecentTrades(symbol, bySymbol);
    const double stdev = calcSTDEV<std::int64_t>(volumes);

    constexpr double EPS = 1e-9;
    if (stdev <= EPS) {
        return std::nullopt;
    }

    if (static_cast<double>(newVolume) > avgVolume + (k * stdev)) {
        Anomaly newAnomaly;
        newAnomaly.type = AnomalyType::Volume;
        newAnomaly.source = SourceType::Bar;
        newAnomaly.direction = Direction::Up;

        newAnomaly.symbol = symbol;
        newAnomaly.timestamp = state.lastBarTs;

        newAnomaly.value = static_cast<double>(newVolume);
        newAnomaly.mean = avgVolume;
        newAnomaly.stdev = stdev;
        newAnomaly.zscore = (newAnomaly.value - avgVolume) / stdev;

        newAnomaly.lower = avgVolume - (k * stdev);
        newAnomaly.upper = avgVolume + (k * stdev);
        newAnomaly.k = k;

        newAnomaly.note =
            "Upward volume anomaly: " + symbol +
            " had bar volume " + std::to_string(newVolume) +
            " shares, above the recent average " + std::to_string(avgVolume) +
            " by " + std::to_string(static_cast<double>(newVolume) - avgVolume) +
            " (" + std::to_string(newAnomaly.zscore) + " standard deviations, threshold > " +
            std::to_string(newAnomaly.upper) + "). "
            "This suggests unusually heavy trading activity, which often happens around news, earnings, market opens/closes, or large institutional orders.";

        return newAnomaly;
    }
    else if (static_cast<double>(newVolume) < avgVolume - (k * stdev)) {
        Anomaly newAnomaly;
        newAnomaly.type = AnomalyType::Volume;
        newAnomaly.source = SourceType::Bar;
        newAnomaly.direction = Direction::Down;

        newAnomaly.symbol = symbol;
        newAnomaly.timestamp = state.lastTradeTs;

        newAnomaly.value = static_cast<double>(newVolume);
        newAnomaly.mean = avgVolume;
        newAnomaly.stdev = stdev;
        newAnomaly.zscore = (newAnomaly.value - avgVolume) / stdev;

        newAnomaly.lower = avgVolume - (k * stdev);
        newAnomaly.upper = avgVolume + (k * stdev);
        newAnomaly.k = k;

        newAnomaly.note =
            "Downward volume anomaly: " + symbol +
            " had bar volume " + std::to_string(newVolume) +
            " shares, below the recent average " + std::to_string(avgVolume) +
            " by " + std::to_string(avgVolume - static_cast<double>(newVolume)) +
            " (" + std::to_string(-newAnomaly.zscore) + " standard deviations, threshold < " +
            std::to_string(newAnomaly.lower) + "). "
            "This suggests unusually quiet trading activity, which can happen during low-interest periods, off-hours, or when liquidity temporarily dries up.";

        return newAnomaly;
    }

    return std::nullopt;
}
