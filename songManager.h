#pragma once
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>

#include "json.h"

namespace fs = std::filesystem;


class SongManager {
public:
    SongManager(const fs::path destFile, const std::string url, const size_t bufferSize = 4096);
    ~SongManager();

    // Record until given dj stops playing
    void setDjCondition(const std::string dj);

    // Minimum time to record (minutes)
    void setMinTimeCondition(const size_t minutes);

    // Max amount of time to record (minutes)
    void setMaxTimeCondition(const size_t minutes);
    bool update();

    bool finished = false;

private:
    void prepareRequest(std::string& hostUrl, std::string& subUrl);
    bool toFile(char* buffer, size_t numBytes);
    bool buildSongText(std::string* finalText, Json& json);
    bool buildDjText(std::string* finalText, Json& json);

    size_t maxTimeCondition = 0;
    size_t minTimeCondition = 0;
    std::string djContition;


    std::string m_hostUrl;
    std::string m_request;
    char* m_buffer = nullptr;
    size_t m_bufferSize;

    Json m_json_old;
    std::ofstream m_of;
};