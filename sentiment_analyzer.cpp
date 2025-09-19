#include "include/sentiment_analyzer.h"
#include <fstream>

bool SentimentAnalyzer::load(const std::string& sentiment_file) {
    std::ifstream f(sentiment_file);
    if (!f.is_open()) return false;
    nlohmann::json j;
    f >> j;

    if (j.contains("positive") && j["positive"].is_array()) {
        for (const auto& word : j["positive"]) {
            if (word.is_string()) {
                positive_words.insert(word.get<std::string>());
            }
        }
    }
    if (j.contains("negative") && j["negative"].is_array()) {
        for (const auto& word : j["negative"]) {
            if (word.is_string()) {
                negative_words.insert(word.get<std::string>());
            }
        }
    }
    return true;
}

std::pair<std::string, double> SentimentAnalyzer::analyze(const std::string& text) const {
    int score = 0;
    for (const auto& word : positive_words) {
        if (text.find(word) != std::string::npos) {
            score++;
        }
    }
    for (const auto& word : negative_words) {
        if (text.find(word) != std::string::npos) {
            score--;
        }
    }

    if (score > 0) {
        return {"positive", 1.0};
    } else if (score < 0) {
        return {"negative", -1.0};
    } else {
        return {"neutral", 0.0};
    }
}


