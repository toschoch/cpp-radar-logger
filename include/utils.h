//
// Created by tobi on 23.09.20.
//

#ifndef RADARREADER_UTILS_H
#define RADARREADER_UTILS_H
#include <string>
#include <chrono>
#include <vector>
#include <map>

using namespace std;

/// Given a map from keys to values, creates a new map from values to keys
template<typename K, typename V>
static map<V, K> reverse_map(const map<K, V>& m) {
    map<V, K> r;
    for (const auto& kv : m)
        r[kv.second] = kv.first;
    return r;
}

string time_in_fmt_MMM(chrono::system_clock::time_point now, const string& fmt);

string get_env_var( string const & key, string const & default_value);

vector<float> generate_data(size_t size);

string get_service_name();
string get_device_name();

#endif //RADARREADER_UTILS_H
