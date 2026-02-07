#include "data_parser.h"
#include <iostream>


// keep only last N points so memory stays bounded
static void push_bounded(std::deque<double>& dq, double x, std::size_t maxN) {
    dq.push_back(x);
    if (dq.size() > maxN) dq.pop_front();
}
static void push_bounded(std::deque<std::int64_t>& dq, std::int64_t x, std::size_t maxN) {
    dq.push_back(x);
    if (dq.size() > maxN) dq.pop_front();
}

// updates the map using parsed events
void updateState(std::unordered_map<std::string, SymbolState>& bySymbol,
                        const std::vector<MarketEvent>& events,
                        std::size_t windowN) {
    for (const auto& ev : events) {
        auto& state = bySymbol[ev.symbol];

        if (ev.type == MarketEventType::Quote) {
            const Quote& q = std::get<Quote>(ev.data);
            state.lastQuote = q;

            double mid = q.mid_price();
            double spr = q.spread();
            if (mid > 0.0) push_bounded(state.prices, mid, windowN);
            if (spr > 0.0) push_bounded(state.spreads, spr, windowN);
        }
        else if (ev.type == MarketEventType::Trade) {
            const Trade& tr = std::get<Trade>(ev.data);
            state.lastTrade = tr;

            if (tr.price > 0.0) push_bounded(state.prices, tr.price, windowN);
            if (tr.size > 0)    push_bounded(state.tradeSizes, tr.size, windowN);
        }
        else if (ev.type == MarketEventType::Bar) {
            const Bar& b = std::get<Bar>(ev.data);
            state.lastBar = b;

            if (b.close > 0.0)  push_bounded(state.prices, b.close, windowN);
            if (b.volume > 0)   push_bounded(state.barVolumes, b.volume, windowN);
        }
    }
}

std::vector<MarketEvent> parseMessage(const std::string& jsonText){

    std::vector<MarketEvent> results;

    json parsedOutput = json::parse(jsonText, nullptr, false);
    if (parsedOutput.is_discarded()) return results;

    auto handle_datatype = [&](const json& msg){
        if (!msg.is_object()) return;

        const std::string T = msg.value("T", "");
        if (T.empty()) return;

        if (T == "success" || T == "subscription") return;

        MarketEvent ev;
        ev.symbol = msg.value("S", "");
        ev.timestamp = msg.value("t", "");
        ev.ts_ns = 0;
        if (ev.symbol.empty()) return;

        // build quote
        if (T == "q"){  

            Quote q;

            q.bid_exchange = msg.value("bx", "");
            q.bid_price = msg.value("bp", 0.0);
            q.bid_size = msg.value("bs", (std::int64_t)0);
            q.ask_exchange = msg.value("ax", "");
            q.ask_price = msg.value("ap", 0.0);
            q.ask_size = msg.value("as", (std::int64_t)0);
            q.conditions = msg.value("c", std::vector<std::string>{});
            q.tape = msg.value("z", "");

            ev.type = MarketEventType::Quote;
            ev.data = std::move(q);

            results.push_back(std::move(ev));
            return;
        }

        // build trade
        if (T == "t") {

            Trade tr;

            tr.price = msg.value("p", 0.0);
            tr.size  = msg.value("s", (std::int64_t)0);
            tr.exchange = msg.value("x", "");
            tr.conditions = msg.value("c", std::vector<std::string>{});
            tr.tape = msg.value("z", "");

            ev.type = MarketEventType::Trade;
            ev.data = std::move(tr);

            results.push_back(std::move(ev));
            return;
        }

        if (T == "b" || T == "u" || T == "d") {

            Bar b;

            b.open = msg.value("o", 0.0);
            b.high = msg.value("h", 0.0);
            b.low = msg.value("l", 0.0);
            b.close = msg.value("c", 0.0);
            b.volume = msg.value("v", (std::int64_t)0);
            b.trade_count = msg.value("n", (std::int64_t)0);

            if (msg.contains("vw") && !msg["vw"].is_null()) {
                b.vwap = msg["vw"].get<double>();
            } else {
                b.vwap = std::nullopt;
            }

            ev.type = MarketEventType::Bar;
            ev.data = std::move(b);
            results.push_back(std::move(ev));

            return;

        }

   
    };

    if (parsedOutput.is_array()) {
        for (const auto& msg : parsedOutput){
            handle_datatype(msg);
        } 
    }
    else{
            handle_datatype(parsedOutput);
    }
    

    return results;

}

    




