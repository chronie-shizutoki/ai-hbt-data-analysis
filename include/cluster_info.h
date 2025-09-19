#pragma once
#include <string>
#include <vector>

// 定义聚类结果结构
struct ClusterInfo {
    std::string label;
    std::vector<size_t> member_indices; // 指向原始记录的下标
    double cluster_total = 0.0;
    double avg_amount = 0.0;
    // 可扩展更多特征
};


