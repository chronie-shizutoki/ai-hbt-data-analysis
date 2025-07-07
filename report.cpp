
#include <string>
namespace {
static std::string str_replace_all(std::string s, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
    }
    return s;
}
}
#include "include/report.h"
#include "include/i18n.h"
#include <fstream>
#include <iomanip>
#include <ctime>
#include <numeric>
#include <algorithm>
#include <set>
#include <string>

static std::string extract_weekday(const Record& record) {
    const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
    return weekdays[record.time_tm.tm_wday];
}

static std::string sentiment_analysis(const std::string& remark) {
    static const std::set<std::string> NEGATIVE_WORDS = {"差", "不好", "黑名单", "极差", "烂", "差劲"};
    for (const auto& word : NEGATIVE_WORDS) {
        if (remark.find(word) != std::string::npos) {
            return "负面";
        }
    }
    return "中性/正面";
}

// 国际化文本报告生成
void generate_report_i18n(const std::vector<Record>& records, const Stats& global_stats, const std::map<std::string, Stats>& type_stats, const std::map<std::string, Stats>& product_stats, const std::map<std::string, Stats>& country_stats, const std::map<std::string, Stats>& monthly_stats, const std::map<std::string, Stats>& unit_price_stats, const I18N& i18n, const std::string& filename) {
    std::ofstream report(filename);
    time_t now = time(nullptr);
    tm* now_tm = localtime(&now);
    report << "==================== " << i18n.t("report_title") << " ====================\n";
    report << i18n.t("analysis_time") << ": " << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S") << "\n";
    report << i18n.t("total_records") << ": " << records.size() << "\n";
    report << i18n.t("total_amount") << ": " << std::fixed << std::setprecision(2) << global_stats.total << " " << i18n.t("yuan") << "\n";
    report << i18n.t("avg_amount") << ": " << global_stats.avg << " " << i18n.t("yuan") << "\n";
    report << i18n.t("min_amount") << ": " << global_stats.min << " " << i18n.t("yuan") << "\n";
    report << i18n.t("max_amount") << ": " << global_stats.max << " " << i18n.t("yuan") << "\n";
    report << i18n.t("median_amount") << ": " << global_stats.median() << " " << i18n.t("yuan") << "\n";
    report << i18n.t("stddev_amount") << ": " << global_stats.std_dev() << " " << i18n.t("yuan") << " (" << i18n.t("volatility") << ")\n\n";
    // 按类别统计
    report << "==================== " << i18n.t("category_analysis") << " ====================\n";
    std::vector<std::pair<std::string, Stats>> sorted_types(type_stats.begin(), type_stats.end());
    std::sort(sorted_types.begin(), sorted_types.end(), [](const auto& a, const auto& b) { return a.second.total > b.second.total; });
    for (const auto& [type, stat] : sorted_types) {
        report << "[" << type << "]\n";
        report << "  " << i18n.t("total") << ": " << stat.total << " " << i18n.t("yuan") << " (" << std::fixed << std::setprecision(1) << (stat.total * 100.0 / global_stats.total) << "%)\n";
        report << "  " << i18n.t("count") << ": " << stat.count << "\n";
        report << "  " << i18n.t("avg") << ": " << stat.avg << " " << i18n.t("per_time") << "\n";
        report << "  " << i18n.t("range") << ": " << stat.min << " - " << stat.max << " " << i18n.t("yuan") << "\n\n";
    }
    // 消费模式识别
    int blacklist_count = 0, imported_count = 0;
    double blacklist_total = 0.0, imported_total = 0.0;
    std::vector<std::string> blacklist_products;
    std::map<std::string, int> weekday_count, sentiment_count;
    std::map<std::string, double> weekday_amount;
    for (const auto& r : records) {
        if (r.is_blacklist) {
            blacklist_count++;
            blacklist_total += r.amount;
            blacklist_products.push_back(r.product_name);
        }
        if (r.is_imported) {
            imported_count++;
            imported_total += r.amount;
        }
        std::string weekday = extract_weekday(r);
        weekday_count[weekday]++;
        weekday_amount[weekday] += r.amount;
        std::string sentiment = sentiment_analysis(r.remark);
        sentiment_count[sentiment]++;
    }
    report << "==================== " << i18n.t("pattern_analysis") << " ====================\n";
    report << "1. " << i18n.t("blacklist_analysis") << ":\n";
    report << "   - " << i18n.t("blacklist_count") << ": " << blacklist_count << " " << i18n.t("item") << " (" << std::fixed << std::setprecision(1) << (blacklist_count * 100.0 / records.size()) << "%)\n";
    report << "   - " << i18n.t("blacklist_total") << ": " << blacklist_total << " " << i18n.t("yuan") << "\n";
    if (!blacklist_products.empty()) {
        report << "   - " << i18n.t("blacklist_main") << ": ";
        for (const auto& p : blacklist_products) report << p << ", ";
        report << "\n";
    }
    report << "\n2. " << i18n.t("import_analysis") << ":\n";
    report << "   - " << i18n.t("import_analysis") << ": " << imported_count << " " << i18n.t("item") << "\n";
    report << "   - " << i18n.t("total_amount") << ": " << std::fixed << std::setprecision(1) << (imported_total * 100.0 / global_stats.total) << "%\n";
    report << "\n3. " << i18n.t("time_distribution") << ":\n";
    const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
    for (const char* day : weekdays) {
        if (weekday_count.find(day) != weekday_count.end()) {
            std::string tpl = i18n.t("weekday_stats");
            std::string line = tpl;
            // 替换 {weekday} {count} {total} {avg}
            size_t pos;
            while ((pos = line.find("{weekday}")) != std::string::npos) line.replace(pos, 9, day);
            while ((pos = line.find("{count}")) != std::string::npos) line.replace(pos, 7, std::to_string(weekday_count[day]));
            while ((pos = line.find("{total}")) != std::string::npos) line.replace(pos, 7, std::to_string(weekday_amount[day]));
            while ((pos = line.find("{avg}")) != std::string::npos) line.replace(pos, 5, std::to_string(weekday_amount[day] / weekday_count[day]));
            report << "   - " << line << "\n";
        }
    }
    report << "\n4. " << i18n.t("sentiment_analysis") << ":\n";
    for (const auto& [sentiment, count] : sentiment_count) {
        std::string key = (sentiment == "负面") ? "sentiment_negative" : "sentiment_neutral";
        std::string tpl = i18n.t(key + "_stats");
        std::string line = tpl;
        double percent = count * 100.0 / records.size();
        size_t pos;
        while ((pos = line.find("{count}")) != std::string::npos) line.replace(pos, 7, std::to_string(count));
        while ((pos = line.find("{percent}")) != std::string::npos) line.replace(pos, 9, std::to_string(percent));
        report << "   - " << i18n.t(key) << ": " << line << "\n";
    }
    // 月度趋势分析
    if (monthly_stats.size() > 1) {
        report << "\n==================== " << i18n.t("monthly_trend") << " ====================\n";
        std::vector<std::pair<std::string, Stats>> monthly_sorted(monthly_stats.begin(), monthly_stats.end());
        std::sort(monthly_sorted.begin(), monthly_sorted.end());
        for (size_t i = 0; i < monthly_sorted.size(); i++) {
            const auto& [month, stat] = monthly_sorted[i];
            report << month << ": " << stat.total << " " << i18n.t("yuan") << " (" << stat.count << " " << i18n.t("count") << ")";
            if (i > 0) {
                double prev_total = monthly_sorted[i-1].second.total;
                double change = (stat.total - prev_total) / prev_total * 100;
                report << " | MoM: " << (change >= 0 ? "+" : "") << std::fixed << std::setprecision(1) << change << "%";
            }
            std::map<std::string, double> type_contrib;
            for (const auto& r : records) {
                std::ostringstream oss;
                oss << std::put_time(&r.time_tm, "%Y-%m");
                if (oss.str() == month) {
                    type_contrib[r.type] += r.amount;
                }
            }
            if (!type_contrib.empty()) {
                report << "\n   " << i18n.t("category_analysis") << ": ";
                for (const auto& [type, amount] : type_contrib) {
                    report << type << "(" << std::fixed << std::setprecision(0) << (amount * 100 / stat.total) << "%) ";
                }
            }
            report << "\n";
        }
    }
    // 优化建议
    report << "\n==================== " << i18n.t("optimization") << " ====================\n";
    if (blacklist_count > 0) {
        std::string line = i18n.t("advice_blacklist_count");
        line = str_replace_all(line, "{count}", std::to_string(blacklist_count));
        line = str_replace_all(line, "{total}", std::to_string(blacklist_total));
        line = str_replace_all(line, "{unit}", i18n.t("yuan"));
        report << "1. " << i18n.t("advice_reduce_blacklist") << "\n";
        report << "   - " << line << "\n";
        report << "   - " << i18n.t("advice_blacklist_suggestion") << "\n";
    }
    double luxury_threshold = global_stats.avg * 3;
    int luxury_count = std::count_if(records.begin(), records.end(), [&](const Record& r) { return r.unit_price > luxury_threshold; });
    if (luxury_count > 0) {
        std::string line = i18n.t("advice_luxury_count");
        line = str_replace_all(line, "{count}", std::to_string(luxury_count));
        line = str_replace_all(line, "{threshold}", std::to_string(luxury_threshold));
        line = str_replace_all(line, "{unit}", i18n.t("yuan"));
        report << "2. " << i18n.t("advice_luxury_opt") << "\n";
        report << "   - " << line << "\n";
        report << "   - " << i18n.t("advice_luxury_suggestion") << "\n";
    }
    if (imported_count > 0) {
        double imported_percent = imported_total * 100 / global_stats.total;
        std::string line = i18n.t("advice_import_percent");
        line = str_replace_all(line, "{percent}", std::to_string(imported_percent));
        report << "3. " << i18n.t("advice_import_opt") << "\n";
        report << "   - " << line << "\n";
        if (imported_percent > 30) {
            report << "   - " << i18n.t("advice_import_high") << "\n";
        } else {
            report << "   - " << i18n.t("advice_import_normal") << "\n";
        }
    }
    report << "\n==================== " << i18n.t("end") << " ====================\n";
    report.close();
}
