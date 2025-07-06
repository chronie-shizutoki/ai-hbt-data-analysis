#pragma once
#include <string>
#include <map>

class I18N {
public:
    bool load(const std::string& lang_file);
    std::string t(const std::string& key) const;
private:
    std::map<std::string, std::string> dict;
};
