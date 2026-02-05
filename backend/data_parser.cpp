#include "data_parser.h"
#include <iostream>


// This function RECEIVES a string and RETURNS a vector
std::vector<MarketEvent> parseMessage(const std::string& jsonText){
    std::vector<MarketEvent> results;

    json parsedOutput = json::parse(jsonText);
    std::vector<Quote> Quotes;
    std::vector<Trade> Trades;
    std::vector<Bar> Bars;

    // build quote
    Quote newQuote;
    newQuote.bid_exchange = parsedOutput.at("bx").get<std::string>();
    newQuote.bid_price = parsedOutput.at("bp").get<double>();
    newQuote.bid_size = parsedOutput.at("bs").get<std::int64_t>();
    newQuote.ask_exchange = parsedOutput.at("ax").get<std::string>();
    newQuote.ask_price = parsedOutput.at("ap").get<double>();
    newQuote.ask_size = parsedOutput.at("as").get<std::int64_t>();
    newQuote.conditions = parsedOutput.at("c").get<std::vector<std::string>>();
    newQuote.tape = parsedOutput.at("z").get<std::string>();
    newQuote.get_to(Quotes)



    // build trade
    Trade newTrade;
    newTrade.price = parsedOutput.at("p").get<double>();
    newTrade.size = parsedOutput.at("s").get<std::int64_t>();
    newTrade.exchange = parsedOutput.at("x").get<std::string>();
    newTrade.conditions = parsedOutput.at("c").get<std::vector<std::string>>();
    newTrade.tape = parsedOutput.at("z").get<std::string>();
    newTrade.get_to(Trades)


    std::cout << "Vector items: ";
    for (int item : v) {
        std::cout << item << ", ";
    }
    std::cout << std::endl;



    return results;

}

    

