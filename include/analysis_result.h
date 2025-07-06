#pragma once
#include <string>
#include <vector>
#include <map>
#include "record.h"
#include <json.hpp>

struct AnalysisResult {
    std::string lang;
    std::string generated_time;
    size_t total_records;
    double total_amount;
    double avg_amount;
    double min_amount;
    double max_amount;
    double median_amount;
    double stddev_amount;
    std::map<std::string, double> category_total;
    std::map<std::string, double> product_total;
    std::vector<std::string> anomalies;
    // ...可扩展更多分析字段...

    nlohmann::json to_json() const;
};
