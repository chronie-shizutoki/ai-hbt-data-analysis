#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>

// 定义关联规则结构
struct AssociationRule {
    std::vector<std::string> antecedent; // 前项
    std::vector<std::string> consequent; // 后项
    double support;   // 支持度
    double confidence; // 置信度
    double lift;      // 提升度
};

// Apriori算法实现
std::vector<AssociationRule> run_apriori(
    const std::vector<std::vector<std::string>>& transactions,
    double min_support,
    double min_confidence
);


