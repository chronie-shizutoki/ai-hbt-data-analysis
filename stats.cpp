
#include "include/stats.h"
#include "include/record.h"
#include <numeric>
#include <algorithm>
#include <cmath>

// 简单自回归(AR)模型实现
void TimeSeriesForecaster::fit(const std::vector<double>& data, int order) {
    ar_order = order;
    train_data = data;
    // 仅实现AR(1)最小二乘拟合
    ar_coeffs.clear();
    if ((int)data.size() <= order) return;
    double num = 0.0, denom = 0.0;
    for (size_t i = order; i < data.size(); ++i) {
        num += data[i] * data[i-1];
        denom += data[i-1] * data[i-1];
    }
    if (denom != 0)
        ar_coeffs.push_back(num / denom);
    else
        ar_coeffs.push_back(0.0);
}

std::vector<double> TimeSeriesForecaster::predict(int steps) const {
    std::vector<double> result;
    if (train_data.empty() || ar_coeffs.empty()) return result;
    double last = train_data.back();
    for (int i = 0; i < steps; ++i) {
        double pred = ar_coeffs[0] * last;
        result.push_back(pred);
        last = pred;
    }
    return result;
}

nlohmann::json TimeSeriesForecaster::to_json() const {
    nlohmann::json j;
    j["ar_order"] = ar_order;
    j["ar_coeffs"] = ar_coeffs;
    j["train_data"] = train_data;
    return j;
}

void Stats::add_value(double value) {
    total += value;
    count++;
    values.push_back(value);
    avg = total / count;
    if (value < min) min = value;
    if (value > max) max = value;
}

double Stats::median() const {
    if (values.empty()) return 0.0;
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    size_t n = sorted.size() / 2;
    if (sorted.size() % 2 == 0) {
        return (sorted[n-1] + sorted[n]) / 2.0;
    }
    return sorted[n];
}

double Stats::std_dev() const {
    if (values.size() < 2) return 0.0;
    double variance = 0.0;
    for (double v : values) {
        variance += pow(v - avg, 2);
    }
    return sqrt(variance / (values.size() - 1));
}

bool Stats::operator<(const Stats& other) const {
    return total < other.total;
}

#include <ctime>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>

static std::string extract_month(const Record& record) {
    std::ostringstream oss;
    oss << std::put_time(&record.time_tm, "%Y-%m");
    return oss.str();
}

void compute_stats(const std::vector<Record>& records, std::map<std::string, Stats>& type_stats, std::map<std::string, Stats>& product_stats, std::map<std::string, Stats>& country_stats, std::map<std::string, Stats>& monthly_stats, std::map<std::string, Stats>& unit_price_stats, Stats& global_stats) {
    for (const auto& r : records) {
        global_stats.add_value(r.amount);
        type_stats[r.type].add_value(r.amount);
        product_stats[r.product_name].add_value(r.amount);
        if (!r.origin_country.empty()) {
            country_stats[r.origin_country].add_value(r.amount);
        }
        monthly_stats[extract_month(r)].add_value(r.amount);
        unit_price_stats[r.product_name].add_value(r.unit_price);
    }
}
