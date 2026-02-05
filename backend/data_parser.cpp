#include "data_parser.h"
#include <iostream>


// This function RECEIVES a string and RETURNS a vector
std::vector<MarketEvent> parseMessage(const std::string& jsonText){
    std::vector<MarketEvent> results;

    json parsedOutput = json::parse(jsonText);
    std::vector<Quote> Quotes;

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


    std::cout << "Vector items: ";
    for (int item : v) {
        std::cout << item << ", ";
    }
    std::cout << std::endl;



    return results;

}

    

