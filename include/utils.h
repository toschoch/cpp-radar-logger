//
// Created by tobi on 23.09.20.
//

#ifndef RADARREADER_UTILS_H
#define RADARREADER_UTILS_H
#include <string>
#include <chrono>
#include <vector>
#include <map>

std::string time_in_fmt_MMM(std::chrono::system_clock::time_point now, const std::string& fmt);

std::string get_env_var( std::string const & key, std::string const & default_value);

std::vector<float> generate_data(size_t size);

#endif //RADARREADER_UTILS_H
