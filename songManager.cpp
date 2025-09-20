#include "songManager.h"

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "http.h"
#include "requester.h"
#include "json.h"
#include "timer.h"

namespace fs = std::filesystem;


SongManager::SongManager(const fs::path destFile, const std::string url, const size_t bufferSize) : m_bufferSize(bufferSize) {
    std::string subUrl;
    Requester::splitUrl(&m_hostUrl, &subUrl, url);

    prepareRequest(m_hostUrl, subUrl);

    m_of.open(destFile, std::ios::out);
    m_buffer = new char[bufferSize];
}

SongManager::~SongManager() {
    if(m_of.is_open())
        m_of.close();

    if(m_buffer != nullptr)
        delete[] m_buffer;
}


void SongManager::setDjCondition(const std::string dj) {
    std::cout << "Set dj to:" << dj << "\n";
    djContition = dj;
}

void SongManager::setMinTimeCondition(const size_t minutes) {
    std::cout << "Set MIN to:" << std::to_string(minutes) << "\n";
    minTimeCondition = minutes;
}

void SongManager::setMaxTimeCondition(const size_t minutes) {
    std::cout << "Set MAX to:" << std::to_string(minutes) << "\n";
    maxTimeCondition = minutes;
}

void SongManager::prepareRequest(std::string& hostUrl, std::string& subUrl) {
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

bool SongManager::buildSongText(std::string* finalText, Json& json) {
    std::string songArtist, songTitle, remixTitle;

    songArtist = json["data"]["artist"].string();
    songTitle = json["data"]["title"].string();

    if(songArtist.empty() && songTitle.empty()) {
        std::cerr << "Could not get live TB infos!\n";
        return false;
    }
    remixTitle = json["data"]["remix"].string();

    std::tm tm = Timer::sysTime();

    finalText->append(std::to_string(tm.tm_mday));
    finalText->append(".");
    finalText->append(std::to_string(tm.tm_mon + 1));
    finalText->append(".");
    finalText->append(std::to_string(tm.tm_year + 1900));
    finalText->append(" ");
    finalText->append(Timer::sysTimeFancy());
    finalText->append(" | New Song: ");
    finalText->append(songTitle);
    finalText->append(" by: ");
    finalText->append(songArtist);

    if(!remixTitle.empty()) {
        finalText->append(" (remix: ");
        finalText->append(remixTitle);
        finalText->append(")");
    }

    int h,m;
    Timer::steadyToHS(&h, &m, Timer::timeRunning);

    finalText->append(" | Timestamp: ");
    finalText->append(Timer::steadyToHSFancy(Timer::timeRunning));
    finalText->append("\n");


    return true;
}

bool SongManager::buildDjText(std::string* finalText, Json& json) {
    bool live = false;
    std::string dj, showName;
    try {
        if(json["data"]["liveBroadcast"]["onAir"].string() == "true") {
            live = true;
        }
    } catch(std::runtime_error) {}

    if(json["data"]["liveBroadcast"]["onAir"]) {
        try {
            dj = json["data"]["liveBroadcast"]["nickName"].string();
            showName = json["data"]["liveBroadcast"]["showName"].string();

        } catch(std::runtime_error e) {
            std::cerr << "Could not get live TB infos!\n";
            return false;
        }
    } else {
        try {
            dj = "Offline playlist";
            showName = "Playlist";
            
        } catch(std::runtime_error e) {
            std::cerr << "Could not get TB infos!\n";
            return false;
        }
    }

    std::tm tm = Timer::sysTime();

    finalText->append(std::to_string(tm.tm_mday));
    finalText->append(".");
    finalText->append(std::to_string(tm.tm_mon + 1));
    finalText->append(".");
    finalText->append(std::to_string(tm.tm_year + 1900));
    finalText->append(" ");
    finalText->append(Timer::sysTimeFancy());
    finalText->append(" | DJ change: ");
    finalText->append(dj);

    int h,m;
    Timer::steadyToHS(&h, &m, Timer::timeRunning);

    finalText->append(" | Timestamp: ");
    finalText->append(Timer::steadyToHSFancy(Timer::timeRunning));
    finalText->append("\n");


    return true;
}

bool SongManager::update() {
    if(!m_of.is_open())
        return false;

    // Check conditions
    size_t timeRunning = Timer::steadyToMin(Timer::timeRunning);
    if(minTimeCondition < timeRunning) {
        if(maxTimeCondition > 0 && maxTimeCondition < timeRunning) {
            finished = true;

        } else if(m_json_old["data"]["liveBroadcast"]["nickName"].string() != djContition) {
            finished = true;
        }
    }

    Requester requester;
    try {
        requester.prepare(m_hostUrl.c_str());
    } catch(std::runtime_error e) {
        return false;
    }
    
    requester.sendRequest(m_request.c_str());
    memset(m_buffer, 0, m_bufferSize);
    int recBytes;
    if(!requester.receive(&recBytes, m_buffer, m_bufferSize))
        return false;

    Http http;
    http.parse(std::string(m_buffer));
    Json js;
    try {
        js.parse(http);
    } catch(std::out_of_range) {
        return false;
    }

    std::string finalText;
    finalText.reserve(1026);

    // Songs/dj ... has changed
    if(js != m_json_old) {
        bool songChange = true;
        std::string oldDj, newDj;
        try {
            oldDj = m_json_old["data"]["liveBroadcast"]["nickName"].string();
            newDj = js["data"]["liveBroadcast"]["nickName"].string();
            if(oldDj != newDj)
                songChange = false;

        } catch(std::out_of_range) {
            songChange = false;
        }

        if(!songChange) { // DJ change!
            buildDjText(&finalText, js);

        } else { // Song change?
            std::string oldSng, newSng;
            try {
                oldSng = m_json_old["data"]["title"].string();
                newSng = js["data"]["title"].string();
                if(oldSng != newSng)
                    songChange = true;

            } catch(std::out_of_range) {
                songChange = true;
            }

            if(!songChange) {
                m_json_old = js;
                return true;
            }

            buildSongText(&finalText, js);
        }

        m_json_old = js;
    }

    if(!toFile(finalText.data(), finalText.size()))
        return false;

    return true;
}

bool SongManager::toFile(char* buffer, size_t numBytes) {
    if(!m_of.is_open())
        return false;

    m_of.write(buffer, numBytes);
    if(!m_of) {
        return false;
    }
    m_of.flush();

    return true;
}