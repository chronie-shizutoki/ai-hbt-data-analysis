#pragma once
#include <string>
#include <ctime>

struct Record {
    std::string type;
    std::string remark;
    double amount;
    std::string time;
    std::string product_name;
    std::string origin_country;
    int quantity = 1;
    bool is_blacklist = false;
    bool is_imported = false;
    double unit_price = 0.0;
    tm time_tm = {};
    std::string extra;

    // 允许修改的构造函数
    Record() = default;
    Record(const std::string& t, const std::string& r, double amt, const std::string& tm_str, const std::string& pn, const std::string& oc, int qty, bool bl, bool imp, double up, const tm& t_tm, const std::string& ext)
        : type(t), remark(r), amount(amt), time(tm_str), product_name(pn), origin_country(oc), quantity(qty), is_blacklist(bl), is_imported(imp), unit_price(up), time_tm(t_tm), extra(ext) {}

    // 拷贝构造函数
    Record(const Record& other) = default;

    // 赋值运算符
    Record& operator=(const Record& other) = default;
};
