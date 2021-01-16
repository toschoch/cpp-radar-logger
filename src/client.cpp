//
// Created by tobi on 12.01.21.
//

//  Hello World client
#include <zmqpp/zmqpp.hpp>
#include <string>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    const string endpoint = "tcp://0.0.0.0:5555";

    // initialize the 0MQ context
    zmqpp::context context;

    // generate a push socket
    zmqpp::socket_type type = zmqpp::socket_type::subscribe;
    zmqpp::socket socket (context, type);

    // open the connection
    cout << "Connecting to server…" << endl;
    socket.connect(endpoint);

    socket.set(zmqpp::socket_option::subscribe, "");

    while (1) {
        // get a message
        zmqpp::message msg;
        cout << "waiting for a message..." << endl;
        socket.receive(msg);
        cout << "got message of " << msg.size(0) << " bytes" << endl;
    }
}