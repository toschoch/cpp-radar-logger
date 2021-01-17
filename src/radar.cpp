//
// Created by tobi on 15.01.21.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "../include/utils.h"
#include "../include/radar.h"

#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "EndpointRadarFmcw.h"
#include "EndpointRadarAdcxmc.h"
#include "EndpointRadarP2G.h"

Radar::Radar() : settings_file(get_env_var("RADAR_SETTINGS", "/radar/settings.json")) {
    std::cout << "use radar settings file: " << settings_file << std::endl;
    restore_settings();
}

bool Radar::connect() {
    protocolHandle = radar_auto_connect();

    bool found = is_connected();

    if (found) {
        identify_available_apis();
        register_callbacks();
    }

    return found;
}

void Radar::disconnect() const {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle, endpointBaseRadar,0);
    protocol_disconnect(protocolHandle);
}

void Radar::restore_settings() {
    std::ifstream input(settings_file);
    if (input.good()) {
        input >> settings;
    }
}

void Radar::store_settings() {
    std::ofstream output(settings_file);
    output << std::setw(4) << settings << std::endl;
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

void Radar::register_callbacks() {
    // register call back functions for adc data
    if (endpointBaseRadar > 0) {
        ep_radar_base_set_callback_device_info(on_device_info_received, this);
        //ep_radar_base_set_callback_data_frame(received_frame_data, NULL);
        ep_radar_base_set_callback_frame_format(on_frame_format_setting_received, this);
        ep_radar_base_set_callback_chirp_duration(on_chirp_duration_received, this);
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
    request_chirp_duration();
    request_frame_format();
    request_fmcw_configuration();
    request_chirp_velocity();
    request_adc_configuration();
}

void Radar::request_data(uint8_t wait) const { handle_error_codes(ep_radar_base_get_frame_data(protocolHandle, endpointBaseRadar, wait)); }
void Radar::request_device_info() const { handle_error_codes(ep_radar_base_get_device_info(protocolHandle, endpointBaseRadar)); }
void Radar::request_chirp_duration() const { handle_error_codes(ep_radar_base_get_chirp_duration(protocolHandle, endpointBaseRadar)); }
void Radar::request_frame_format() const { handle_error_codes(ep_radar_base_get_frame_format(protocolHandle, endpointBaseRadar)); }
void Radar::request_fmcw_configuration() const { handle_error_codes(ep_radar_fmcw_get_fmcw_configuration(protocolHandle, endpointFmcwRadar)); }
void Radar::request_chirp_velocity() const { handle_error_codes(ep_radar_fmcw_get_bandwidth_per_second(protocolHandle, endpointFmcwRadar)); }
void Radar::request_adc_configuration() const { handle_error_codes(ep_radar_adcxmc_get_adc_configuration(protocolHandle, endpointAdcRadar)); }

void Radar::handle_error_codes(int32_t error_code) {

    if (-2000 < error_code and error_code <= -1000) {
        throw std::runtime_error("lost connection to radar");
    }
}

void Radar::start_measurement(int interval_us) const {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                              endpointBaseRadar,
                                              interval_us);
}

void Radar::stop_measurement() const {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                              endpointBaseRadar,
                                              0);
}

void Radar::set_frame_format(const Frame_Format_t *fmt) const {
    ep_radar_base_set_frame_format(protocolHandle, endpointBaseRadar, fmt);
}

void Radar::set_pga_level(uint16_t ppa_level) const {
    ep_radar_p2g_set_pga_level(protocolHandle, endpointP2GRadar, ppa_level);
}