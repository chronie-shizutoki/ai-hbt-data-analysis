#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <numeric>
#include <regex>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// 数据结构定义
struct Record {
    string type;
    string remark;
    double amount;
    string time;
    string product_name;
    string origin_country;
    int quantity = 1;
    bool is_blacklist = false;
    bool is_imported = false;
    double unit_price = 0.0;
    tm time_tm = {};
};

// 为Stats结构体添加比较运算符
struct Stats {
    double total = 0.0;
    double avg = 0.0;
    int count = 0;
    double min = 1e9;
    double max = 0.0;
    vector<double> values;
    
    void add_value(double value) {
        total += value;
        count++;
        values.push_back(value);
        avg = total / count;
        if (value < min) min = value;
        if (value > max) max = value;
    }
    
    double median() const {
        if (values.empty()) return 0.0;
        vector<double> sorted = values;
        sort(sorted.begin(), sorted.end());
        size_t n = sorted.size() / 2;
        if (sorted.size() % 2 == 0) {
            return (sorted[n-1] + sorted[n]) / 2.0;
        }
        return sorted[n];
    }
    
    double std_dev() const {
        if (values.size() < 2) return 0.0;
        double variance = 0.0;
        for (double v : values) {
            variance += pow(v - avg, 2);
        }
        return sqrt(variance / (values.size() - 1));
    }
    
    // 添加比较运算符
    bool operator<(const Stats& other) const {
        return total < other.total;
    }
};

// 情感分析关键词
const set<string> NEGATIVE_WORDS = {"差", "不好", "黑名单", "极差", "烂", "差劲"};
const set<string> IMPORT_KEYWORDS = {"进口", "进口品"};

// 文本挖掘解析函数
void parse_remark(Record& record) {
    string& remark = record.remark;
    
    // 检测黑名单
    record.is_blacklist = (remark.find("黑名单") != string::npos);
    
    // 检测进口商品
    record.is_imported = (remark.find("进口") != string::npos);
    
    // 提取数量 (格式: *数字)
    size_t star_pos = remark.find_last_of('*');
    if (star_pos != string::npos && star_pos + 1 < remark.size()) {
        string qty_str = remark.substr(star_pos + 1);
        try {
            record.quantity = stoi(qty_str);
            remark = remark.substr(0, star_pos); // 移除数量部分
        } catch (...) {}
    }
    
    // 提取原产国 (格式: (国家))
    size_t open_paren = remark.find('(');
    size_t close_paren = remark.find(')', open_paren);
    if (open_paren != string::npos && close_paren != string::npos && open_paren < close_paren) {
        record.origin_country = remark.substr(open_paren + 1, close_paren - open_paren - 1);
        remark = remark.substr(0, open_paren) + remark.substr(close_paren + 1);
    }
    
    // 提取产品名称 (第一个'-'之前的内容)
    size_t dash_pos = remark.find('-');
    if (dash_pos != string::npos) {
        record.product_name = remark.substr(0, dash_pos);
        remark = remark.substr(dash_pos + 1);
    } else {
        // 尝试从开头提取直到遇到非中文字符
        regex chinese_re(R"([\p{Han}]+)");
        smatch match;
        if (regex_search(remark, match, chinese_re)) {
            record.product_name = match.str();
        } else {
            record.product_name = remark;
        }
    }
    
    // 计算单价
    record.unit_price = (record.quantity > 0) ? record.amount / record.quantity : record.amount;
}

// 时间解析函数
void parse_time(Record& record) {
    istringstream ss(record.time);
    ss >> get_time(&record.time_tm, "%Y-%m-%d");
    // 计算星期几
    time_t t = mktime(&record.time_tm);
    tm* local_tm = localtime(&t);
    if (local_tm) {
        record.time_tm = *local_tm;
    }
}

string extract_month(const Record& record) {
    ostringstream oss;
    oss << put_time(&record.time_tm, "%Y-%m");
    return oss.str();
}

string extract_weekday(const Record& record) {
    const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
    return weekdays[record.time_tm.tm_wday];
}

// 情感分析函数
string sentiment_analysis(const string& remark) {
    for (const auto& word : NEGATIVE_WORDS) {
        if (remark.find(word) != string::npos) {
            return "负面";
        }
    }
    return "中性/正面";
}

