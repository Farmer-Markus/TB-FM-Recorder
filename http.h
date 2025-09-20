#pragma once
#include <unordered_map>
#include <vector>
#include <string>


// Another shitty parser written by me, Farmer_Markus... yay
class Http {
    std::unordered_map<std::string, std::vector<std::string>> m_data;
    std::string m_httpContent;

public:
    std::vector<std::string> operator[](const std::string& key) {
        return m_data[key];
    }

    std::vector<std::string> operator[](const char* key) {
        return m_data[std::string(key)];
    }

    operator std::string() const {
        return m_httpContent;
    }

    std::string content() {
        return m_httpContent;
    }
    void parse(const std::string& httpStr);


private:
    void readContent(const std::string& httpStr, size_t offset);
    int hexStringToInt(const std::string& hex);




};