//
// Created by tobi on 15.01.21.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include "../include/utils.h"
#include "../include/radar.h"

#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "EndpointRadarFmcw.h"
#include "EndpointRadarAdcxmc.h"
#include "EndpointRadarP2G.h"

using namespace std;

Radar::Radar() : settings_file(get_env_var("RADAR_SETTINGS", "radar/settings.json")) {
    cout << "use radar settings file: " << settings_file << endl;
    restore_settings();
}

Radar::~Radar() {
    disconnect();
}

bool Radar::connect() {
    protocolHandle = radar_auto_connect();

    bool found = is_connected();

    if (found) {
        identify_available_apis();
        register_settings_callbacks();
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
        send_settings_to_radar();
    }
}

void Radar::send_settings_to_radar() {
    auto fmt = new Frame_Format_t;
    fmt->num_chirps_per_frame = settings["data"]["chirps per frame"].get<int>();
    fmt->num_samples_per_chirp = settings["data"]["samples per chrip"].get<int>();
    fmt->rx_mask = 0x03;
    fmt->eSignalPart = EP_RADAR_BASE_SIGNAL_I_AND_Q;
    set_frame_format(fmt);

    auto fmcw = new Fmcw_Configuration_t;
    fmcw->tx_power = settings["antennas"]["tx"]["power"]["current"].get<int>();
    fmcw->upper_frequency_kHz = settings["frequency"]["high"].get<int>();
    fmcw->lower_frequency_kHz = settings["frequency"]["low"].get<int>();
    fmcw->direction = EP_RADAR_FMCW_DIR_UPCHIRP_ONLY;

    set_fmcw_configuration(fmcw);
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

void Radar::start_measurement(int interval_us, int reconnection_interval_s) {

    cout << "start measurement trigger, request and reconnection loop..." << endl;
    settings["data"]["frame interval"]["current"] = interval_us;
    settings["data"]["frame interval"]["unit"] = "us";
    store_settings();

    ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                              endpointBaseRadar,
                                              interval_us);

    while (true) {
        try {
            while (true) {
                request_data(true);
                this_thread::sleep_for(5ms);
            }
        } catch (const RadarConnectionLost &err) {
            cout << "Warning: Lost connection to radar!" << endl;

            do {
                cout << "try reconnecting ..." << endl;
                if (connect()) {
                    cout << "successfully reconnected" << endl;
                    break;
                }

                cout << "wait for " << reconnection_interval_s << "s ..." << endl;
                this_thread::sleep_for(reconnection_interval_s * 1s);

            } while (!is_connected());
        }
    }
}

void Radar::stop_measurement() const {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                              endpointBaseRadar,
                                              0);
}

void Radar::set_frame_format(const Frame_Format_t *fmt) const {
    ep_radar_base_set_frame_format(protocolHandle, endpointBaseRadar, fmt);
}

void Radar::set_fmcw_configuration(const Fmcw_Configuration_t *config) const {
    ep_radar_fmcw_set_fmcw_configuration(protocolHandle, endpointFmcwRadar, config);
}

void Radar::set_pga_level(uint16_t ppa_level) const {
    ep_radar_p2g_set_pga_level(protocolHandle, endpointP2GRadar, ppa_level);
}

void Radar::set_adc_configuration(const Adc_Xmc_Configuration_t* config) const {
    ep_radar_adcxmc_set_adc_configuration(protocolHandle, endpointAdcRadar, config);
}