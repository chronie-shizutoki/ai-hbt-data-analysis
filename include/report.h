#pragma once
#include <vector>
#include <string>
#include "record.h"
#include "stats.h"
#include <map>

// 新增国际化版本
#include "i18n.h"
void generate_report_i18n(const std::vector<Record>& records, const Stats& global_stats, const std::map<std::string, Stats>& type_stats, const std::map<std::string, Stats>& product_stats, const std::map<std::string, Stats>& country_stats, const std::map<std::string, Stats>& monthly_stats, const std::map<std::string, Stats>& unit_price_stats, const I18N& i18n, const std::string& filename = "report.txt");
