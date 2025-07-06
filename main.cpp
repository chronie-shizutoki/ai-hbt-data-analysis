#include "include/csv_parser.h"
#include "include/stats.h"
#include "include/report.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        std::cout << "请输入CSV文件名: ";
        std::getline(std::cin, filename);
    }
    if (!fs::exists(filename)) {
        if (!fs::exists(filename + ".csv")) {
            std::cerr << "错误: 文件不存在 - " << filename << std::endl;
            return 1;
        }
        filename += ".csv";
    }
    auto records = parse_csv(filename);
    if (records.empty()) {
        std::cout << "未找到有效记录" << std::endl;
        return 2;
    }
    std::map<std::string, Stats> type_stats, product_stats, country_stats, monthly_stats, unit_price_stats;
    Stats global_stats;
    compute_stats(records, type_stats, product_stats, country_stats, monthly_stats, unit_price_stats, global_stats);
    generate_report(records, global_stats, type_stats, product_stats, country_stats, monthly_stats, unit_price_stats);
    std::cout << "高级分析报告已生成: 高级消费分析报告.txt" << std::endl;
    return 0;
}
