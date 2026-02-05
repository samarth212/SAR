#include "data_parser.h"
#include <iostream>


// This function RECEIVES a string and RETURNS a vector
std::vector<TradeData> parseMessage(const std::string& jsonText){
    std::vector<TradeData> results;

    json jsonifiedText = json::parse(jsonText);
    std::vector<int> trades;

    jsonifiedText["trades"].get_to(trades)

    std::cout << "Vector items: ";
    for (int item : v) {
        std::cout << item << ", ";
    }
    std::cout << std::endl;



    return results;

}

    

