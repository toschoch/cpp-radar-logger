//
// Created by tobi on 20.09.20.
//

#ifndef RADARREADER_LOGGER_H
#define RADARREADER_LOGGER_H


#include <fstream>
#include <vector>

class Logger {
public:
    Logger(std::string output_path);
    ~Logger();

    int append_row(time_t t, std::vector<float> data, std::vector<float> data_fft);

private:

    int create_new_file();
    int write_header();


    time_t file_time_epoch;

    std::string path;
    std::string current_file_name;
    std::ofstream current_file;
};


#endif //RADARREADER_LOGGER_H
