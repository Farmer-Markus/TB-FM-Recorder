#include "http.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


enum {
    NONE,
    HTTPVER, // HTTP version on top of response
    HEADER, // Read http header
    VALUE, // Read value for variable in http header
    CONTENTLENGHT, // Read length of content
    CONTENT // Read content itself
};

void Http::parse(const std::string& httpStr) {
    uint8_t mode = NONE;
    bool lineEnd = false;
    size_t httpLength = httpStr.length();
    size_t lastLine = 0;
    std::string buffer;
    std::vector<std::string>* currVar = nullptr;


    for(size_t i = 0; i < httpLength; i++) {
        char byte = httpStr.at(i);
        //std::cout << "Processing byte: " << byte << "\n";

        switch(byte) {
            case 0x0D: // Carriage Return "\r"
        
                // If in HEADER mode AND nextLine
                if(mode == HEADER || mode == VALUE && httpStr.at(i + 1) == 0x0A) { // Line Feed "\n"
                    if(lineEnd) { // If previously
                        mode = CONTENTLENGHT; // Next step is to read content lenght
                        readContent(httpStr, i + 2);
                        return;

                    } else {
                        lineEnd = true;
                        currVar->push_back(buffer);
                        buffer.clear();
                        mode = HEADER;
                    }

                } else if(mode == HTTPVER) {
                    if(currVar != nullptr) {
                        currVar->push_back(buffer);
                        buffer.clear();
                    }
                    currVar = nullptr;
                    mode = HEADER; // HTTP Version at top of file and now follows header
                }
                continue;

            case 0x0A: // Line Feed "\n"
                continue;
            
            case ';':
                if(mode == VALUE && currVar != nullptr) {
                    currVar->push_back(buffer);
                    buffer.clear();
                    continue;
                }
                break;
            
            case ':':
                if(mode == HEADER) {
                    currVar = &m_data[buffer];
                    buffer.clear();
                    mode = VALUE;
                    i++; // Skip empty space
                    continue;
                }
                break;
            
            case '/':
                if(mode == HTTPVER) {
                    buffer.clear();
                    continue; // Skip '/' at HTTP/1.1
                } 
                break;
            
            case ' ':
                if(mode == HTTPVER && currVar != nullptr) {
                    currVar->push_back(buffer);
                    buffer.clear();
                    continue;
                }
                break;

            default:
                lineEnd = false;
                if(buffer == "HTT" && byte == 'P') {
                    currVar = &m_data["HTTP"];
                    mode = HTTPVER;
                    continue;
                }
                break;
        
        }
            
        buffer += byte;
    }
}

void Http::readContent(const std::string& httpStr, size_t offset) {
    size_t contentSize = 0;
    std::string hexValue;

    try {
        if(m_data.at("Transfer-Encoding").at(0) == "chunked") {
            while(offset < httpStr.size() && httpStr[offset] != '\r' && httpStr[offset] != '\n') {
                hexValue += httpStr[offset];
                offset++;
            }

            if(httpStr[offset+1] > httpStr.size() || httpStr[offset+1] != '\n') {
                throw std::runtime_error("Failed: Reached end");
            }

            contentSize = hexStringToInt(hexValue);
        }

    } catch(std::out_of_range) {
        std::cerr << "FAILED\n";
        // Fuck you
    }


    if(contentSize == 0) {
        try {
            contentSize =  std::stoi(m_data.at("Content-Length").at(0));
        } catch(std::out_of_range) {
            throw std::runtime_error("Failed to read http content, no content size specified in http header");
        }
    }

    // + 2 to skip endl character \r and \n
    m_httpContent = httpStr.substr(offset + 2, contentSize);
}

int Http::hexStringToInt(const std::string& hex) {
    int value = 0;
    for (char c : hex) {
        value *= 16;
        if (c >= '0' && c <= '9') value += c - '0';
        else if (c >= 'a' && c <= 'f') value += c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') value += c - 'A' + 10;
        else throw std::invalid_argument("Not a valid hex symbol");
    }
    return value;
}