// 价格弹性分析
void price_elasticity_analysis(const map<string, Stats>& product_stats, 
                               const map<string, Stats>& unit_price_stats,
                               ostream& report) {
    report << "==================== 价格弹性分析 ====================\n";
    report << "产品\t平均单价\t总销量\t价格弹性\n";
    
    for (const auto& [product, stat] : product_stats) {
        if (unit_price_stats.find(product) == unit_price_stats.end()) continue;
        
        double avg_price = unit_price_stats.at(product).avg;
        double total_quantity = stat.count;
        
        // 简单价格弹性计算 (需更多数据点才准确)
        double elasticity = 0.0;
        if (avg_price > 0 && total_quantity > 0) {
            // 假设价格每增加1元，销量减少0.5% (示例逻辑)
            elasticity = -0.5 * (avg_price / 10.0);
        }
        
        report << product << "\t" << fixed << setprecision(2) << avg_price << "\t"
               << total_quantity << "\t" << elasticity << "\n";
    }
}

// 消费模式识别
void consumption_pattern_analysis(const vector<Record>& records, ostream& report) {
    report << "==================== 消费模式识别 ====================\n";
    
    // 1. 黑名单分析
    int blacklist_count = 0;
    double blacklist_total = 0.0;
    vector<string> blacklist_products;
    
    // 2. 进口商品分析
    int imported_count = 0;
    double imported_total = 0.0;
    
    // 3. 按时间段分析
    map<string, int> weekday_count;
    map<string, double> weekday_amount;
    
    // 4. 情感分析
    map<string, int> sentiment_count;
    
    for (const auto& r : records) {
        if (r.is_blacklist) {
            blacklist_count++;
            blacklist_total += r.amount;
            blacklist_products.push_back(r.product_name);
        }
        
        if (r.is_imported) {
            imported_count++;
            imported_total += r.amount;
        }
        
        string weekday = extract_weekday(r);
        weekday_count[weekday]++;
        weekday_amount[weekday] += r.amount;
        
        string sentiment = sentiment_analysis(r.remark);
        sentiment_count[sentiment]++;
    }
    
    // 黑名单报告
    report << "1. 黑名单消费分析:\n";
    report << "   - 黑名单商品数量: " << blacklist_count << " 个 (" 
           << fixed << setprecision(1) << (blacklist_count * 100.0 / records.size()) << "%)\n";
    report << "   - 黑名单消费总额: " << blacklist_total << " 元\n";
    if (!blacklist_products.empty()) {
        report << "   - 主要黑名单商品: ";
        for (const auto& p : blacklist_products) {
            report << p << ", ";
        }
        report << "\n";
    }
    
    // 进口商品报告
    report << "\n2. 进口商品分析:\n";
    report << "   - 进口商品数量: " << imported_count << " 个\n";
    report << "   - 进口商品消费占比: " << fixed << setprecision(1) 
           << (imported_total * 100.0 / accumulate(records.begin(), records.end(), 0.0, 
                [](double sum, const Record& r) { return sum + r.amount; })) 
           << "%\n";
    
    // 时间段分析
    report << "\n3. 消费时间分布 (按星期):\n";
    const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
    for (const char* day : weekdays) {
        if (weekday_count.find(day) != weekday_count.end()) {
            report << "   - 星期" << day << ": " << weekday_count[day] << " 笔交易, "
                   << "总额 " << weekday_amount[day] << " 元, "
                   << "平均 " << weekday_amount[day] / weekday_count[day] << " 元/笔\n";
        }
    }
    
    // 情感分析报告
    report << "\n4. 消费评价情感分析:\n";
    for (const auto& [sentiment, count] : sentiment_count) {
        report << "   - " << sentiment << "评价: " << count << " 条 ("
               << fixed << setprecision(1) << (count * 100.0 / records.size()) << "%)\n";
    }
}

