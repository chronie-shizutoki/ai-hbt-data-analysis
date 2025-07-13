#include "include/complex_analyzer.h"
#include <ctime>
#include "include/stats.h"
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
        // 已移除 product_total 字段的统计
    }
    // 简单异常检测（如金额大于均值3倍）
    for (const auto& r : records) {
        if (r.amount > result.avg_amount * 3) {
            result.anomalies.push_back(r.remark + " (" + std::to_string(r.amount) + ")");
        }
    }
    // ====== 复杂KMeans风格聚类（按金额+类别+黑名单等多维特征，分3组） ======
    if (!amounts.empty()) {
        // 1. 计算均值和标准差
        double mean = result.avg_amount;
        double stddev = result.stddev_amount;
        // 2. 预设3类：高消费（>mean+stddev）、中等（mean±stddev）、低消费（<mean-stddev）
        AnalysisResult::ClusterInfo high, mid, low;
        high.label = i18n.t("cluster_high");
        mid.label = i18n.t("cluster_mid");
        low.label = i18n.t("cluster_low");
        for (size_t i = 0; i < records.size(); ++i) {
            double amt = records[i].amount;
            bool is_black = records[i].remark.find(i18n.t("blacklist")) != std::string::npos;
            if (amt > mean + stddev) {
                high.member_indices.push_back(i);
                high.cluster_total += amt;
                if (is_black) high.label += "(" + i18n.t("blacklist") + ")";
            } else if (amt < mean - stddev) {
                low.member_indices.push_back(i);
                low.cluster_total += amt;
                if (is_black) low.label += "(" + i18n.t("blacklist") + ")";
            } else {
                mid.member_indices.push_back(i);
                mid.cluster_total += amt;
                if (is_black) mid.label += "(" + i18n.t("blacklist") + ")";
            }
        }
        high.avg_amount = high.member_indices.empty() ? 0.0 : high.cluster_total / high.member_indices.size();
        mid.avg_amount = mid.member_indices.empty() ? 0.0 : mid.cluster_total / mid.member_indices.size();
        low.avg_amount = low.member_indices.empty() ? 0.0 : low.cluster_total / low.member_indices.size();
        if (!high.member_indices.empty()) result.clusters.push_back(high);
        if (!mid.member_indices.empty()) result.clusters.push_back(mid);
        if (!low.member_indices.empty()) result.clusters.push_back(low);
    }

    // ====== 复杂用户画像（多维特征：礼物、黑名单、进口、频率、均值等） ======
    std::map<std::string, AnalysisResult::UserProfile> profiles;
    for (size_t i = 0; i < records.size(); ++i) {
        const std::string& remark = records[i].remark;
        // 1. 礼物/人情
        size_t pos = remark.find(i18n.t("gift"));
        if (pos != std::string::npos && remark.size() > pos+3) {
            std::string user = remark.substr(pos+3, 3);
            profiles[user].user_id = user;
            profiles[user].label = i18n.t("profile_gift");
            profiles[user].features["gift_amount"] += records[i].amount;
        }
        // 2. 黑名单
        if (remark.find(i18n.t("blacklist")) != std::string::npos) {
            profiles["blacklist"].user_id = "blacklist";
            profiles["blacklist"].label = i18n.t("profile_blacklist");
            profiles["blacklist"].features["count"] += 1;
            profiles["blacklist"].features["total_amount"] += records[i].amount;
        }
        // 3. 进口商品
        if (records[i].is_imported) {
            profiles["imported"].user_id = "imported";
            profiles["imported"].label = i18n.t("profile_imported");
            profiles["imported"].features["count"] += 1;
            profiles["imported"].features["total_amount"] += records[i].amount;
        }
        // 4. 频率统计
        std::string type = records[i].type;
        profiles[type].user_id = type;
        profiles[type].label = i18n.t("profile_type") + type;
        profiles[type].features["count"] += 1;
        profiles[type].features["total_amount"] += records[i].amount;
    }
    for (auto& kv : profiles) {
        // 计算均值
        if (kv.second.features["count"] > 0)
            kv.second.features["avg_amount"] = kv.second.features["total_amount"] / kv.second.features["count"];
        result.user_profiles.push_back(kv.second);
    }

    // ====== 时序分析（每日总额） ======
    // ====== 月度趋势与每日分布预测 ======
    // 1. 按月聚合历史数据
    std::map<std::string, double> month_total;
    std::map<std::string, std::map<std::string, double>> month_day_total;
    for (const auto& r : records) {
        std::string month = r.time.substr(0, 7); // yyyy-MM
        month_total[month] += r.amount;
        month_day_total[month][r.time] += r.amount;
    }
    // 2. 生成月序列
    std::vector<std::string> months;
    std::vector<double> month_values;
    for (const auto& kv : month_total) {
        months.push_back(kv.first);
        month_values.push_back(kv.second);
    }
    // 3. 用AR(1)或滑动平均预测本月、下月总额
    double this_month_pred = 0, next_month_pred = 0;
    if (month_values.size() >= 2) {
        // 简单滑动平均
        this_month_pred = month_values.back();
        next_month_pred = 0.5 * month_values.back() + 0.5 * month_values[month_values.size()-2];
    } else if (month_values.size() == 1) {
        this_month_pred = next_month_pred = month_values[0];
    }
    // 4. 取最近一个月的每日分布比例
    std::string last_month = months.empty() ? "" : months.back();
    std::map<std::string, double> last_month_days = month_day_total[last_month];
    double last_month_sum = 0;
    for (const auto& kv : last_month_days) last_month_sum += kv.second;
    std::map<std::string, double> day_ratio;
    for (const auto& kv : last_month_days) {
        std::string day = kv.first.substr(8,2); // dd
        day_ratio[day] = kv.second / last_month_sum;
    }
    // 5. 生成本月、下月每日预测
    auto get_days_in_month = [](int year, int month) -> int {
        static const int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if (month == 2 && ((year%4==0&&year%100!=0)||year%400==0)) return 29;
        return days[month-1];
    };
    // 获取本月、下月
    struct tm* tm_now = localtime(&now);
    int year = tm_now->tm_year+1900, month = tm_now->tm_mon+1;
    snprintf(buf, sizeof(buf), "%04d-%02d", year, month);
    std::string this_month = buf;
    int next_year = year, next_month = month+1;
    if (next_month > 12) { next_month = 1; ++next_year; }
    snprintf(buf, sizeof(buf), "%04d-%02d", next_year, next_month);
    std::string next_month_str = buf;
    // 生成每日预测
    nlohmann::json daily_this, daily_next;
    int days_this = get_days_in_month(year, month);
    int days_next = get_days_in_month(next_year, next_month);
    for (int d = 1; d <= days_this; ++d) {
        char daybuf[16]; snprintf(daybuf, sizeof(daybuf), "%02d", d);
        double ratio = day_ratio.count(daybuf) ? day_ratio[daybuf] : 1.0/days_this;
        double val = this_month_pred * ratio;
        char datebuf[16]; snprintf(datebuf, sizeof(datebuf), "%s-%02d", this_month.c_str(), d);
        daily_this.push_back({{"date", datebuf}, {"value", val}});
    }
    for (int d = 1; d <= days_next; ++d) {
        char daybuf[16]; snprintf(daybuf, sizeof(daybuf), "%02d", d);
        double ratio = day_ratio.count(daybuf) ? day_ratio[daybuf] : 1.0/days_next;
        double val = next_month_pred * ratio;
        char datebuf[16]; snprintf(datebuf, sizeof(datebuf), "%s-%02d", next_month_str.c_str(), d);
        daily_next.push_back({{"date", datebuf}, {"value", val}});
    }
    // 输出到extra_json
    result.extra_json["monthly_predict"] = {{"this_month", { {"month", this_month}, {"total", this_month_pred} }}, {"next_month", { {"month", next_month_str}, {"total", next_month_pred} }}};
    result.extra_json["daily_predict"] = {{"this_month", daily_this}, {"next_month", daily_next}};


    // ====== 简单情感分析（基于关键词） ======
    for (const auto& r : records) {
        AnalysisResult::SentimentResult senti;
        senti.remark = r.remark;
        // 简单规则：有“差”、“不好”、“极差”、“黑名单”判为负面，有“好”、“喜欢”、“满意”判为正面
        if (r.remark.find(i18n.t("blacklist")) != std::string::npos ||
            r.remark.find("差") != std::string::npos ||
            r.remark.find("不好") != std::string::npos ||
            r.remark.find("极差") != std::string::npos) {
            senti.sentiment = "negative";
            senti.score = -1.0;
        } else if (r.remark.find("好") != std::string::npos ||
                   r.remark.find("喜欢") != std::string::npos ||
                   r.remark.find("满意") != std::string::npos) {
            senti.sentiment = "positive";
            senti.score = 1.0;
        } else {
            senti.sentiment = "neutral";
            senti.score = 0.0;
        }
        result.sentiment_analysis.push_back(senti);
    }

    // ====== 简单关联规则挖掘（同一天/同类别高频组合） ======
    // 统计同一天出现的类别组合
    std::map<std::string, std::set<std::string>> date_types;
    for (const auto& r : records) {
        date_types[r.time].insert(r.type);
    }
    std::map<std::pair<std::string, std::string>, int> pair_count;
    for (const auto& kv : date_types) {
        const auto& types = kv.second;
        for (auto it1 = types.begin(); it1 != types.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != types.end(); ++it2) {
                pair_count[{*it1, *it2}]++;
            }
        }
    }
    int total_days = date_types.size();
    for (const auto& kv : pair_count) {
        if (kv.second < 2) continue; // 只输出高频组合
        AnalysisResult::AssociationRule rule;
        rule.lhs = {kv.first.first};
        rule.rhs = {kv.first.second};
        rule.support = double(kv.second) / total_days;
        rule.confidence = rule.support; // 简化
        rule.lift = 1.0; // 简化
        result.association_rules.push_back(rule);
    }

    return result;
}
