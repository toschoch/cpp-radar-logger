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

ZMQ::ZMQ() : socket(context, zmqpp::socket_type::pub) {
    string port = get_env_var("ZMQ_PORT");
    if (port == "") {
        port = "5555";
    }
    endpoint = string("tcp://*:") + port;
}

void ZMQ::start() {
    cout << "starting zmq server on " << endpoint << "..." << endl;

    // bind to the socket
    socket.bind(endpoint);
}

bool ZMQ::send_tensor(shared_ptr<arrow::Tensor> tensor) {
    zmqpp::message message;
    message.add_raw(written_buffer->data(), written_buffer->size());

    socket.send(message);
}