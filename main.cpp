
#include <stdio.h>
#include <string.h>
#include <opencv2/core.hpp>
#include <iostream>
#include <vector>
#include "Protocol.h"
#include "COMPort.h"
#include "EndpointRadarBase.h"
#include "EndpointRadarFmcw.h"
#include "Logger.h"

using namespace std;

#define AUTOMATIC_DATA_TRIGGER_TIME_US (1500000)	// get ADC data each 1ms in automatic trigger mode

Logger logger("data");

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
    cout << "frame " << frame_info->frame_number << endl; //<< " processed" << std::endl;


    for (int ant=0; ant<frame_info->num_rx_antennas; ant++)
    {
        cout << "read data from antenna " << ant << endl;
        cv::Mat data(frame_info->num_chirps, frame_info->num_samples_per_chirp, CV_32FC2);
        for (int chirp=0; chirp<frame_info->num_chirps; chirp++)
        {
            int chirp_start = chirp * frame_info->num_rx_antennas * frame_info->num_samples_per_chirp *
                    (frame_info->data_format==EP_RADAR_BASE_RX_DATA_REAL ? 1 : 2);
            for (int sample=0; sample<frame_info->num_samples_per_chirp; sample++)
            {
                int offset = chirp_start +
                        2 * ant*frame_info->num_samples_per_chirp + sample;
                data.at<cv::Vec2f>(chirp, sample)[0] = frame_info->sample_data[offset];
                data.at<cv::Vec2f>(chirp, sample)[1] = frame_info->sample_data[offset+1];
                //cout << fixed << frame_info->sample_data[offset] << "+" << frame_info->sample_data[offset+1] << "j ";
            }
            //cout << endl;

        }
        cv::Mat data_fft;
        cv::dft(data,data_fft,cv::DFT_ROWS|cv::DFT_COMPLEX_INPUT|::cv::DFT_COMPLEX_OUTPUT);
        cout << data_fft;
        cout << endl;

        logger.append_row(t,"antenna"+std::to_string(ant)+"_time",data);
        //logger.append_row(t,"antenna"+std::to_string(ant)+"_frequency",data_fft);
    }

    logger.close();
}

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

int main(void)
{
    int res = -1;
    int protocolHandle = 0;
    int endpointBaseRadar = 0;
    int endpointFmcwRadar = 0;

    cout.precision(3);

    // open COM port
    std::cout << "try to find connected radar..." << std::endl;
    protocolHandle = radar_auto_connect();

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

        cout << "done!" << endl << endl;
        res = ep_radar_base_get_device_info(protocolHandle, endpointBaseRadar);

        res = ep_radar_base_get_frame_format(protocolHandle, endpointBaseRadar);


        // enable automatic trigger
        res = ep_radar_base_set_automatic_frame_trigger(protocolHandle,
                                                        endpointBaseRadar,
                                                        AUTOMATIC_DATA_TRIGGER_TIME_US);

        try {
            while (1) {
                // get raw data
                res = ep_radar_base_get_frame_data(protocolHandle,
                                                   endpointBaseRadar,
                                                   1);
            }
        }
        catch (...) {
            logger.close();
        }
    }

    return res;
}