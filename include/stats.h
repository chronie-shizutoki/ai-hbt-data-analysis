#pragma once
#include <vector>
#include <string>
#include <map>
#include "record.h"
#include "json.hpp" // nlohmann/json 头文件相对路径修正

// 简单自回归(AR)时序预测
// 用于预测未来n步消费趋势

class TimeSeriesForecaster {
public:
    // 训练AR模型，order为阶数
    void fit(const std::vector<double>& data, int order = 1);
    // 预测未来n步
    std::vector<double> predict(int steps) const;
    // 导出模型参数和预测结果为JSON
    nlohmann::json to_json() const;
private:
    int ar_order = 1;
    std::vector<double> ar_coeffs; // AR系数
    std::vector<double> train_data;
};
struct Stats {
    double total = 0.0;
    double avg = 0.0;
    int count = 0;
    double min = 1e9;
    double max = 0.0;
    std::vector<double> values;
    void add_value(double value);
    double median() const;
    double std_dev() const;
    bool operator<(const Stats& other) const;
};

void compute_stats(const std::vector<Record>& records, std::map<std::string, Stats>& type_stats, std::map<std::string, Stats>& product_stats, std::map<std::string, Stats>& country_stats, std::map<std::string, Stats>& monthly_stats, std::map<std::string, Stats>& unit_price_stats, Stats& global_stats);
