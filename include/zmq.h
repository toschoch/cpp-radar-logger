//
// Created by tobi on 16.01.21.
//

#ifndef RADARREADER_ZMQ_H
#define RADARREADER_ZMQ_H

#include <arrow/buffer.h>
#include <zmqpp/zmqpp.hpp>
#include <string>

class ZMQ {
protected:
    zmqpp::context context;
    std::string const endpoint;

public:
    ZMQ();

    void start();

    bool send_buffer(std::shared_ptr<arrow::Buffer> buffer);

    zmqpp::socket socket;
};

#endif //RADARREADER_ZMQ_H
