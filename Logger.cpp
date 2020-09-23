//
// Created by tobi on 20.09.20.
//

#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace std;

inline bool exists(const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

Logger::Logger(string output_path) {
    path = output_path;
    create_new_file();
}

Logger::~Logger() {
    if (current_file.is_open())
    {
        current_file.close();
    }
}

int Logger::append_row(time_t t, vector<float> data, vector<float> data_fft) {

    // check if new file is necessary
    auto tDiff = (t - file_time_epoch);

    auto n = data.size();

    if (tDiff >= 3600) {
        create_new_file();
    }

    // write data
    current_file << t << ";";

    for (int i=0; i<n; ++i){
        current_file << data[i] << ";";
    }

    n = data_fft.size();

    for (int i=0; i<(n-1); ++i){
        current_file << data_fft[i] << ";";
    }

    current_file << data_fft[n-1] << endl;

}

int Logger::write_header() {
    auto header_lines = 10;
    current_file << "# time domain: " << 64 << endl;
    header_lines--;
    current_file << "# freq domain: " << 64 << endl;
    header_lines--;
    for (int i=header_lines;i>1;i--){
        current_file << "# " << endl;
    }
    current_file << "# time; tData; fData" <<  endl;
    header_lines--;
}

int Logger::create_new_file() {
    file_time_epoch = time(nullptr);
    auto tm = *localtime(&file_time_epoch);

    ostringstream oss;
    oss << path << "/" << put_time(&tm, "%Y-%m-%dT%H-00-00.dat");
    current_file_name = oss.str();

    if (current_file.is_open())
    {
        current_file.close();
    }

    if(exists(current_file_name)) {
        current_file.open(current_file_name, ios::app | ios::binary);
    } else {
        current_file.open(current_file_name, ios::out | ios::binary);
        write_header();
    }

    if(current_file.good()) {
        cout << "start logging to '"<< current_file_name << "'..." << endl;
        return 0;
    }
    return 1;
}