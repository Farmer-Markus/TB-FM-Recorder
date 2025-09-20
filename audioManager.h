#pragma once
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <string>
#include "requester.h"

namespace fs = std::filesystem;

class AudioManager {
public:
    AudioManager(const fs::path destFile, const std::string url, const size_t bufferSize = 4096);
    ~AudioManager();
    bool tick();


private:
    void prepareRequest(std::string& hostUrl, std::string& subUrl);
    bool toFile(char* buffer, size_t numBytes);

    std::string m_hostUrl;
    bool m_initialized = false;
    size_t m_bufferSize;
    char* m_buffer = nullptr;
    std::string m_request;
    
    Requester m_requester;
    std::ofstream m_of;
};