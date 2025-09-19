#pragma once
#include <string>
#include <vector>
#include <map>
#include <json.hpp>
#include <set>

class SentimentAnalyzer {
public:
    bool load(const std::string& sentiment_file);
    std::pair<std::string, double> analyze(const std::string& text) const;

private:
    std::set<std::string> positive_words;
    std::set<std::string> negative_words;
};


