#include "include/complex_analyzer.h"
#include <ctime>
#include <algorithm>
#include <numeric>
#include <set>

AnalysisResult complex_analysis(const std::vector<Record>& records, const I18N& i18n) {
    AnalysisResult result;
    result.lang = i18n.t("lang_code");
    // 生成时间
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    result.generated_time = buf;
    result.total_records = records.size();
    // 金额统计
    std::vector<double> amounts;
    for (const auto& r : records) amounts.push_back(r.amount);
    result.total_amount = std::accumulate(amounts.begin(), amounts.end(), 0.0);
    result.avg_amount = result.total_records ? result.total_amount / result.total_records : 0.0;
    result.min_amount = amounts.empty() ? 0.0 : *std::min_element(amounts.begin(), amounts.end());
    result.max_amount = amounts.empty() ? 0.0 : *std::max_element(amounts.begin(), amounts.end());
    if (!amounts.empty()) {
        std::vector<double> sorted = amounts;
        std::sort(sorted.begin(), sorted.end());
        size_t n = sorted.size() / 2;
        result.median_amount = (sorted.size() % 2 == 0) ? (sorted[n-1] + sorted[n]) / 2.0 : sorted[n];
        double mean = result.avg_amount;
        double var = 0.0;
        for (auto v : amounts) var += (v - mean) * (v - mean);
        result.stddev_amount = sorted.size() > 1 ? sqrt(var / (sorted.size() - 1)) : 0.0;
    }
    // 按类别/产品统计
    for (const auto& r : records) {
        result.category_total[r.type] += r.amount;
        result.product_total[r.product_name] += r.amount;
    }
    // 简单异常检测（如金额大于均值3倍）
    for (const auto& r : records) {
        if (r.amount > result.avg_amount * 3) {
            result.anomalies.push_back(r.remark + " (" + std::to_string(r.amount) + ")");
        }
    }
    // ...可扩展复杂聚类、预测、规则挖掘等...
    return result;
}
