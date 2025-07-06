#include "include/report.h"
#include <fstream>
#include <iomanip>
#include <ctime>
#include <numeric>
#include <algorithm>
#include <set>

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

void generate_report(const std::vector<Record>& records, const Stats& global_stats, const std::map<std::string, Stats>& type_stats, const std::map<std::string, Stats>& product_stats, const std::map<std::string, Stats>& country_stats, const std::map<std::string, Stats>& monthly_stats, const std::map<std::string, Stats>& unit_price_stats, const std::string& filename) {
    std::ofstream report(filename);
    time_t now = time(nullptr);
    tm* now_tm = localtime(&now);
    report << "==================== 高级消费数据分析报告 ====================\n";
    report << "分析时间: " << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S") << "\n";
    report << "总记录数: " << records.size() << "\n";
    report << "总消费金额: " << std::fixed << std::setprecision(2) << global_stats.total << " 元\n";
    report << "单笔平均消费: " << global_stats.avg << " 元\n";
    report << "单笔最低消费: " << global_stats.min << " 元\n";
    report << "单笔最高消费: " << global_stats.max << " 元\n";
    report << "消费金额中位数: " << global_stats.median() << " 元\n";
    report << "消费金额标准差: " << global_stats.std_dev() << " 元 (波动率)\n\n";
    // 按类别统计
    report << "==================== 按消费类别分析 ====================\n";
    std::vector<std::pair<std::string, Stats>> sorted_types(type_stats.begin(), type_stats.end());
    std::sort(sorted_types.begin(), sorted_types.end(), [](const auto& a, const auto& b) { return a.second.total > b.second.total; });
    for (const auto& [type, stat] : sorted_types) {
        report << "[" << type << "]\n";
        report << "  总消费: " << stat.total << " 元 (" << std::fixed << std::setprecision(1) << (stat.total * 100.0 / global_stats.total) << "%)\n";
        report << "  交易次数: " << stat.count << "\n";
        report << "  平均消费: " << stat.avg << " 元/次\n";
        report << "  价格区间: " << stat.min << " - " << stat.max << " 元\n\n";
    }
    // 按产品统计
    report << "==================== 按产品分析 ====================\n";
    std::vector<std::pair<std::string, Stats>> sorted_products(product_stats.begin(), product_stats.end());
    std::sort(sorted_products.begin(), sorted_products.end(), [](const auto& a, const auto& b) { return a.second.total > b.second.total; });
    for (const auto& [product, stat] : sorted_products) {
        report << "[" << product << "]\n";
        report << "  总消费: " << stat.total << " 元\n";
        report << "  购买次数: " << stat.count << "\n";
        if (unit_price_stats.count(product)) {
            auto& price_stat = unit_price_stats.at(product);
            report << "  平均单价: " << price_stat.avg << " 元\n";
            report << "  单价波动: ±" << price_stat.std_dev() << " 元\n";
        }
        report << "\n";
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
    report << "==================== 消费模式识别 ====================\n";
    report << "1. 黑名单消费分析:\n";
    report << "   - 黑名单商品数量: " << blacklist_count << " 个 (" << std::fixed << std::setprecision(1) << (blacklist_count * 100.0 / records.size()) << "%)\n";
    report << "   - 黑名单消费总额: " << blacklist_total << " 元\n";
    if (!blacklist_products.empty()) {
        report << "   - 主要黑名单商品: ";
        for (const auto& p : blacklist_products) report << p << ", ";
        report << "\n";
    }
    report << "\n2. 进口商品分析:\n";
    report << "   - 进口商品数量: " << imported_count << " 个\n";
    report << "   - 进口商品消费占比: " << std::fixed << std::setprecision(1) << (imported_total * 100.0 / global_stats.total) << "%\n";
    report << "\n3. 消费时间分布 (按星期):\n";
    const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
    for (const char* day : weekdays) {
        if (weekday_count.find(day) != weekday_count.end()) {
            report << "   - 星期" << day << ": " << weekday_count[day] << " 笔交易, 总额 " << weekday_amount[day] << " 元, 平均 " << weekday_amount[day] / weekday_count[day] << " 元/笔\n";
        }
    }
    report << "\n4. 消费评价情感分析:\n";
    for (const auto& [sentiment, count] : sentiment_count) {
        report << "   - " << sentiment << "评价: " << count << " 条 (" << std::fixed << std::setprecision(1) << (count * 100.0 / records.size()) << "%)\n";
    }
    // 价格弹性分析
    report << "==================== 价格弹性分析 ====================\n";
    report << "产品\t平均单价\t总销量\t价格弹性\n";
    for (const auto& [product, stat] : product_stats) {
        if (unit_price_stats.find(product) == unit_price_stats.end()) continue;
        double avg_price = unit_price_stats.at(product).avg;
        double total_quantity = stat.count;
        double elasticity = 0.0;
        if (avg_price > 0 && total_quantity > 0) {
            elasticity = -0.5 * (avg_price / 10.0);
        }
        report << product << "\t" << std::fixed << std::setprecision(2) << avg_price << "\t" << total_quantity << "\t" << elasticity << "\n";
    }
    // 月度趋势分析
    if (monthly_stats.size() > 1) {
        report << "\n==================== 月度消费趋势 ====================\n";
        std::vector<std::pair<std::string, Stats>> monthly_sorted(monthly_stats.begin(), monthly_stats.end());
        std::sort(monthly_sorted.begin(), monthly_sorted.end());
        for (size_t i = 0; i < monthly_sorted.size(); i++) {
            const auto& [month, stat] = monthly_sorted[i];
            report << month << ": " << stat.total << " 元 (" << stat.count << " 笔)";
            if (i > 0) {
                double prev_total = monthly_sorted[i-1].second.total;
                double change = (stat.total - prev_total) / prev_total * 100;
                report << " | 环比: " << (change >= 0 ? "+" : "") << std::fixed << std::setprecision(1) << change << "%";
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
                report << "\n   消费构成: ";
                for (const auto& [type, amount] : type_contrib) {
                    report << type << "(" << std::fixed << std::setprecision(0) << (amount * 100 / stat.total) << "%) ";
                }
            }
            report << "\n";
        }
    }
    // 优化建议
    report << "\n==================== 消费优化建议 ====================\n";
    if (blacklist_count > 0) {
        report << "1. 减少黑名单商品消费:\n";
        report << "   - 您有 " << blacklist_count << " 笔黑名单商品消费，总额 " << blacklist_total << " 元\n";
        report << "   - 建议减少购买品质不佳的商品，优化消费选择\n";
    }
    double luxury_threshold = global_stats.avg * 3;
    int luxury_count = std::count_if(records.begin(), records.end(), [&](const Record& r) { return r.unit_price > luxury_threshold; });
    if (luxury_count > 0) {
        report << "2. 奢侈品消费优化:\n";
        report << "   - 您有 " << luxury_count << " 笔奢侈品消费 (单价 > " << luxury_threshold << " 元)\n";
        report << "   - 建议评估这些高价值商品的性价比和实际需求\n";
    }
    if (imported_count > 0) {
        double imported_percent = imported_total * 100 / global_stats.total;
        report << "3. 进口商品消费分析:\n";
        report << "   - 进口商品消费占比: " << std::fixed << std::setprecision(1) << imported_percent << "%\n";
        if (imported_percent > 30) {
            report << "   - 进口商品比例较高，可考虑部分替换为国产品牌以节省开支\n";
        } else {
            report << "   - 进口商品比例在合理范围，保持当前消费习惯\n";
        }
    }
    report << "\n==================== 报告结束 ====================\n";
    report.close();
}
