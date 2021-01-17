//
// Created by tobi on 26.09.20.
//

#include <iostream>
#include <EndpointRadarBase.h>
#include <COMPort.h>
#include <cstring>
#include <Protocol.h>
#include <EndpointRadarFmcw.h>
#include <EndpointRadarAdcxmc.h>
#include "../include/radar.h"

using namespace std;

int radar_auto_connect(void)
{
    int radar_handle = 0;
    int num_of_ports = 0;
    char comp_port_list[256];
    char* comport;
    const char *delim = ";";

    //----------------------------------------------------------------------------
    num_of_ports = com_get_port_list(comp_port_list, (size_t)256);

    if (num_of_ports == 0)
    {
        std::cout << "no comports found!" << std::endl;
        return -1;
    }
    else
    {
        comport = strtok(comp_port_list, delim);

        while (num_of_ports > 0)
        {
            num_of_ports--;

            std::cout << "try comport '" << comport << "'..." << std::endl;

            // open COM port
            radar_handle = protocol_connect(comport);

            if (radar_handle >= 0)
            {
                break;
            }

            comport = strtok(nullptr, delim);
        }

        return radar_handle;
    }

}

void Radar::on_frame_format_setting_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Frame_Format_t *frame_format)
{
    auto radar = (Radar*) context;

    radar->settings["data"]["chirps per frame"] = frame_format->num_chirps_per_frame;
    radar->settings["data"]["samples per chrip"] = frame_format->num_samples_per_chirp;
    radar->settings["data"]["signal part"] = radar->signal_part_names.at(frame_format->eSignalPart);

    vector<int> activated;
    for (auto ant=0;ant<8;++ant) {
        if (frame_format->rx_mask & (0x1 << ant))
            activated.push_back(ant);
    }
    radar->settings["antennas"]["rx"]["activated"] = activated;

    radar->store_settings();
}

void Radar::on_device_info_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Device_Info_t *device_info)
{
    auto radar = (Radar*) context;

    radar->settings["device"] = device_info->description;
    radar->settings["antennas"]["tx"]["count"] = device_info->num_tx_antennas;
    radar->settings["antennas"]["rx"]["count"] = device_info->num_rx_antennas;
    radar->settings["frequency"]["limit"]["lower"] = device_info->min_rf_frequency_kHz;
    radar->settings["frequency"]["limit"]["upper"] = device_info->max_rf_frequency_kHz;
    radar->settings["frequency"]["unit"] = "kHz";
    radar->settings["antennas"]["tx"]["power"]["max"] = device_info->max_tx_power;
    radar->settings["data"]["format"] = radar->data_format_names.at(device_info->data_format);
    radar->settings["temperature sensors"]["count"] = device_info->num_temp_sensors;

    radar->store_settings();
}

void Radar::on_chirp_duration_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint32_t chirp_duration_ns)
{
    auto radar = (Radar*) context;
    radar->settings["frequency"]["chirp"]["duration ns"] = chirp_duration_ns;

    radar->store_settings();
}

void Radar::on_fmcw_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Fmcw_Configuration_t *fmcw_configuration)
{
    auto radar = (Radar*) context;
    radar->settings["frequency"]["chirp"]["direction"] = radar->chirp_direction_names.at(fmcw_configuration->direction);
    radar->settings["frequency"]["low"] = fmcw_configuration->lower_frequency_kHz;
    radar->settings["frequency"]["high"] = fmcw_configuration->upper_frequency_kHz;
    radar->settings["frequency"]["bandwidth"] = fmcw_configuration->upper_frequency_kHz - fmcw_configuration->lower_frequency_kHz;
    radar->settings["frequency"]["avg"] = (fmcw_configuration->lower_frequency_kHz + fmcw_configuration->upper_frequency_kHz) / 2.0;
    radar->settings["frequency"]["unit"] = "kHz";
    radar->settings["antennas"]["tx"]["power"]["current"] = fmcw_configuration->tx_power;

    radar->store_settings();
}
void Radar::on_fmcw_bandwith_per_second_received(void *context, int32_t protocol_handle, uint8_t endpoint,
                                                 uint32_t bandwidth_per_second) {
    auto radar = (Radar*) context;

    radar->settings["frequency"]["chirp"]["velocity kHz/s"] = bandwidth_per_second;

    radar->store_settings();
}

void Radar::on_adc_gain_level_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint16_t pga_level_val)
{
    auto radar = (Radar*) context;
    radar->settings["sampling"]["programmable gain level"] = pga_level_val;

    radar->store_settings();
}

void Radar::on_adc_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Adc_Xmc_Configuration_t *adc_configuration)
{
    auto radar = (Radar*) context;
    radar->settings["sampling"]["frequency Hz"] = adc_configuration->samplerate_Hz;

    radar->store_settings();
}