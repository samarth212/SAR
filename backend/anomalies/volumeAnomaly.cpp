#include "anomaly_detector.h"
#include "util/stdev.h"

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

    constexpr std::size_t MIN_POINTS = 20;
    if (volumes.size() < MIN_POINTS) {
        return std::nullopt;
    }

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

