#include "include/csv_parser.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>
#include <iomanip>

// 解析备注，提取数量、原产国、产品名等
static void parse_remark(Record& record) {
    std::string& remark = record.remark;
    record.is_blacklist = (remark.find("黑名单") != std::string::npos);
    record.is_imported = (remark.find("进口") != std::string::npos);
    // 数量
    size_t star_pos = remark.find_last_of('*');
    if (star_pos != std::string::npos && star_pos + 1 < remark.size()) {
        std::string qty_str = remark.substr(star_pos + 1);
        try {
            record.quantity = std::stoi(qty_str);
            remark = remark.substr(0, star_pos);
        } catch (...) {}
    }
    // 原产国
    size_t open_paren = remark.find("（");
    size_t close_paren = remark.find("）", open_paren);
    if (open_paren != std::string::npos && close_paren != std::string::npos && open_paren < close_paren) {
        record.origin_country = remark.substr(open_paren + 3, close_paren - open_paren - 3); // UTF-8下每个全角括号3字节
        remark = remark.substr(0, open_paren) + remark.substr(close_paren + 3);
    }
    // 产品名
    size_t dash_pos = remark.find('-');
    if (dash_pos != std::string::npos) {
        record.product_name = remark.substr(0, dash_pos);
        remark = remark.substr(dash_pos + 1);
    } else {
        std::regex chinese_re(R"([\p{Han}]+)");
        std::smatch match;
        if (std::regex_search(remark, match, chinese_re)) {
            record.product_name = match.str();
        } else {
            record.product_name = remark;
        }
    }
    record.unit_price = (record.quantity > 0) ? record.amount / record.quantity : record.amount;
}

// 解析时间
static void parse_time(Record& record) {
    std::istringstream ss(record.time);
    ss >> std::get_time(&record.time_tm, "%Y-%m-%d");
    time_t t = mktime(&record.time_tm);
    tm* local_tm = localtime(&t);
    if (local_tm) record.time_tm = *local_tm;
}

std::vector<Record> parse_csv(const std::string& filename) {
    std::vector<Record> records;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return records;
    }
    std::string line;
    getline(file, line); // 跳过标题
    int line_num = 1;
    while (getline(file, line)) {
        line_num++;
        if (line.empty()) continue;
        if (line.find('"') != std::string::npos) {
            line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
        }
        std::stringstream ss(line);
        Record record;
        std::string amount_str;
        if (!getline(ss, record.type, ',')) continue;
        if (!getline(ss, record.remark, ',')) continue;
        if (!getline(ss, amount_str, ',')) continue;
        if (!getline(ss, record.time, ',')) {
            // 兼容部分行无多余字段
            if (!getline(ss, record.time)) continue;
        }
        // 兼容多余字段
        getline(ss, record.extra);

        // --- 修正：去除所有字段首尾空白和不可见字符 ---
        auto trim = [](std::string& s) {
            size_t start = s.find_first_not_of(" \t\r\n\v\f");
            if (start == std::string::npos) {
                s.clear();
                return;
            }
            size_t end = s.find_last_not_of(" \t\r\n\v\f");
            if (end == std::string::npos || end < start) {
                s.clear();
                return;
            }
            s = s.substr(start, end - start + 1);
        };
        trim(record.type);
        trim(record.remark);
        trim(amount_str);
        trim(record.time);
        trim(record.extra);

        // 检查所有字段是否为有效UTF-8（简单过滤非法字节）
        auto is_valid_utf8 = [](const std::string& str) {
            int c,i,ix,n,j;
            for (i=0, ix=str.length(); i < ix; i++) {
                c = (unsigned char) str[i];
                if (0x00 <= c && c <= 0x7f) n=0; // 0bbbbbbb
                else if ((c & 0xE0) == 0xC0) n=1; // 110bbbbb
                else if ( c==0xed && i<(ix-1) && ((unsigned char)str[i+1] & 0xa0)==0xa0) return false; // 禁止代理区
                else if ((c & 0xF0) == 0xE0) n=2; // 1110bbbb
                else if ((c & 0xF8) == 0xF0) n=3; // 11110bbb
                else return false;
                for (j=0; j<n && i<ix; j++) {
                    if ((++i == ix) || (( (unsigned char)str[i] & 0xC0 ) != 0x80)) return false;
                }
            }
            return true;
        };
        if (!is_valid_utf8(record.type) || !is_valid_utf8(record.remark) || !is_valid_utf8(record.time) || !is_valid_utf8(amount_str)) {
            std::cerr << "警告: 第" << line_num << "行存在非法UTF-8字符，已跳过。" << std::endl;
            continue;
        }

        try {
            record.amount = std::stod(amount_str);
        } catch (...) {
            std::cerr << "警告: 第" << line_num << "行金额解析失败: " << amount_str << std::endl;
            continue;
        }
        parse_remark(record);
        try {
            parse_time(record);
        } catch (...) {
            std::cerr << "警告: 第" << line_num << "行时间解析失败: " << record.time << std::endl;
        }
        records.push_back(record);
    }
    return records;
}
