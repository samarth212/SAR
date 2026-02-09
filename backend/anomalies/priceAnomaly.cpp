#include "anomaly_detector.h"
#include "util/stdev.h"

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

    constexpr std::size_t MIN_POINTS = 20;
    if (prices.size() < MIN_POINTS) {
        return std::nullopt;
    }

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

