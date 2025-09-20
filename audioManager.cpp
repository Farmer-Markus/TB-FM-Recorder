#include "audioManager.h"
#include "requester.h"

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <strings.h>

namespace fs = std::filesystem;



AudioManager::AudioManager(const fs::path destFile, const std::string url, const size_t bufferSize) : m_bufferSize(bufferSize) {
    m_of.open(destFile, std::ios::binary);
    if(!m_of.is_open())
        throw std::runtime_error("Failed to open destination file");


    std::string subUrl;
    Requester::splitUrl(&m_hostUrl, &subUrl, url);
    prepareRequest(m_hostUrl, subUrl);

    m_requester.prepare(m_hostUrl.c_str());
    m_requester.sendRequest(m_request.c_str());

    m_buffer = new char[bufferSize];
}

AudioManager::~AudioManager() {
    if(m_buffer != nullptr)
        delete[] m_buffer;

    if(m_of.is_open())
        m_of.close();
}

void AudioManager::prepareRequest(std::string& hostUrl, std::string& subUrl) {
    // Build request string
    m_request = "GET /";
    m_request.append(subUrl);
    m_request.append(
        " HTTP/1.1\r\n"
        "Host: "
    );
    m_request.append(hostUrl);
    m_request.append(
        "\r\n"
        "Connection: close\r\n\r\n"
    );
}

bool AudioManager::tick() {
    int recBytes;

    // Try to reconnect (5 times)
    for(size_t i = 0; i < 6; i++) {
        if(m_requester.receive(&recBytes, m_buffer, m_bufferSize)) {
            break;
        }
        if(i > 5)
            return false;

        m_requester = Requester();
        try {
            m_requester.prepare(m_hostUrl.c_str());
        } catch(std::runtime_error) {
            continue;
        }
        
        m_requester.sendRequest(m_request.c_str());
    }

    if(!m_initialized) {
        // ignore header...
        size_t i;
        uint8_t count = 0; 
        // Searches for more than 5 * 00 bytes (end of header)
        for(i = 0; i < recBytes; i++) {
            if(m_buffer[i] == 0x00) {
                count++;
            } else if(count > 5) {
                break;
            } else {
                count = 0;
            }
        }

        toFile(&m_buffer[i], recBytes - i);

        m_initialized = true;

    } else {
        toFile(m_buffer, recBytes);
    }

    return true;
}

bool AudioManager::toFile(char* buffer, size_t numBytes) {
    if(!m_of.is_open())
        return false;

    m_of.write(buffer, numBytes);
    if(!m_of) {
        return false;
    }

    return true;
}