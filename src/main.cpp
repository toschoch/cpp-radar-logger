
#include <iostream>
#include <vector>
#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "../include/radar.h"
#include "../include/radar_enums.h"
#include "../include/utils.h"
#include "../include/arrow.h"
#include "../include/zmq.h"
#include "../include/mqtt.h"
#include <cstdlib>
#include <chrono>
#include <exception>

using namespace std;
using namespace std::chrono_literals;

#define AUTOMATIC_DATA_TRIGGER_TIME_US (30000)	// get ADC data each 1ms in automatic trigger mode
int main(void)
{
    ZMQ zmq_server;
    Radar radar;
    radar.set_reconnection_interval(5);

    cout << "try to find connected radar..." << endl;
    if (!radar.connect()) {
        exit(1);
    }

    MQTTClient mqtt_client(get_device_name()+"/radar");

    mqtt_client.subscribe("data/frame_interval/set", [&radar](const string& s) {
        auto interval_us = stoi(s);
        cout << "set measurement interval to: " << interval_us << "us" << endl;
        radar.set_frame_interval(interval_us);
    });
    mqtt_client.subscribe("data/chirps_per_frame/set",[&radar](const string& s) {
        auto n_chirps = stoi(s);
        cout << "set number of chirps per frame to: " << n_chirps << endl;
        auto frame_fmt = radar.get_settings_frame_format();
        frame_fmt->num_chirps_per_frame = n_chirps;
        radar.set_frame_format(frame_fmt);
    });
    mqtt_client.subscribe("data/samples_per_chirp/set",[&radar](const string& s) {
        auto n_samples = stoi(s);
        cout << "set number of samples per chirp to: " << n_samples << endl;
        auto frame_fmt = radar.get_settings_frame_format();
        frame_fmt->num_samples_per_chirp = n_samples;
        radar.set_frame_format(frame_fmt);
    });
    mqtt_client.subscribe("antennas/tx/power/set",[&radar](const string& s) {
        auto tx_power = stoi(s);
        cout << "set transmission power to: " << tx_power << endl;
        auto fmcw = radar.get_settings_fmcw_configuration();
        fmcw->tx_power = tx_power;
        radar.set_fmcw_configuration(fmcw);
    });
    mqtt_client.subscribe("frequency/chirp/direction/set",[&radar](const string& s) {
        auto chirp_direction = chirp_direction_enums.at(s);
        cout << "set chirp direction to: " << s << "(" << chirp_direction << ")" << endl;
        auto fmcw = radar.get_settings_fmcw_configuration();
        fmcw->direction = chirp_direction;
        radar.set_fmcw_configuration(fmcw);
    });

    mqtt_client.subscribe("frequency/low/set",[&radar](const string& s) {
        auto freq = stoi(s);
        cout << "set (lower) frequency to: " << s << endl;
        auto fmcw = radar.get_settings_fmcw_configuration();
        fmcw->lower_frequency_kHz = freq;
        radar.set_fmcw_configuration(fmcw);
    });

    mqtt_client.subscribe("frequency/bandwidth/set",[&radar](const string& s) {
        auto bw = stoi(s);
        cout << "set bandwidth to: " << s << endl;
        auto fmcw = radar.get_settings_fmcw_configuration();
        fmcw->upper_frequency_kHz = fmcw->lower_frequency_kHz + bw;
        radar.set_fmcw_configuration(fmcw);
    });

    mqtt_client.subscribe("sampling/programmable_gain_level/set",[&radar](const string& s) {
        auto pga = stoi(s);
        cout << "set programmable gain to: " << pga << endl;
        radar.set_pga_level(pga);
    });

    mqtt_client.connect();

    this_thread::sleep_for(500ms);

    cout << "success! connecting..." << endl;

    zmq_server.start();

    // called every time ep_radar_base_get_frame_data method is called to return measured time domain signals
    auto received_frame_data = [&zmq_server](const Frame_Info_t* frame_info)
    {
        auto t = std::chrono::system_clock::now().time_since_epoch().count() * 1e-9;

        vector<int64_t> dims;
        dims.push_back(frame_info->num_chirps);
        dims.push_back(frame_info->num_rx_antennas);
        dims.push_back(frame_info->data_format==EP_RADAR_BASE_RX_DATA_REAL ? 1 : 2);
        dims.push_back(frame_info->num_samples_per_chirp);

        auto elements = accumulate(begin(dims), end(dims), 1, multiplies<int>());
        vector<float> data;
        data.assign(frame_info->sample_data, frame_info->sample_data+elements);

        auto buf = create_and_serialize_tensor(dims, data);
        zmq_server.send_time_and_buffer(t, buf);
    };

    radar.register_data_received_callback(received_frame_data);

    auto received_settings = [&mqtt_client](const json& settings) {
        if (mqtt_client.is_connected())
            mqtt_client.publish_string("settings", settings.dump(2), 1, true);
    };

    radar.register_settings_received_callback(received_settings);

    cout << "done!" << endl << endl;
    radar.update_settings();

    cout << radar.settings.dump(4) << endl;

    radar.start_measurement_loop();

    exit(0);
}