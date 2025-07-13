#include "include/analysis_result.h"
#include <json.hpp>
#include <iostream>

nlohmann::json AnalysisResult::to_json() const {
    // 工具函数：trim和UTF-8校验
    auto trim = [](const std::string& s) -> std::string {
        if (s.empty()) return "";
        size_t start = s.find_first_not_of(" \t\r\n\v\f");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n\v\f");
        if (end == std::string::npos || end < start) return "";
        return s.substr(start, end - start + 1);
    };
    auto is_valid_utf8 = [](const std::string& str) {
        int c,i,ix,n,j;
        for (i=0, ix=str.length(); i < ix; i++) {
            c = (unsigned char) str[i];
            if (0x00 <= c && c <= 0x7f) n=0;
            else if ((c & 0xE0) == 0xC0) n=1;
            else if ( c==0xed && i<(ix-1) && ((unsigned char)str[i+1] & 0xa0)==0xa0) return false;
            else if ((c & 0xF0) == 0xE0) n=2;
            else if ((c & 0xF8) == 0xF0) n=3;
            else return false;
            for (j=0; j<n && i<ix; j++) {
                if ((++i == ix) || (((unsigned char)str[i] & 0xC0 ) != 0x80)) return false;
            }
        }
        return true;
    };
    // 清洗string字段
    auto safe_str = [&](const std::string& s, const std::string& field) -> std::string {
        std::string t = trim(s);
        if (!is_valid_utf8(t)) {
            std::cerr << "[to_json警告] 字段 '" << field << "' 存在非法UTF-8，已替换。原内容: " << s << std::endl;
            return "[非法UTF8]";
        }
        return t;
    };
    nlohmann::json j;
    j["lang"] = safe_str(lang, "lang");
    j["generated_time"] = safe_str(generated_time, "generated_time");
    j["total_records"] = total_records;
    j["total_amount"] = total_amount;
    j["avg_amount"] = avg_amount;
    j["min_amount"] = min_amount;
    j["max_amount"] = max_amount;
    j["median_amount"] = median_amount;
    j["stddev_amount"] = stddev_amount;
    // map<string, double> 清洗key
    nlohmann::json cat_total = nlohmann::json::object();
    for (const auto& kv : category_total) {
        std::string key = safe_str(kv.first, "category_total.key");
        cat_total[key] = kv.second;
    }
    j["category_total"] = cat_total;
    // 已移除 product_total 字段的生成
    // anomalies: vector<string>
    nlohmann::json anom = nlohmann::json::array();
    for (const auto& s : anomalies) {
        anom.push_back(safe_str(s, "anomalies[]"));
    }
    j["anomalies"] = anom;
    // 多维聚类结果
    nlohmann::json clusters_json = nlohmann::json::array();
    for (const auto& c : clusters) {
        nlohmann::json cj;
        cj["label"] = safe_str(c.label, "clusters.label");
        cj["member_indices"] = c.member_indices;
        cj["cluster_total"] = c.cluster_total;
        cj["avg_amount"] = c.avg_amount;
        clusters_json.push_back(cj);
    }
    j["clusters"] = clusters_json;

    // 用户画像
    nlohmann::json profiles_json = nlohmann::json::array();
    for (const auto& p : user_profiles) {
        nlohmann::json pj;
        pj["user_id"] = safe_str(p.user_id, "user_profiles.user_id");
        pj["label"] = safe_str(p.label, "user_profiles.label");
        pj["features"] = p.features;
        profiles_json.push_back(pj);
    }
    j["user_profiles"] = profiles_json;

    // 时序分析
    nlohmann::json ts_json = nlohmann::json::array();
    for (const auto& t : time_series) {
        nlohmann::json tj;
        tj["date"] = safe_str(t.date, "time_series.date");
        tj["value"] = t.value;
        ts_json.push_back(tj);
    }
    j["time_series"] = ts_json;

    // 关联规则
    nlohmann::json rules_json = nlohmann::json::array();
    for (const auto& r : association_rules) {
        nlohmann::json rj;
        rj["lhs"] = r.lhs;
        rj["rhs"] = r.rhs;
        rj["support"] = r.support;
        rj["confidence"] = r.confidence;
        rj["lift"] = r.lift;
        rules_json.push_back(rj);
    }
    j["association_rules"] = rules_json;

    // 情感分析
    nlohmann::json senti_json = nlohmann::json::array();
    for (const auto& s : sentiment_analysis) {
        nlohmann::json sj;
        sj["remark"] = safe_str(s.remark, "sentiment_analysis.remark");
        sj["sentiment"] = safe_str(s.sentiment, "sentiment_analysis.sentiment");
        sj["score"] = s.score;
        senti_json.push_back(sj);
    }
    j["sentiment_analysis"] = senti_json;

    // 扩展字段：如AR模型参数等
    if (!extra_json.is_null() && !extra_json.empty()) {
        for (auto it = extra_json.begin(); it != extra_json.end(); ++it) {
            j[it.key()] = it.value();
        }
    }

    return j;
}
