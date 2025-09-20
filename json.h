#pragma once
#include <cstddef>
#include <cstdlib>
#include <string>
#include <unordered_map>


// A shitty json reading implementation
class Json {
    std::string content;

public:

    Json& operator[](const std::string& key) {
        return data[key];
    }

    Json& operator[](const char* key) {
        return data[key]; //(*this)[std::string(key)];
    }

    void operator =(const std::string val) {
        content = val;
    }

    void operator =(const char* val) {
        content = std::string(val);
    }

    operator std::string() const {
        return content;
    }

    operator bool() const {
        if(content == "true") {
            return true;
        } else {
            return false;
        }
    }

    bool operator==(const Json& json) const {
        return (data == json.data) && (content == json.content);
    }

    bool operator!=(const Json& json) const {
        return (data != json.data) || (content != json.content);
    }

    std::string string() const {
        return content;
    }


    bool parse(std::string src);
    size_t parsePart(Json& json, std::string src, size_t offset = 0);

    std::unordered_map<std::string, Json> data;

private:


};