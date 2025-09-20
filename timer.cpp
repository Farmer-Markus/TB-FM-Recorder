#include "timer.h"

#include <chrono>
#include <string>

namespace tme = std::chrono;



tme::steady_clock::time_point Timer::timeRunning = tme::steady_clock::now();

size_t Timer::steadyToMin(const tme::steady_clock::time_point time) {
    auto now = tme::steady_clock::now();
    auto diff = now - time; 
    return std::chrono::duration_cast<std::chrono::minutes>(diff).count();
}


void Timer::steadyToHS(int* h, int* m, const tme::steady_clock::time_point time) {
    const tme::steady_clock::time_point now = tme::steady_clock::now();
    const tme::duration<long> diff = std::chrono::duration_cast<std::chrono::seconds>(now - time);

    *h = diff.count() / 3600;
    *m = (diff.count() % 3600) / 60;
    // *s = diff.count() % 60;
}

std::string Timer::steadyToHSFancy(const tme::steady_clock::time_point time) {
    int h,m;
    steadyToHS(&h, &m, time);

    auto intToStr = [](int tm) {
        std::string tmStr = std::to_string(tm);
        if(tmStr.size() <= 1)
            tmStr.insert(tmStr.begin(), '0');
        return tmStr;
    };

    std::string tm;
    tm.append(intToStr(h));
    tm.append(":");
    tm.append(intToStr(m));
    return tm;

}

std::tm Timer::sysTime() {
    std::time_t sysTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return *std::localtime(&sysTime);
}

std::string Timer::sysTimeFancy() {
    std::tm tm = sysTime();
    std::string time;

    auto intToStr = [](int tm) {
        std::string tmStr = std::to_string(tm);
        if(tmStr.size() <= 1)
            tmStr.insert(tmStr.begin(), '0');
        return tmStr;
    };

    time.append(intToStr(tm.tm_hour));
    time.append(":");
    time.append(intToStr(tm.tm_min));
    return time;
}