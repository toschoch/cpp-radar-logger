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
        return -1;
    }
    else
    {
        comport = strtok(comp_port_list, delim);

        while (num_of_ports > 0)
        {
            num_of_ports--;

            // open COM port
            radar_handle = protocol_connect(comport);

            if (radar_handle >= 0)
            {
                break;
            }

            comport = strtok(NULL, delim);
        }

        return radar_handle;
    }

}

void on_frame_format_setting_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Frame_Format_t *frame_format)
{
    cout << "Frame info:" << endl;
    cout << "chrips per frame: " << frame_format->num_chirps_per_frame << endl;
    cout << "samples per chrip: " << frame_format->num_samples_per_chirp << endl;
    cout << "real/complex/interleaved: " << frame_format->eSignalPart << endl;
    cout << "rx mask: "  << int(frame_format->rx_mask) << endl << endl;

}

void on_device_info_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Device_Info_t *device_info)
{
    cout << "Device info:" << endl;
    cout << device_info->description << endl;
    cout << "antennas tx: " << int(device_info->num_tx_antennas) << endl;
    cout << "antennas rx: " << int(device_info->num_rx_antennas) << endl;
    cout << "frequency range: [" << device_info->min_rf_frequency_kHz << " - " << device_info->max_rf_frequency_kHz << "] kHz"<< endl;
    cout << "maximal tx power: " << int(device_info->max_tx_power) << endl;
    cout << "data format: " << device_info->data_format << endl;
    cout << "temperature sensors: " << int(device_info->num_temp_sensors) << endl << endl;

}

void on_chirp_duration_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint32_t chirp_duration_ns)
{
    cout << "Chirp duration: " << chirp_duration_ns << "ns" << endl << endl;
}

void on_fmcw_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Fmcw_Configuration_t *fmcw_configuration)
{
    cout << "Chirp direction: " << fmcw_configuration->direction  << endl ;
    cout << "set frequency: [" << fmcw_configuration->lower_frequency_kHz << " - " << fmcw_configuration->upper_frequency_kHz << "] kHz"<< endl;
    cout << "set tx power: " << int(fmcw_configuration->tx_power) << endl << endl;
}

void on_adc_gain_level_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint16_t pga_level_val)
{
    cout << "Prog. gain level: " << pga_level_val << endl << endl;
}

void on_adc_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Adc_Xmc_Configuration_t *adc_configuration)
{
    cout << "ADC configuration:" << endl;
    cout << "sample frequency: " << adc_configuration->samplerate_Hz << " Hz" << endl << endl;
}