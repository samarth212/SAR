#include "anomaly_detector.h"

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