#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <string>
#include <vector>

struct TradeData {
    std::string symbol;
    double price;
    int volume;
    bool isValid;
};

class DataParser {
public:
    // This function RECEIVES a string and RETURNS a vector
    std::vector<TradeData> parseMessage(const std::string& jsonText);
};

#endif