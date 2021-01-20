//
// Created by tobi on 15.01.21.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "../include/utils.h"
#include "../include/radar.h"

#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "EndpointRadarFmcw.h"
#include "EndpointRadarAdcxmc.h"
#include "EndpointRadarP2G.h"

using namespace std;

Radar::Radar() : settings_file(get_env_var("RADAR_SETTINGS", "radar/settings.json")),
                 send_settings_wait_time(100) {
    cout << "use radar settings file: " << settings_file << endl;
    restore_settings();
}

Radar::~Radar() {
    disconnect();
}

bool Radar::connect() {
    protocolHandle = radar_auto_connect();
    bool found = is_connected();

    while (!found && reconnection_interval_s > 0) {
        cout << "could not find radar! try again in " << reconnection_interval_s << "s..." << endl;
        std::this_thread::sleep_for(reconnection_interval_s*1s);
        protocolHandle = radar_auto_connect();
        found = is_connected();
    }

    if (found) {
        identify_available_apis();
        stop_automatic_frame_triggering();
        register_settings_callbacks();
        send_settings_to_radar();
    }

    return found;
}

void Radar::disconnect() const {
    if (is_connected()) {
        ep_radar_base_set_automatic_frame_trigger(protocolHandle, endpointBaseRadar,0);
        protocol_disconnect(protocolHandle);
    }
}

void Radar::restore_settings() {
    ifstream input(settings_file);
    if (input.good()) {
        input >> settings;
    }
}

unique_ptr<Frame_Format_t> Radar::get_settings_frame_format() {
    auto fmt = unique_ptr<Frame_Format_t>{new Frame_Format_t};
    fmt->num_chirps_per_frame = settings["data"]["chirps per frame"].get<int>();
    fmt->num_samples_per_chirp = settings["data"]["samples per chrip"].get<int>();
    auto activated = settings["antennas"]["rx"]["activated"];
    auto mask = 0x00;
    for (auto & it : activated) {
        int ant = it;
        mask = mask | (0x01 << ant);
    }
    fmt->rx_mask = mask;
    auto signal_part = settings["data"]["signal part"].get<string>();
    Signal_Part_t signal_enum;
    for (const auto & signal_part_name : signal_part_names)
        if (signal_part_name.second == signal_part)
            signal_enum = signal_part_name.first;
    fmt->eSignalPart = signal_enum;

    return fmt;
}

unique_ptr<Fmcw_Configuration_t> Radar::get_settings_fmcw_configuration() {

    auto fmcw = unique_ptr<Fmcw_Configuration_t>{new Fmcw_Configuration_t};
    fmcw->tx_power = settings["antennas"]["tx"]["power"]["current"].get<int>();
    fmcw->upper_frequency_kHz = settings["frequency"]["high"].get<int>();
    fmcw->lower_frequency_kHz = settings["frequency"]["low"].get<int>();
    auto chirp_direction = settings["frequency"]["chirp"]["direction"].get<string>();
    Chirp_Direction_t chirp_enum;
    for (const auto & chirp_name : chirp_direction_names)
        if (chirp_name.second == chirp_direction)
            chirp_enum = chirp_name.first;
    fmcw->direction = chirp_enum;

    return fmcw;
}

void Radar::send_settings_to_radar() {

    auto fmt = get_settings_frame_format();
    set_frame_format(fmt.get());

    auto fmcw = get_settings_fmcw_configuration();
    set_fmcw_configuration(fmcw.get());

    set_pga_level(settings["sampling"]["programmable gain level"].get<int>());
}

void Radar::store_settings() {
    ofstream output(settings_file);
    output << setw(4) << settings << endl;
}

void Radar::identify_available_apis() {
    for (int i = 1; i <= protocol_get_num_endpoints(protocolHandle); ++i) {
        // current endpoint is radar base endpoint
        if (ep_radar_base_is_compatible_endpoint(protocolHandle, i) == 0)
        {
            endpointBaseRadar = i;
            continue;
        }
        if (ep_radar_fmcw_is_compatible_endpoint(protocolHandle, i) == 0)
        {
            endpointFmcwRadar = i;
            continue;
        }
        if (ep_radar_adcxmc_is_compatible_endpoint(protocolHandle, i) == 0)
        {
            endpointAdcRadar = i;
            continue;
        }
        if (ep_radar_p2g_is_compatible_endpoint(protocolHandle, i) == 0)
        {
            endpointP2GRadar = i;
            continue;
        }
    }
}

void Radar::register_data_received_callback(const function<void(const Frame_Info_t*)>& callback) {

    data_callback = callback;

    auto callback_wrapper = [] (void *context, int32_t protocol_handle, uint8_t endpoint, const Frame_Info_t* frame_info) {
        auto radar = (Radar*) context;
        radar->data_callback(frame_info);
    };

    ep_radar_base_set_callback_data_frame(callback_wrapper, this);
}

void Radar::register_settings_callbacks() {
    // register call back functions for adc data
    if (endpointBaseRadar > 0) {
        ep_radar_base_set_callback_device_info(on_device_info_received, this);
        ep_radar_base_set_callback_frame_format(on_frame_format_setting_received, this);
        ep_radar_base_set_callback_chirp_duration(on_chirp_duration_received, this);
        ep_radar_base_set_callback_min_frame_interval(on_minimal_frame_interval_received, this);
    }

    if (endpointFmcwRadar) {
        ep_radar_fmcw_set_callback_fmcw_configuration(on_fmcw_config_received, this);
        ep_radar_fmcw_set_callback_bandwidth_per_second(on_fmcw_bandwith_per_second_received, this);
    }

    if (endpointAdcRadar) {
        ep_radar_adcxmc_set_callback_adc_configuration(on_adc_config_received, this);
    }

    if (endpointP2GRadar) {
        ep_radar_p2g_set_callback_pga_level(on_adc_gain_level_received, this);
    }
}

