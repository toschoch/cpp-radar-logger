
#include <string.h>
#include <iostream>
#include <vector>
#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "../include/radar.h"
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <zmqpp/zmqpp.hpp>

using namespace std;

#define AUTOMATIC_DATA_TRIGGER_TIME_US (30000)	// get ADC data each 1ms in automatic trigger mode

// called every time ep_radar_base_get_frame_data method is called to return measured time domain signals
void received_frame_data(void* context,
                         int32_t protocol_handle,
                         uint8_t endpoint,
                         const Frame_Info_t* frame_info)
{
    auto t = std::chrono::system_clock::now();
    if (frame_info->frame_number % 50 == 0)
        cout << "frame " << frame_info->frame_number << endl;

    for (int ant=0; ant<frame_info->num_rx_antennas; ant++)
    {
        for (int chirp=0; chirp<frame_info->num_chirps; chirp++)
        {
            int chirp_start = chirp * frame_info->num_rx_antennas * frame_info->num_samples_per_chirp *
                              (frame_info->data_format==EP_RADAR_BASE_RX_DATA_REAL ? 1 : 2);
            for (int sample=0; sample<frame_info->num_samples_per_chirp; sample++)
            {
                int offset = chirp_start +
                             2 * ant*frame_info->num_samples_per_chirp + sample;
                cout << fixed << frame_info->sample_data[offset] << "+" << frame_info->sample_data[offset+1] << "j ";
            }
            cout << endl;

        }
    }
};


auto radar = Radar();

int main(void)
{
    int res = -1;


    // open COM port
    cout << "try to find connected radar..." << endl;
    radar.connect();

    //signal requires lam take an int parameter
    //this parameter is equal to the signals value
    auto lam =
            [] (int i) { cout << "aborting..." << endl;
                cout << "close files..."  << endl; //logger.close();
                cout << "disconnect..."  << endl;
                radar.disconnect();
                exit(0); };

    //^C
    signal(SIGINT, lam);
    //abort()
    signal(SIGABRT, lam);
    //sent by "kill" command
    signal(SIGTERM, lam);
    //^Z
    signal(SIGTSTP, lam);

    if (radar.is_connected())
    {
        cout << "success! connecting...";
        cout << "done!" << endl << endl;
        radar.request_device_info();

        // disable automatic trigger
        radar.stop_measurement();

        cout << "configure...";
        auto fmt = new Frame_Format_t;
        fmt->num_chirps_per_frame = 32;
        fmt->num_samples_per_chirp = 128;
        fmt->rx_mask = 0x03;
        fmt->eSignalPart = EP_RADAR_BASE_SIGNAL_I_AND_Q;
        radar.set_frame_format(fmt);
        cout << "done!" << endl;

        /*ep_radar_p2g_set_pga_level(protocolHandle, endpointP2GRadar, 3);

        ep_radar_base_get_frame_format(protocolHandle, endpointBaseRadar);

        ep_radar_base_get_chirp_duration(protocolHandle, endpointBaseRadar);
        ep_radar_fmcw_get_fmcw_configuration(protocolHandle, endpointFmcwRadar);
        ep_radar_adcxmc_get_adc_configuration(protocolHandle, endpointAdcRadar);
        ep_radar_p2g_get_pga_level(protocolHandle, endpointP2GRadar);

        // enable automatic trigger
        ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                                  endpointBaseRadar,
                                                  AUTOMATIC_DATA_TRIGGER_TIME_US);*/

/*        while (1) {
            // get raw data
            ep_radar_base_get_frame_data(protocolHandle,
                                         endpointBaseRadar,
                                         1);
        }*/

        exit(0);
    }

    return res;
}