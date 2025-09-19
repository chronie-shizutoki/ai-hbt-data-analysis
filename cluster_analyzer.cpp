#include "include/cluster_analyzer.h"
#include <random>
#include <limits>
#include <cmath>
#include <numeric>

// 计算两个记录之间的距离（这里简化为金额的欧氏距离）
static double calculate_distance(const Record& r1, const Record& r2) {
    return std::abs(r1.amount - r2.amount);
}

std::vector<ClusterInfo> ClusterAnalyzer::kmeans_cluster(const std::vector<Record>& records, int num_clusters) {
    std::vector<ClusterInfo> clusters(num_clusters);
    if (records.empty() || num_clusters <= 0) return clusters;

    // 1. 初始化质心：随机选择 num_clusters 个记录作为初始质心
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, records.size() - 1);

    std::vector<Record> centroids(num_clusters);
    std::vector<int> assigned_cluster(records.size());

    for (int i = 0; i < num_clusters; ++i) {
        centroids[i] = records[distrib(gen)];
    }

    bool changed = true;
    int max_iterations = 100; // 防止无限循环
    int iteration = 0;

    while (changed && iteration < max_iterations) {
        changed = false;
        iteration++;

        // 2. 分配阶段：将每个记录分配到最近的质心
        for (size_t i = 0; i < records.size(); ++i) {
            double min_dist = std::numeric_limits<double>::max();
            int closest_cluster = -1;

            for (int j = 0; j < num_clusters; ++j) {
                double dist = calculate_distance(records[i], centroids[j]);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_cluster = j;
                }
            }

            if (closest_cluster != assigned_cluster[i]) {
                assigned_cluster[i] = closest_cluster;
                changed = true;
            }
        }

        // 如果没有记录改变所属集群，则收敛
        if (!changed) break;

        // 3. 更新阶段：重新计算每个集群的质心
        std::vector<double> new_centroids_amount(num_clusters, 0.0);
        std::vector<int> cluster_member_counts(num_clusters, 0);

        for (int i = 0; i < num_clusters; ++i) {
            clusters[i].member_indices.clear(); // 清空旧的成员
        }

        for (size_t i = 0; i < records.size(); ++i) {
            int cluster_idx = assigned_cluster[i];
            if (cluster_idx != -1) {
                new_centroids_amount[cluster_idx] += records[i].amount;
                cluster_member_counts[cluster_idx]++;
                clusters[cluster_idx].member_indices.push_back(i);
            }
        }

        for (int i = 0; i < num_clusters; ++i) {
            if (cluster_member_counts[i] > 0) {
                centroids[i].amount = new_centroids_amount[i] / cluster_member_counts[i];
            } else {
                // 如果某个集群为空，重新随机选择一个记录作为质心
                centroids[i] = records[distrib(gen)];
            }
        }
    }

    // 填充最终的集群信息
    for (int i = 0; i < num_clusters; ++i) {
        clusters[i].cluster_total = 0.0;
        for (size_t record_idx : clusters[i].member_indices) {
            clusters[i].cluster_total += records[record_idx].amount;
        }
        clusters[i].avg_amount = clusters[i].member_indices.empty() ? 0.0 : clusters[i].cluster_total / clusters[i].member_indices.size();
        clusters[i].label = "Cluster " + std::to_string(i + 1);
    }

    return clusters;
}


