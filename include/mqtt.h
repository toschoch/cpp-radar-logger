//
// Created by tobi on 19.01.21.
//

#ifndef RADARREADER_MQTT_H
#define RADARREADER_MQTT_H

#include <iostream>
#include <cstdlib>
#include <string>
#include <tuple>
#include <mqtt/async_client.h>
#include <thread>
#include <chrono>
#include "../include/utils.h"


class action_listener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token& tok) override;
    void on_success(const mqtt::token& tok) override;

public:
    action_listener(const std::string& name) : name_(name) {}
};

class callback : public virtual mqtt::callback,
                 public virtual mqtt::iaction_listener

{
    // The MQTT client
    mqtt::async_client& cli_;
    // Options to use if we need to reconnect
    mqtt::connect_options& connOpts_;
    // An action listener to display the result of actions.
    action_listener subListener_;

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
    void connected(const std::string& cause) override;

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void connection_lost(const std::string& cause) override;

    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override;

    void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
    callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
            : cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

class MQTTClient
{
    std::string const broker;
    std::string const client_id;

    mqtt::async_client cli;
    mqtt::connect_options connOpts;
    callback cb;

    static std::tuple<std::string, std::string> get_credentials();

public:
    MQTTClient();

    void connect();
    void disconnect();
};

#endif //RADARREADER_MQTT_H
