#include "include/csv_parser.h"
#include "include/stats.h"
#include "include/report.h"
#include "include/complex_analyzer.h"
#include "include/analysis_result.h"
#include "include/i18n.h"
#include <iostream>
#include <filesystem>
#include <json.hpp>
#include <fstream>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string filename;
    std::string lang = "zh_CN";
    std::string out_json = "analysis.json";
    // 完整命令行参数解析，支持任意顺序和国际化
    std::string next_opt;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (!next_opt.empty()) {
            if (next_opt == "lang") lang = arg;
            else if (next_opt == "output") out_json = arg;
            next_opt.clear();
            continue;
        }
        if (arg == "--lang" || arg == "-l") {
            next_opt = "lang";
        } else if (arg == "-o" || arg == "--output") {
            next_opt = "output";
        } else if (!arg.empty() && arg[0] != '-' && filename.empty()) {
            filename = arg;
        }
    }
    // 交互式输入
    if (filename.empty()) {
        std::cout << (lang == "en_US" ? "Please enter CSV filename: " : "请输入CSV文件名: ");
        std::getline(std::cin, filename);
    }
    if (!fs::exists(filename)) {
        if (!fs::exists(filename + ".csv")) {
            std::cerr << (lang == "en_US" ? "Error: File not found - " : "错误: 文件不存在 - ") << filename << std::endl;
            return 1;
        }
        filename += ".csv";
    }
    I18N i18n;
    if (!i18n.load("lang/" + lang + ".json")) {
        std::cerr << (lang == "en_US" ? "Failed to load language pack: " : "语言包加载失败: ") << lang << std::endl;
        return 1;
    }
    auto records = parse_csv(filename);
    if (records.empty()) {
        std::cout << i18n.t("未找到有效记录") << std::endl;
        return 2;
    }
    // 复杂分析
    AnalysisResult result = complex_analysis(records, i18n);
    // 输出JSON
    std::ofstream jout(out_json);
    jout << result.to_json().dump(2);
    jout.close();
    std::cout << i18n.t("分析已完成，结果已输出到 ") << out_json << std::endl;

    // 统计信息
    Stats global_stats;
    std::map<std::string, Stats> type_stats, product_stats, country_stats, monthly_stats, unit_price_stats;
    compute_stats(records, type_stats, product_stats, country_stats, monthly_stats, unit_price_stats, global_stats);
    // 输出国际化文本报告
    generate_report_i18n(records, global_stats, type_stats, product_stats, country_stats, monthly_stats, unit_price_stats, i18n, "report.txt");

    return 0;
}
