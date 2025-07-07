#include "include/analysis_result.h"
#include <json.hpp>
#include <iostream>

nlohmann::json AnalysisResult::to_json() const {
    // 工具函数：trim和UTF-8校验
    auto trim = [](const std::string& s) -> std::string {
        size_t start = s.find_first_not_of(" \t\r\n\v\f");
        size_t end = s.find_last_not_of(" \t\r\n\v\f");
        if (start == std::string::npos || end == std::string::npos) return "";
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
    // ...可扩展更多分析字段...
    return j;
}