// 主分析函数
void analyze_data(const vector<Record>& records) {
    // 基础统计
    Stats global_stats;
    for (const auto& r : records) {
        global_stats.add_value(r.amount);
    }

    // 按类别统计
    map<string, Stats> type_stats;
    // 按产品统计
    map<string, Stats> product_stats;
    // 按原产国统计
    map<string, Stats> country_stats;
    // 按月统计
    map<string, Stats> monthly_stats;
    // 单价统计
    map<string, Stats> unit_price_stats;

    for (const auto& r : records) {
        type_stats[r.type].add_value(r.amount);
        product_stats[r.product_name].add_value(r.amount);
        if (!r.origin_country.empty()) {
            country_stats[r.origin_country].add_value(r.amount);
        }
        monthly_stats[extract_month(r)].add_value(r.amount);
        unit_price_stats[r.product_name].add_value(r.unit_price);
    }

    // 生成报告
    ofstream report("高级消费分析报告.txt");
    
    // 获取当前时间
    time_t now = time(nullptr);
    tm* now_tm = localtime(&now);
    
    // 基础报告
    report << "==================== 高级消费数据分析报告 ====================\n";
    report << "分析时间: " << put_time(now_tm, "%Y-%m-%d %H:%M:%S") << "\n";
    report << "总记录数: " << records.size() << "\n";
    report << "总消费金额: " << fixed << setprecision(2) << global_stats.total << " 元\n";
    report << "单笔平均消费: " << global_stats.avg << " 元\n";
    report << "单笔最低消费: " << global_stats.min << " 元\n";
    report << "单笔最高消费: " << global_stats.max << " 元\n";
    report << "消费金额中位数: " << global_stats.median() << " 元\n";
    report << "消费金额标准差: " << global_stats.std_dev() << " 元 (波动率)\n\n";
    
    // 按类别统计 - 使用lambda排序
    report << "==================== 按消费类别分析 ====================\n";
    vector<pair<string, Stats>> sorted_types(type_stats.begin(), type_stats.end());
    sort(sorted_types.begin(), sorted_types.end(), 
        [](const auto& a, const auto& b) { 
            return a.second.total > b.second.total; 
        });
    
    for (const auto& [type, stat] : sorted_types) {
        report << "[" << type << "]\n";
        report << "  总消费: " << stat.total << " 元 (" 
               << fixed << setprecision(1) << (stat.total * 100.0 / global_stats.total) << "%)\n";
        report << "  交易次数: " << stat.count << "\n";
        report << "  平均消费: " << stat.avg << " 元/次\n";
        report << "  价格区间: " << stat.min << " - " << stat.max << " 元\n\n";
    }
    
    // 按产品统计 - 使用lambda排序
    report << "==================== 按产品分析 ====================\n";
    vector<pair<string, Stats>> sorted_products(product_stats.begin(), product_stats.end());
    sort(sorted_products.begin(), sorted_products.end(), 
        [](const auto& a, const auto& b) { 
            return a.second.total > b.second.total; 
        });
    
    for (const auto& [product, stat] : sorted_products) {
        report << "[" << product << "]\n";
        report << "  总消费: " << stat.total << " 元\n";
        report << "  购买次数: " << stat.count << "\n";
        
        if (unit_price_stats.count(product)) {
            auto& price_stat = unit_price_stats[product];
            report << "  平均单价: " << price_stat.avg << " 元\n";
            report << "  单价波动: ±" << price_stat.std_dev() << " 元\n";
        }
        report << "\n";
    }
    
    // 消费模式识别
    consumption_pattern_analysis(records, report);
    
    // 价格弹性分析
    price_elasticity_analysis(product_stats, unit_price_stats, report);
    
    // 月度趋势分析
    if (monthly_stats.size() > 1) {
        report << "\n==================== 月度消费趋势 ====================\n";
        vector<pair<string, Stats>> monthly_sorted(monthly_stats.begin(), monthly_stats.end());
        sort(monthly_sorted.begin(), monthly_sorted.end());
        
        for (size_t i = 0; i < monthly_sorted.size(); i++) {
            const auto& [month, stat] = monthly_sorted[i];
            report << month << ": " << stat.total << " 元 (" << stat.count << " 笔)";
            
            if (i > 0) {
                double prev_total = monthly_sorted[i-1].second.total;
                double change = (stat.total - prev_total) / prev_total * 100;
                report << " | 环比: " << (change >= 0 ? "+" : "") << fixed << setprecision(1) << change << "%";
            }
            
            // 消费构成分析
            map<string, double> type_contrib;
            for (const auto& r : records) {
                if (extract_month(r) == month) {
                    type_contrib[r.type] += r.amount;
                }
            }
            
            if (!type_contrib.empty()) {
                report << "\n   消费构成: ";
                for (const auto& [type, amount] : type_contrib) {
                    report << type << "(" << fixed << setprecision(0) 
                           << (amount * 100 / stat.total) << "%) ";
                }
            }
            
            report << "\n";
        }
    }
    
    // 优化建议
    report << "\n==================== 消费优化建议 ====================\n";
    
    // 黑名单建议
    int blacklist_count = count_if(records.begin(), records.end(), 
                                [](const Record& r) { return r.is_blacklist; });
    if (blacklist_count > 0) {
        report << "1. 减少黑名单商品消费:\n";
        report << "   - 您有 " << blacklist_count << " 笔黑名单商品消费，总额 "
               << accumulate(records.begin(), records.end(), 0.0, 
                  [](double sum, const Record& r) { 
                      return r.is_blacklist ? sum + r.amount : sum; 
                  }) << " 元\n";
        report << "   - 建议减少购买品质不佳的商品，优化消费选择\n";
    }
    
    // 奢侈品建议
    double luxury_threshold = global_stats.avg * 3;
    int luxury_count = count_if(records.begin(), records.end(), 
                             [&](const Record& r) { return r.unit_price > luxury_threshold; });
    if (luxury_count > 0) {
        report << "2. 奢侈品消费优化:\n";
        report << "   - 您有 " << luxury_count << " 笔奢侈品消费 (单价 > " 
               << luxury_threshold << " 元)\n";
        report << "   - 建议评估这些高价值商品的性价比和实际需求\n";
    }
    
    // 进口商品建议
    int imported_count = count_if(records.begin(), records.end(), 
                               [](const Record& r) { return r.is_imported; });
    if (imported_count > 0) {
        double imported_total = accumulate(records.begin(), records.end(), 0.0, 
                  [](double sum, const Record& r) { 
                      return r.is_imported ? sum + r.amount : sum; 
                  });
        double imported_percent = imported_total * 100 / global_stats.total;
        
        report << "3. 进口商品消费分析:\n";
        report << "   - 进口商品消费占比: " << fixed << setprecision(1) << imported_percent << "%\n";
        
        if (imported_percent > 30) {
            report << "   - 进口商品比例较高，可考虑部分替换为国产品牌以节省开支\n";
        } else {
            report << "   - 进口商品比例在合理范围，保持当前消费习惯\n";
        }
    }
    
    report << "\n==================== 报告结束 ====================\n";
    report.close();
    
    cout << "高级分析报告已生成: 高级消费分析报告.txt\n";
}

