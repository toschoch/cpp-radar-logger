//
// Created by tobi on 19.01.21.
//

#ifndef RADARREADER_MQTT_H
#define RADARREADER_MQTT_H

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <tuple>
#include <mqtt/async_client.h>
#include <thread>
#include <chrono>
#include "../include/utils.h"

using namespace std;

class action_listener : public virtual mqtt::iaction_listener
{
    string name_;

    void on_failure(const mqtt::token& tok) override;
    void on_success(const mqtt::token& tok) override;

public:
    action_listener(const string& name) : name_(name) {}
};

class MQTTClient : public virtual mqtt::callback,
                   public virtual mqtt::iaction_listener
{
    string const broker;
    string const client_id;
    string const topic_prefix;

    int const qos = 1;

    mqtt::async_client cli;
    mqtt::connect_options connOpts;

    // An action listener to display the result of actions.
    action_listener subListener;

    shared_ptr<mqtt::string_collection> topics;
    vector<int> qos_vector;
    vector<function<void(const string&)>> topic_callbacks;

    static tuple<string, string> get_credentials();

    // CALLABACKS
    // This demonstrates manually reconnecting to the broker by calling
    // connect() again. This is a possibility for an application that keeps
    // a copy of it's original connect_options, or if the app wants to
    // reconnect with different options.
    // Another way this can be done manually, if using the same options, is
    // to just call the async_client::reconnect() method.
    void reconnect();

    // Re-connection failure
    void on_failure(const mqtt::token& tok) override;

    // (Re)connection success
    // Either this or connected() can be used for callbacks.
    void on_success(const mqtt::token& tok) override {}

    // (Re)connection success
    void connected(const string& cause) override;

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void connection_lost(const string& cause) override;

    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override;

    void delivery_complete(mqtt::delivery_token_ptr token) override {}


public:
    MQTTClient(string prefix="");

    void subscribe(string subtopic, const function<void(string)>& callback);

    void connect();
    void disconnect();
};

#endif //RADARREADER_MQTT_H
