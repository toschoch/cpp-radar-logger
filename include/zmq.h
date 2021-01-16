//
// Created by tobi on 16.01.21.
//

#ifndef RADARREADER_ZMQ_H
#define RADARREADER_ZMQ_H

#include <string>

class ZMQ {
protected:
    zmqpp::context context;
    const std::string endpoint;

public:
    ZMQ();

    void start();

    bool send_raw()

    zmqpp::socket socket;
};

#endif //RADARREADER_ZMQ_H
