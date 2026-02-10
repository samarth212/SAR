#include "anomaly_detector.h"
#include "util/stdev.h"

/*

What you do:

For each quote update, compute
newSpread = ask_price − bid_price
(only if both are valid and ask ≥ bid)

Keep a rolling history of spreads for that symbol (last N spreads).

Compute baseline:
avgSpread and stdevSpread over that window.

Trigger anomaly when spread gets unusually wide, like:
If newSpread > avgSpread + k × stdevSpread, that’s a spread-widening anomaly.




*/

double averageSpreadOfRecentQuotes(const std::string &symbol,
                                   const std::unordered_map<std::string, SymbolState> &bySymbol) {

    if (symbol.empty() || !bySymbol.contains(symbol)) {
        return 0.0;
    }

    const auto &state = bySymbol.at(symbol);
    const auto &spreads = state.spreads;

    if (spreads.empty())
        return 0.0;

    double sum = 0.0;

    for (double spread : spreads) {
        sum += spread;
    }

    return sum / static_cast<double>(spreads.size());
}

std::optional<Anomaly>
detectSpreadAnomaly(const std::string &symbol,
                    const std::unordered_map<std::string, SymbolState> &bySymbol, double k) {
    if (symbol.empty() || !bySymbol.contains(symbol)) {
        return std::nullopt;
    }

    const auto &state = bySymbol.at(symbol);
    if (!state.lastQuote.has_value()) {
        return std::nullopt;
    }

    const double ap = state.lastQuote.value().ask_price;
    const double bp = state.lastQuote.value().bid_price;

    if (ap <= 0.0 || bp <= 0.0 || ap < bp) {
        return std::nullopt;
    }

    const double newSpread = ap - bp;
    const auto &spreads = state.spreads;

    constexpr std::size_t MIN_POINTS = 20;
    if (spreads.size() < MIN_POINTS) {
        return std::nullopt;
    }

    double avgSpread = averageSpreadOfRecentQuotes(symbol, bySymbol);
    double stdev = calcSTDEV<double>(spreads);

    constexpr double EPS = 1e-9;
    if (stdev <= EPS) {
        return std::nullopt;
    }

    if (newSpread > avgSpread + (k * stdev)) {
        Anomaly newAnomaly;
        newAnomaly.type = AnomalyType::Spread;
        newAnomaly.source = SourceType::Quote;
        newAnomaly.direction = Direction::Up;

        newAnomaly.symbol = symbol;
        newAnomaly.timestamp = state.lastQuoteTs;

        newAnomaly.value = newSpread;
        newAnomaly.mean = avgSpread;
        newAnomaly.stdev = stdev;
        newAnomaly.zscore = (newSpread - avgSpread) / stdev;

        newAnomaly.lower = avgSpread - (k * stdev);
        newAnomaly.upper = avgSpread + (k * stdev);
        newAnomaly.k = k;

        newAnomaly.note =
            "Upward spread anomaly: " + symbol + " has a bid-ask spread of " +
            std::to_string(newSpread) + ", which is above the recent average " +
            std::to_string(avgSpread) + " by " + std::to_string(newSpread - avgSpread) + " (" +
            std::to_string(newAnomaly.zscore) +
            " standard deviations). "
            "This suggests liquidity is thinner than usual and prices may be less stable, which "
            "can happen during uncertainty, low activity, or around fast-moving news.";

        return newAnomaly;
    } else if (newSpread < avgSpread - (k * stdev)) {
        Anomaly newAnomaly;
        newAnomaly.type = AnomalyType::Spread;
        newAnomaly.source = SourceType::Quote;
        newAnomaly.direction = Direction::Down;

        newAnomaly.symbol = symbol;
        newAnomaly.timestamp = state.lastQuoteTs;

        newAnomaly.value = newSpread;
        newAnomaly.mean = avgSpread;
        newAnomaly.stdev = stdev;
        newAnomaly.zscore = (newSpread - avgSpread) / stdev;

        newAnomaly.lower = avgSpread - (k * stdev);
        newAnomaly.upper = avgSpread + (k * stdev);
        newAnomaly.k = k;

        newAnomaly.note =
            "Downward spread anomaly: " + symbol + " has a bid-ask spread of " +
            std::to_string(newSpread) + ", which is below the recent average " +
            std::to_string(avgSpread) + " by " + std::to_string(avgSpread - newSpread) + " (" +
            std::to_string(-newAnomaly.zscore) +
            " standard deviations). "
            "This suggests unusually tight liquidity and smoother trading conditions than normal, "
            "which can happen when many buyers and sellers are active at the same time.";

        return newAnomaly;
    }
    return std::nullopt;
}
