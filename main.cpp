#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <signal.h>
#include <chrono>
#include <string>

#include "audioManager.h"
#include "songManager.h"


int main(int argc, char** argv) {
    // When server closes socket
    signal(SIGPIPE, SIG_IGN);

    const char* url = "https://listener1.mp3.tb-group.fm/tb.mp3";
    const char* apiUrl = "https://www.technobase.fm/index.php?option=com_broadcast&task=current.playhistory&format=json&station=technobase.fm";

    size_t minTimeCon{0}, maxTimeCon{0};
    std::string djCon, oPath;


    int write_value = -1;
    for(int i = 1; i < argc; i++) {
    std::string arg = argv[i];
        switch (write_value) {
            case 0:
                minTimeCon = std::atoi(argv[i]);
                write_value = -1;
                continue;

            case 1:
                maxTimeCon = std::atoi(argv[i]);
                write_value = -1;
                continue;

            case 2:
                djCon = arg;
                write_value = -1;
                continue;
            
            case 3:
                oPath = arg;
                write_value = -1;
                continue;

            default:
                break;
        }

        if(arg == "-?" || arg == "-h" || arg == "--help") {
            std::cout << "Available options:\n"
                        "--umin-time <time>     | Set minimum record time\n"
                        "--umax-time <time>     | Set maximum record time\n"
                        "--udj-change <dj-name> | Set name of dj to stop after\n"
                        "-o <path>              | Set output path for audio/text files\n";
            return 0;
        } else if(arg == "--umin-time") {
            write_value = 0;
        } else if(arg == "--umax-time") {
            write_value = 1;
            
        } else if(arg == "--udj-change") {
            write_value = 2;
        } else if(arg == "-o") {
            write_value = 3;
        } else {
            std::cout << "Option '" << arg << "' not found\n";
            return 1;
        }
    }



    std::time_t sysTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm = *std::localtime(&sysTime);

    std::string audioFile, textFile, baseFile;
    baseFile.append("TechnoBaseFM-");
    baseFile.append(std::to_string(tm.tm_mday));
    baseFile.append(".");
    baseFile.append(std::to_string(tm.tm_mon + 1));
    baseFile.append(".");
    baseFile.append(std::to_string(tm.tm_year + 1900));
    baseFile.append("-");
    baseFile.append(std::to_string(tm.tm_hour));
    baseFile.append(".");
    baseFile.append(std::to_string(tm.tm_min));


    if(!oPath.empty()) {
        audioFile = oPath + "/" + baseFile;
        audioFile.append(".mp3");
        textFile = oPath + "/" + baseFile;
        textFile.append(".txt");
    } else {
        audioFile = baseFile;
        audioFile.append(".mp3");
        textFile = baseFile;
        textFile.append(".txt");
    }

    


    AudioManager audioManager(audioFile, url);
    SongManager sngManager(textFile, apiUrl);
    if(minTimeCon > 0)
        sngManager.setMinTimeCondition(minTimeCon);
    if(maxTimeCon > 0)
        sngManager.setMaxTimeCondition(maxTimeCon);
    if(!djCon.empty())
        sngManager.setDjCondition(djCon);


    auto time =  std::chrono::steady_clock::now();

    while(true) {
        if(!audioManager.tick())
            std::cout << "AudioManager: Web request failed\n";

        auto aTime = std::chrono::steady_clock::now();
        if(aTime >= time) {
            if(!sngManager.update())
                std::cout << "SongManager: Web request failed\n";
            time = aTime + std::chrono::seconds(10);
        }

        if(sngManager.finished)
            break;
    }

    return 0;
}