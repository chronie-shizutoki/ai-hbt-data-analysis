#pragma once
#include <vector>
#include <string>
#include <map>
#include "record.h"

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
