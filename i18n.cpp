#include "include/i18n.h"
#include <json.hpp>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

bool I18N::load(const std::string& lang_file) {
    std::ifstream f(lang_file);
    if (!f.is_open()) return false;
    json j;
    f >> j;
    dict.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        dict[it.key()] = it.value();
    }
    return true;
}

std::string I18N::t(const std::string& key) const {
    auto it = dict.find(key);
    if (it != dict.end()) return it->second;
    return key;
}
