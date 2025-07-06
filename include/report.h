#pragma once
#include <vector>
#include <string>
#include "record.h"
#include "stats.h"
#include <map>

void generate_report(const std::vector<Record>& records, const Stats& global_stats, const std::map<std::string, Stats>& type_stats, const std::map<std::string, Stats>& product_stats, const std::map<std::string, Stats>& country_stats, const std::map<std::string, Stats>& monthly_stats, const std::map<std::string, Stats>& unit_price_stats, const std::string& filename = "高级消费分析报告.txt");
