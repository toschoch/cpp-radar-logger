//
// Created by tobi on 19.01.21.
//

#include "../include/mqtt.h"

// async_subscribe.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT subscriber using the C++ asynchronous client
// interface, employing callbacks to receive messages and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Subscribing to a topic
//  - Receiving messages through the callback API
//  - Receiving network disconnect updates and attempting manual reconnects.
//  - Using a "clean session" and manually re-subscribing to topics on
//    reconnect.
//

/*******************************************************************************
 * Copyright (c) 2013-2020 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>


using namespace std;


/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

void action_listener::on_failure(const mqtt::token& tok) {
    cout << name_ << " failure";
    if (tok.get_message_id() != 0)
        cout << " for token: [" << tok.get_message_id() << "]" << endl;
    cout << endl;
}

void action_listener::on_success(const mqtt::token& tok) {
    cout << name_ << " success";
    if (tok.get_message_id() != 0)
        cout << " for token: [" << tok.get_message_id() << "]" << endl;
    auto top = tok.get_topics();
    if (top && !top->empty())
        cout << "\ttoken topic: '" << (*top)[0] << "', ..." << endl;
    cout << endl;
}

MQTTClient::MQTTClient(string prefix) : broker(get_env_var("MQTT_BROKER", "")),
                                              client_id(get_device_name()+"/"+get_service_name()),
                                              cli(broker, client_id), subListener("Subscription"),
                                              topic_prefix(prefix+"/")
{
    connOpts.set_clean_session(true);

    auto creds = get_credentials();
    auto username = get<0>(creds);
    auto pw = get<1>(creds);

    if (username.length() > 0 ) {
        connOpts.set_user_name(username);
    }

    if (pw.length() > 0 ) {
        connOpts.set_password(pw);
    }

    topics = make_shared<mqtt::string_collection>(mqtt::string_collection());

    // Install the callback(s) before connecting.
    cli.set_callback(*this);
}

void MQTTClient::subscribe(string subtopic, const function<void(string)>& callback) {
    topics->push_back(topic_prefix+subtopic);
    topic_callbacks.push_back(callback);
    qos_vector.push_back(qos);
}

void MQTTClient::connect() {
    try {
        cout << "Connecting to the MQTT server..." << flush << endl;
        cli.connect(connOpts, nullptr, *this);
    }
    catch (const mqtt::exception& exc) {
        cerr << "\nERROR: Unable to connect to MQTT server: '"
                  << broker << "'" << exc << endl;
    }
}

void MQTTClient::disconnect() {

    try {
        cout << "\nDisconnecting from the MQTT server..." << flush;
        cli.disconnect()->wait();
        cout << "OK" << endl;
    }
    catch (const mqtt::exception& exc) {
        cerr << exc << endl;
    }
}

tuple<string, string> MQTTClient::get_credentials()
{
    auto credentials = get_env_var("MQTT_CREDENTIALS","");

    ifstream input(credentials);
    if (input.good()) {
        credentials.clear();
        getline(input, credentials);
    }

    auto idx = credentials.find(":");

    string username;
    string pw;

    if (idx != credentials.npos) {
        username = credentials.substr(0, idx);
        pw = credentials.substr(idx+1, credentials.length());
    }

    auto creds(make_tuple(username, pw));
    return creds;
}

void MQTTClient::reconnect() {
    this_thread::sleep_for(10s);
    try {
        cli.connect(connOpts, nullptr, *this);
    }
    catch (const mqtt::exception& exc) {
        cerr << "Error: " << exc.what() << endl;
        exit(1);
    }
}

// Re-connection failure
void MQTTClient::on_failure(const mqtt::token& tok) {
    cout << "Connection attempt failed" << endl;
    reconnect();
}


// (Re)connection success
void MQTTClient::connected(const string& cause) {
    cout << "Connection success" << endl;
    cout << "Subscribing to topic...'";
    for (auto i=0; i<topics->size(); ++i)
        cout << (*topics)[i] << " ";
    cout << endl;

    cli.subscribe(topics, qos_vector, nullptr, subListener);
}

// Callback for when the connection is lost.
// This will initiate the attempt to manually reconnect.
void MQTTClient::connection_lost(const string& cause) {
    cout << "Connection lost" << endl;
    if (!cause.empty())
        cout << "\tcause: " << cause << endl;

    cout << "Reconnecting..." << endl;
    reconnect();
}

// Callback for when a message arrives.
void MQTTClient::message_arrived(mqtt::const_message_ptr msg) {
    for (auto i=0; i<topics->size(); ++i)
        if ((*topics)[i] == msg->get_topic())
            topic_callbacks[i](msg->get_payload_str());

}

/////////////////////////////////////////////////////////////////////////////
