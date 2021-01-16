//
// Created by tobi on 15.01.21.
//

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include "../include/radar.h"
#include "../include/callbacks.h"

#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "EndpointRadarFmcw.h"
#include "EndpointRadarAdcxmc.h"
#include "EndpointRadarP2G.h"

Radar::Radar() {
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

void Radar::disconnect() {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle, endpointBaseRadar,0);
    protocol_disconnect(protocolHandle);
}

const std::string Radar::settings_file("settings.json");

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
        ep_radar_base_set_callback_device_info(on_device_info_received, NULL);
        //ep_radar_base_set_callback_data_frame(received_frame_data, NULL);
        ep_radar_base_set_callback_frame_format(on_frame_format_setting_received, NULL);
        ep_radar_base_set_callback_chirp_duration(on_chirp_duration_received, NULL);
    }

    if (endpointFmcwRadar) {
        ep_radar_fmcw_set_callback_fmcw_configuration(on_fmcw_config_received, NULL);
    }

    if (endpointAdcRadar) {
        ep_radar_adcxmc_set_callback_adc_configuration(on_adc_config_received, NULL);
    }

    if (endpointP2GRadar) {
        ep_radar_p2g_set_callback_pga_level(on_adc_gain_level_received, NULL);
    }
}

void Radar::request_device_info() { ep_radar_base_get_device_info(protocolHandle, endpointBaseRadar); };


void Radar::start_measurement(int interval_us) {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                              endpointBaseRadar,
                                              interval_us);
}

void Radar::stop_measurement() {
    ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                              endpointBaseRadar,
                                              0);
}

void Radar::set_frame_format(const Frame_Format_t *fmt) {
    ep_radar_base_set_frame_format(protocolHandle, endpointBaseRadar, fmt);
}