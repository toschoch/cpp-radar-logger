
#include <string.h>
//#include <opencv2/core/core.hpp>
#include <iostream>
#include <vector>
#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "EndpointRadarFmcw.h"
#include "EndpointRadarAdcxmc.h"
#include "EndpointRadarP2G.h"
//#include "Logger.h"
#include "../include/callbacks.h"
#include <csignal>
#include <cstdlib>
#include <chrono>

using namespace std;

#define AUTOMATIC_DATA_TRIGGER_TIME_US (30000)	// get ADC data each 1ms in automatic trigger mode

//Logger logger("/home/pi/radar/data");
int protocolHandle = 0;
int endpointBaseRadar = 0;
int endpointFmcwRadar = 0;
int endpointAdcRadar = 0;
int endpointP2GRadar = 0;


// called every time ep_radar_base_get_frame_data method is called to return measured time domain signals
void received_frame_data(void* context,
                         int32_t protocol_handle,
                         uint8_t endpoint,
                         const Frame_Info_t* frame_info)
{
    //std::vector<float> frame (frame_info->sample_data, frame_info->sample_data+frame_info->num_samples_per_chirp);

    //std::vector<float> frame_fft;
    auto t = std::chrono::system_clock::now();
    //cv::dft(frame, frame_fft);
    if (frame_info->frame_number % 50 == 0)
        cout << "frame " << frame_info->frame_number << endl; //<< " processed" << std::endl;

    for (int ant=0; ant<frame_info->num_rx_antennas; ant++)
    {
        //cout << "read data from antenna " << ant << endl;
//        cv::Mat data(frame_info->num_chirps, frame_info->num_samples_per_chirp, CV_32FC2);
        for (int chirp=0; chirp<frame_info->num_chirps; chirp++)
        {
            int chirp_start = chirp * frame_info->num_rx_antennas * frame_info->num_samples_per_chirp *
                    (frame_info->data_format==EP_RADAR_BASE_RX_DATA_REAL ? 1 : 2);
            for (int sample=0; sample<frame_info->num_samples_per_chirp; sample++)
            {
                int offset = chirp_start +
                        2 * ant*frame_info->num_samples_per_chirp + sample;
//                data.at<cv::Vec2f>(chirp, sample)[0] = frame_info->sample_data[offset];
//                data.at<cv::Vec2f>(chirp, sample)[1] = frame_info->sample_data[offset+1];
                cout << fixed << frame_info->sample_data[offset] << "+" << frame_info->sample_data[offset+1] << "j ";
            }
            cout << endl;

        }
        /*cv::Mat data_fft;
        cv::dft(data,data_fft,cv::DFT_ROWS|cv::DFT_COMPLEX_INPUT|::cv::DFT_COMPLEX_OUTPUT);
        cv::dft(data_fft.t(),data_fft,cv::DFT_ROWS|cv::DFT_COMPLEX_INPUT|::cv::DFT_COMPLEX_OUTPUT);

        cv::Mat channels[2];

        cv::split(data, channels);
        cout << channels[0] << endl;
        cout << channels[1] << endl;*/

        //logger.append_row(t,"antenna"+std::to_string(ant)+"_time",data);
        //logger.append_row(t,"antenna"+std::to_string(ant)+"_frequency",data_fft);
    }

    // assure a good file
    //logger.close();
}


int main(void)
{
    int res = -1;


    cout.precision(3);

    // open COM port
    std::cout << "try to find connected radar..." << std::endl;
    protocolHandle = radar_auto_connect();

    //signal requires lam take an int parameter
    //this parameter is equal to the signals value
    auto lam =
            [] (int i) { cout << "aborting..." << endl;
                cout << "close files..."  << endl; //logger.close();
                cout << "disconnect..."  << endl;
                ep_radar_base_set_automatic_frame_trigger(protocolHandle,endpointBaseRadar,0);
                protocol_disconnect(protocolHandle);exit(0); };

    //^C
    signal(SIGINT, lam);
    //abort()
    signal(SIGABRT, lam);
    //sent by "kill" command
    signal(SIGTERM, lam);
    //^Z
    signal(SIGTSTP, lam);



    // get endpoint ids
    if (protocolHandle >= 0)
    {
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
    } else {
        std::cout << "no radar found! exiting..." << std::endl;
        return res;
    }


    if (endpointBaseRadar >= 0)
    {
        cout << "success! connecting...";
        // register call back functions for adc data
        ep_radar_base_set_callback_device_info(on_device_info_received, NULL);
        ep_radar_base_set_callback_data_frame(received_frame_data, NULL);
        ep_radar_base_set_callback_frame_format(on_frame_format_setting_received, NULL);
        ep_radar_base_set_callback_chirp_duration(on_chirp_duration_received, NULL);
        ep_radar_fmcw_set_callback_fmcw_configuration(on_fmcw_config_received, NULL);
        ep_radar_adcxmc_set_callback_adc_configuration(on_adc_config_received, NULL);
        ep_radar_p2g_set_callback_pga_level(on_adc_gain_level_received, NULL);

        cout << "done!" << endl << endl;
        ep_radar_base_get_device_info(protocolHandle, endpointBaseRadar);


        // disable automatic trigger
        ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                                  endpointBaseRadar,
                                                  0);

        cout << "configure...";
        auto fmt = new Frame_Format_t;
        fmt->num_chirps_per_frame = 32;
        fmt->num_samples_per_chirp = 128;
        fmt->rx_mask = 0x03;
        fmt->eSignalPart = EP_RADAR_BASE_SIGNAL_I_AND_Q;
        ep_radar_base_set_frame_format(protocolHandle, endpointBaseRadar, fmt);
        cout << "done!" << endl;

        ep_radar_p2g_set_pga_level(protocolHandle, endpointP2GRadar, 3);

        ep_radar_base_get_frame_format(protocolHandle, endpointBaseRadar);

        ep_radar_base_get_chirp_duration(protocolHandle, endpointBaseRadar);
        ep_radar_fmcw_get_fmcw_configuration(protocolHandle, endpointFmcwRadar);
        ep_radar_adcxmc_get_adc_configuration(protocolHandle, endpointAdcRadar);
        ep_radar_p2g_get_pga_level(protocolHandle, endpointP2GRadar);

        // enable automatic trigger
        ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                                  endpointBaseRadar,
                                                  AUTOMATIC_DATA_TRIGGER_TIME_US);

        while (1) {
            // get raw data
            ep_radar_base_get_frame_data(protocolHandle,
                                         endpointBaseRadar,
                                         1);
        }
    }

    return res;
}