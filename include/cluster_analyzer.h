#pragma once
#include <vector>
#include <string>
#include "record.h"
#include "cluster_info.h"

// 聚类分析器类
class ClusterAnalyzer {
public:
    // 执行KMeans聚类
    std::vector<ClusterInfo> kmeans_cluster(const std::vector<Record>& records, int num_clusters);
};


