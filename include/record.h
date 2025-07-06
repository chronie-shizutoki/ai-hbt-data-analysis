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
};