void Radar::update_settings() const {
    request_device_info();
    request_minimal_frame_interval();
    request_chirp_duration();
    request_frame_format();
    request_fmcw_configuration();
    request_chirp_velocity();
    request_adc_configuration();
    request_adc_gain_level();
}

void Radar::request_data(uint8_t wait) const { handle_error_codes(ep_radar_base_get_frame_data(protocolHandle, endpointBaseRadar, wait)); }
void Radar::request_device_info() const { handle_error_codes(ep_radar_base_get_device_info(protocolHandle, endpointBaseRadar)); }
void Radar::request_chirp_duration() const { handle_error_codes(ep_radar_base_get_chirp_duration(protocolHandle, endpointBaseRadar)); }
void Radar::request_frame_format() const { handle_error_codes(ep_radar_base_get_frame_format(protocolHandle, endpointBaseRadar)); }
void Radar::request_fmcw_configuration() const { handle_error_codes(ep_radar_fmcw_get_fmcw_configuration(protocolHandle, endpointFmcwRadar)); }
void Radar::request_chirp_velocity() const { handle_error_codes(ep_radar_fmcw_get_bandwidth_per_second(protocolHandle, endpointFmcwRadar)); }
void Radar::request_adc_configuration() const { handle_error_codes(ep_radar_adcxmc_get_adc_configuration(protocolHandle, endpointAdcRadar)); }
void Radar::request_adc_gain_level() const { handle_error_codes(ep_radar_p2g_get_pga_level(protocolHandle, endpointP2GRadar));}
void Radar::request_minimal_frame_interval() const { handle_error_codes( ep_radar_base_get_min_frame_interval(protocolHandle, endpointBaseRadar)); }

void Radar::handle_error_codes(int32_t error_code) {

    if (-2000 < error_code and error_code <= -1000) {
        throw RadarConnectionLost("lost connection to radar");
    }
}

void Radar::start_measurement() {

    cout << "start measurement trigger, request and reconnection loop..." << endl;
    store_settings();

    measurement_started = true;

    do {
        start_automatic_frame_triggering();
        try {
            while (true) {
                request_data(true);
                this_thread::sleep_for(5ms);
            }
        } catch (const RadarConnectionLost &err) {
            cout << "Warning: Lost connection to radar!" << endl;

            if (reconnection_interval_s>=0) {
                cout << "try reconnecting ..." << endl;
                if (connect()) {
                    cout << "successfully reconnected" << endl;
                    break;
                }
            }
        }
    } while (reconnection_interval_s>=0);
}

void Radar::stop_measurement() {
    measurement_started = false;
}

void Radar::start_automatic_frame_triggering() {
    auto interval_us = settings["data"]["frame interval"]["current"].get<int>();
    if (interval_us > 0 ) {
        ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                                  endpointBaseRadar,
                                                  interval_us);
        frame_triggering_activated = true;
    }
}

void Radar::stop_automatic_frame_triggering() {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                              endpointBaseRadar,
                                              0);
    frame_triggering_activated = false;
}

void Radar::set_frame_interval(int interval_us) {
    stop_automatic_frame_triggering();

    settings["data"]["frame interval"]["current"] = min(interval_us, settings["data"]["frame interval"]["min"].get<int>());
    settings["data"]["frame interval"]["unit"] = "us";
    request_minimal_frame_interval();

    if (measurement_started) {
        this_thread::sleep_for(send_settings_wait_time);
        start_automatic_frame_triggering();
    }
}

void Radar::set_reconnection_interval(int interval_s) {
    reconnection_interval_s = interval_s;
}

void Radar::set_frame_format(const Frame_Format_t *fmt) {
    stop_automatic_frame_triggering();

    ep_radar_base_set_frame_format(protocolHandle, endpointBaseRadar, fmt);

    // read back for updates
    request_frame_format();
    request_minimal_frame_interval();

    if (measurement_started) {
        this_thread::sleep_for(send_settings_wait_time);
        start_automatic_frame_triggering();
    }
}

void Radar::set_fmcw_configuration(const Fmcw_Configuration_t *config) {
    stop_automatic_frame_triggering();
    ep_radar_fmcw_set_fmcw_configuration(protocolHandle, endpointFmcwRadar, config);

    // read back for updates
    request_fmcw_configuration();
    request_minimal_frame_interval();

    if (measurement_started) {
        this_thread::sleep_for(send_settings_wait_time);
        start_automatic_frame_triggering();
    }
}


void Radar::set_pga_level(uint16_t ppa_level) {
    stop_automatic_frame_triggering();
    ep_radar_p2g_set_pga_level(protocolHandle, endpointP2GRadar, ppa_level);

    // read back for updates
    request_adc_gain_level();

    if (measurement_started) {
        this_thread::sleep_for(send_settings_wait_time);
        start_automatic_frame_triggering();
    }
}

void Radar::set_adc_configuration(const Adc_Xmc_Configuration_t *config) {
    stop_automatic_frame_triggering();
    ep_radar_adcxmc_set_adc_configuration(protocolHandle, endpointAdcRadar, config);

    // read back for updates
    request_adc_configuration();
    request_minimal_frame_interval();

    if (measurement_started) {
        this_thread::sleep_for(send_settings_wait_time);
        start_automatic_frame_triggering();
    }
}