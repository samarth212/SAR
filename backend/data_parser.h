#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <deque>
#include <optional>


using json = nlohmann::json;

/*
Sample stream:
$ wscat -c wss://stream.data.alpaca.markets/v2/test
Connected (press CTRL+C to quit)
< [{"T":"success","msg":"connected"}]
> {"action":"auth","key":"<YOUR API KEY>","secret":"<YOUR API SECRET>"}
< [{"T":"success","msg":"authenticated"}]
> {"action":"subscribe","bars":["FAKEPACA"],"quotes":["FAKEPACA"]}
< [{"T":"subscription","trades":[],"quotes":["FAKEPACA"],"bars":["FAKEPACA"]}]
< [{"T":"q","S":"FAKEPACA","bx":"O","bp":133.85,"bs":4,"ax":"R","ap":135.77,"as":5,"c":["R"],"z":"A","t":"2024-07-24T07:56:53.639713735Z"}]
< [{"T":"q","S":"FAKEPACA","bx":"O","bp":133.85,"bs":4,"ax":"R","ap":135.77,"as":5,"c":["R"],"z":"A","t":"2024-07-24T07:56:58.641207127Z"}]
< [{"T":"b","S":"FAKEPACA","o":132.65,"h":136,"l":132.12,"c":134.65,"v":205,"t":"2024-07-24T07:56:00Z","n":16,"vw":133.7}]
*/

enum class MarketEventType {
    Quote,
    Trade,
    Bar
};


struct Quote {
    std::string bid_exchange;                // bx
    double bid_price = 0.0;                  // bp
    std::int64_t bid_size = 0;               // bs
    std::string ask_exchange;                // ax
    double ask_price = 0.0;                  // ap
    std::int64_t ask_size = 0;               // as
    std::vector<std::string> conditions;     // c
    std::string tape;                        // z

    double mid_price() const {
        if (bid_price > 0.0 && ask_price > 0.0) return (bid_price + ask_price) / 2.0;
        return 0.0;
    }

    double spread() const {
        if (bid_price > 0.0 && ask_price > 0.0) return ask_price - bid_price;
        return 0.0;
    }
};

struct Trade {
    double price = 0.0;                      // p
    std::int64_t size = 0;                   // s
    std::string exchange;                    // x
    std::vector<std::string> conditions;     // c
    std::string tape;                        // z
};

struct Bar {
    double open = 0.0;                       // o
    double high = 0.0;                       // h
    double low = 0.0;                        // l
    double close = 0.0;                      // c
    std::int64_t volume = 0;                 // v
    std::int64_t trade_count = 0;            // n
    std::optional<double> vwap;              // vw

    double range() const { return high - low; }
    double body() const { return close - open; }
};

struct MarketEvent {
    MarketEventType type = MarketEventType::Quote;
    std::string symbol;                      // S
    std::string timestamp;                   // t (ISO-8601)
    std::int64_t ts_ns = 0;                  // parsed epoch ns if available
    std::variant<Quote, Trade, Bar> data;
};

struct SymbolState {
    std::optional<Quote> lastQuote;
    std::optional<Trade> lastTrade;
    std::optional<Bar>   lastBar;

    std::deque<double> prices;
    std::deque<std::int64_t> sizes;
    std::deque<double> spreads;
};

std::vector<MarketEvent> parseMessage(const std::string& jsonText);

void updateState(std::unordered_map<std::string, SymbolState>& bySymbol,
                 const std::vector<MarketEvent>& events,
                 std::size_t windowN = 200);

class DataParser {
public:
    std::vector<MarketEvent> parseMessage(const std::string& jsonText);
};

#endif
