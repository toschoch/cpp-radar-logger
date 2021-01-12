//
// Created by tobi on 23.09.20.
//

#ifndef RADARREADER_UTILS_H
#define RADARREADER_UTILS_H
#include <string>

std::string time_in_fmt_MMM(std::chrono::system_clock::time_point now, const std::string& fmt);

#endif //RADARREADER_UTILS_H
