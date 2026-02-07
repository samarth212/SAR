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


Anomaly detectPriceAnomaly(const std::string& symbol, const std::unordered_map<std::string, SymbolState>& bySymbol, double k){
    if (symbol.empty() || !bySymbol.contains(symbol)){
        return;
    }

    const auto& state  = bySymbol.at(symbol);
    if (!state.lastTrade.has_value()) {
        return;
    }
    const auto& newPrice = state.lastTrade.value().price;
    const auto& prices = state.prices;


    double avgPrice = averagePriceOfRecentTrades(symbol, bySymbol);
    double stdev = calcSTDEV<double>(prices);

    if(newPrice > avgPrice + (k * stdev)){
        Anomaly newAnomaly;
        newAnomaly.type = AnomalyType::Price;
        
    }
    else if(newPrice > avgPrice - (k * stdev)){
        //down anomaly
    }
    else{
        return;
    }
}