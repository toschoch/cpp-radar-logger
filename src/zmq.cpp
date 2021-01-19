//
// Created by tobi on 12.01.21.
//

//  Hello World server

#include "../include/zmq.h"
#include "../include/utils.h"
#include <zmqpp/zmqpp.hpp>
#include <string>
#include <iostream>

using namespace std;

ZMQ::ZMQ() : socket(context, zmqpp::socket_type::pub),
             endpoint(string("tcp://*:") + get_env_var("ZMQ_PORT", "5555")) {}

string ZMQ::get_endpoint() const {
    return endpoint;
}

void ZMQ::start() {
    cout << "starting zmq server on " << endpoint << "..." << endl;

    // bind to the socket
    socket.bind(endpoint);
}

bool ZMQ::send_buffer(shared_ptr<arrow::Buffer> buffer) {
    zmqpp::message message;
    message.add_raw(buffer->data(), buffer->size());

    return socket.send(message);
}