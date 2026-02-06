#include "data_parser.h"
#include <iostream>


// This function RECEIVES a string and RETURNS a vector
std::vector<MarketEvent> parseMessage(const std::string& jsonText){

    std::vector<MarketEvent> results;

    json parsedOutput = json::parse(jsonText, nullptr, false);
    if (root.is_discarded()) return results;

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

   
    }

    if(parsedOutput.isArray()){
        for (const auto& msg : parsedOutput){
            handle_datatype(msg)
        } else{
            handle_datatype(parsedOutput)
        }
    }

    return results;

}

    

