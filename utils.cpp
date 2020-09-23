//
// Created by tobi on 23.09.20.
//


#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "utils.h"

// ...

std::string time_in_fmt_MMM(std::chrono::system_clock::time_point now, const std::string& fmt)
{
    using namespace std::chrono;

/*    // get number of milliseconds for the current second
    // (remainder after division into seconds)
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = system_clock::to_time_t(now);

    // convert to broken time
    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, fmt.c_str()); // HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();*/

    double us = duration_cast<microseconds>(now.time_since_epoch()).count() * 1e-6;
    return std::to_string(us);
}