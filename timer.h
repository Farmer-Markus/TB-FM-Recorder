#pragma once
#include <chrono>
#include <cstddef>
#include <string>

namespace tme = std::chrono;


class Timer {
public:
    static tme::steady_clock::time_point timeRunning;

    static size_t steadyToMin(const tme::steady_clock::time_point time);
    // Converts a steady clock to hours and minutes
    static void steadyToHS(int* h, int* m, const tme::steady_clock::time_point time);
    static std::string steadyToHSFancy(const tme::steady_clock::time_point time);
    static std::tm sysTime();
    static std::string sysTimeFancy();


private:




};