// 主程序
int main() {
    string filename;
    cout << "请输入CSV文件名: ";
    getline(cin, filename); // 使用getline处理可能包含空格的路径

    // 检查文件是否存在
    if (!fs::exists(filename)) {
        // 尝试添加.csv扩展名
        if (!fs::exists(filename + ".csv")) {
            cerr << "错误: 文件不存在 - " << filename << endl;
            return 1;
        }
        filename += ".csv";
    }

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return 1;
    }

    vector<Record> records;
    string line;
    
    // 跳过标题行
    getline(file, line);

    int line_num = 1;
    while (getline(file, line)) {
        line_num++;
        // 跳过空行
        if (line.empty()) continue;
        
        // 处理可能的引号
        if (line.find('"') != string::npos) {
            line.erase(remove(line.begin(), line.end(), '"'), line.end());
        }
        
        stringstream ss(line);
        Record record;
        string amount_str;
        
        // 解析CSV行
        if (!getline(ss, record.type, ',')) continue;
        if (!getline(ss, record.remark, ',')) continue;
        if (!getline(ss, amount_str, ',')) continue;
        if (!getline(ss, record.time)) continue;
        
        try {
            record.amount = stod(amount_str);
        } catch (...) {
            cerr << "警告: 第" << line_num << "行金额解析失败: " << amount_str << endl;
            continue; // 跳过无效行
        }
        
        // 文本挖掘
        parse_remark(record);
        
        // 时间解析
        try {
            parse_time(record);
        } catch (...) {
            cerr << "警告: 第" << line_num << "行时间解析失败: " << record.time << endl;
        }
        
        records.push_back(record);
    }

    if (records.empty()) {
        cout << "未找到有效记录" << endl;
        return 2;
    }

    analyze_data(records);
    
    return 0;
}