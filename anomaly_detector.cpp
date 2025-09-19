#include "include/anomaly_detector.h"
#include <algorithm>
#include <random>
#include <set>

// 简单的异常检测实现，模拟 Isolation Forest 的思想
// 通过随机划分数据，异常点通常更容易被“隔离”
std::vector<size_t> AnomalyDetector::detect(const std::vector<Record>& records, double contamination) {
    std::vector<size_t> anomalies_indices;
    if (records.empty() || contamination <= 0 || contamination >= 1) {
        return anomalies_indices;
    }

    // 提取金额数据
    std::vector<double> amounts;
    for (const auto& r : records) {
        amounts.push_back(r.amount);
    }

    // 计算异常点的数量
    size_t num_anomalies = static_cast<size_t>(records.size() * contamination);
    if (num_anomalies == 0 && records.size() > 0) num_anomalies = 1; // 至少检测一个
    if (num_anomalies >= records.size()) num_anomalies = records.size() - 1; // 不能所有都是异常

    // 使用一个简单的启发式方法：金额离群值
    // 这里简化为找出金额最大（或最小）的N个点作为异常
    // 更复杂的实现会涉及随机树构建和路径长度计算

    // 创建 (amount, index) 对，方便排序后获取原始索引
    std::vector<std::pair<double, size_t>> indexed_amounts;
    for (size_t i = 0; i < amounts.size(); ++i) {
        indexed_amounts.push_back({amounts[i], i});
    }

    // 按金额降序排序
    std::sort(indexed_amounts.begin(), indexed_amounts.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    // 取前 num_anomalies 个作为异常点
    for (size_t i = 0; i < num_anomalies; ++i) {
        anomalies_indices.push_back(indexed_amounts[i].second);
    }

    // 也可以考虑金额最小的作为异常，或者结合均值和标准差
    // 例如：金额小于 (均值 - 2*标准差) 的也认为是异常
    // 这里为了简化，只取最大值作为异常

    return anomalies_indices;
}


