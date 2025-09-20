#include "json.h"

#include <cstddef>
#include <string>


enum {
    NONE,
    TEXT,
    VALUE,
    TEXTVALUE,
    SKIP
};


size_t Json::parsePart(Json& json, std::string src, size_t offset) {
    size_t mode = NONE;
    size_t oldMode = NONE;

    std::string buffer;
    Json* currElement = &json;

    size_t srcSize = src.size();
    size_t i;
    for(i = offset; i < srcSize; i++) {
        char byte = src.at(i);

        
        if(mode != SKIP) {
            switch(byte) {
                case '{':
                    if(mode == NONE || mode == VALUE) {
                        i = parsePart(*currElement, src, i + 1);
                    }
                    continue;

                case '}':
                    if(mode == NONE) {
                        return i;
                    }
                    break;
                
                case '"': // If reading text stop. If not, begin
                    if(mode == TEXT || mode == TEXTVALUE) {
                        if(mode == TEXTVALUE) {
                            (*currElement) = buffer;
                        }
                        mode = NONE;

                    } else if(mode == VALUE) {
                        mode = TEXTVALUE;

                    } else if(mode == NONE) {
                        buffer.clear();
                        mode = TEXT;
                    }

                    continue;

                case ':':
                    currElement = &json.data[buffer]; // Access/Create new element
                    buffer.clear();
                    mode = VALUE;
                    continue;
                
                case ',':
                    if(mode == VALUE) {
                        (*currElement) = buffer;
                    }
                    buffer.clear();
                    mode = NONE;
                    continue;
                
                case '\\':
                    oldMode = mode;
                    mode = SKIP;
                    continue;
                
                default:
                    break;
            }
        } else {
            mode = oldMode;
        }

        if(mode != NONE)
            buffer += byte;
    }

    return srcSize;
}

bool Json::parse(std::string src) {
    if(parsePart((*this), src) < 0)
        return false;

    return true;
}