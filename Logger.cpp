//
// Created by tobi on 20.09.20.
//

#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <opencv2/core.hpp>
#include <opencv2/hdf.hpp>

#include "utils.h"

using namespace std;

inline bool exists(const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

Logger::Logger(string output_path) : path(output_path), current_file_name(""), current_file(nullptr), file_time_epoch(0) {}

Logger::~Logger() {
    if(current_file)
        current_file->close();
}

int Logger::append_row(chrono::system_clock::time_point t, const string& group, const cv::Mat& mat) {

    // time label
    auto time_label = time_in_fmt_MMM(t, "%Y-%m-%dT%H-%M-%S");
    auto label = group+"/"+time_label;

    assure_open();

    // create group
    if (!current_file->hlexists(group))
    {
        current_file->grcreate(group);
    }

    if (!current_file->hlexists(label))
    {
        current_file->dscreate(mat.rows, mat.cols, mat.type(), label, 9);
    }

    current_file->dswrite(mat,label);
}

void Logger::assure_open() {

    auto now = chrono::system_clock::now();

    auto filename = get_filename(now);

    auto t_diff = chrono::system_clock::to_time_t(now) - file_time_epoch;

    // make file rollover if necessary
    if ((t_diff > 3600) || (!current_file))
    {
        if (current_file){
            current_file->close();
        }
        current_file_name = filename;
        current_file = cv::hdf::open(current_file_name);
        file_time_epoch = chrono::system_clock::to_time_t(now);
    }
}

std::string Logger::get_filename(chrono::system_clock::time_point t) {
    auto timer = chrono::system_clock::to_time_t(t);
    auto tm = *localtime(&timer);

    ostringstream oss;
    oss << path << "/" << put_time(&tm, "%Y-%m-%dT%H-00-00.h5");
    return oss.str();
}

int Logger::close() {
    current_file->close();
    current_file = nullptr;
}