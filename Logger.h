//
// Created by tobi on 20.09.20.
//

#ifndef RADARREADER_LOGGER_H
#define RADARREADER_LOGGER_H


#include <fstream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/hdf.hpp>

class Logger {
public:
    Logger(std::string output_path);
    ~Logger();

    int append_row(std::chrono::system_clock::time_point t, const std::string& group, const cv::Mat& mat) ;
    int close();

private:

    std::string get_filename(std::chrono::system_clock::time_point t);
    void assure_open();


    time_t file_time_epoch;

    std::string path;
    std::string current_file_name;
    cv::Ptr<cv::hdf::HDF5> current_file;
};


#endif //RADARREADER_LOGGER_H
