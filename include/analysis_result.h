#pragma once
#include <string>
#include <vector>
#include <map>
#include "record.h"
#include <json.hpp>
#include "apriori.h"
#include "cluster_info.h"

struct AnalysisResult {
    // 可选：扩展字段，便于输出额外模型信息
    nlohmann::json extra_json;
    std::string lang;
    std::string generated_time;
    size_t total_records;
    double total_amount;
    double avg_amount;
    double min_amount;
    double max_amount;
    double median_amount;
    double stddev_amount;
    std::map<std::string, double> category_total;
    std::vector<std::string> anomalies;


    // 新增：多维聚类结果
    std::vector<ClusterInfo> clusters;
    // 新增：用户画像
    struct UserProfile {
        std::string user_id;
        std::string label;
        std::map<std::string, double> features;
        // 可扩展更多画像特征
    };
    std::vector<UserProfile> user_profiles;

    // 新增：时序分析
    struct TimeSeriesPoint {
        std::string date;
        double value;
    };
    std::vector<TimeSeriesPoint> time_series;


    // 新增：关联规则
    std::vector<AssociationRule> association_rules;

    // 新增：情感分析
    struct SentimentResult {
        std::string remark;
        std::string sentiment; // positive/negative/neutral
        double score = 0.0;
    };
    std::vector<SentimentResult> sentiment_analysis;

    nlohmann::json to_json() const;
};



