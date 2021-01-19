
#include <iostream>
#include <vector>
#include "Protocol.h"
#include "EndpointRadarBase.h"
#include "../include/radar.h"
#include "../include/utils.h"
#include "../include/arrow.h"
#include "../include/zmq.h"
#include <cstdlib>
#include <chrono>
#include <exception>

using namespace std;
using namespace std::chrono_literals;

#define AUTOMATIC_DATA_TRIGGER_TIME_US (30000)	// get ADC data each 1ms in automatic trigger mode


int main(void)
{
    auto zmq_server = ZMQ();
    auto radar = Radar();

    cout << "try to find connected radar..." << endl;
    radar.connect();

    while (!radar.is_connected()) {
        cout << "could not find radar! wait for 15s..." << endl;
        std::this_thread::sleep_for(15s);
        cout << "try to find connected radar..." << endl;
        radar.connect();
    }

    cout << "success! connecting...";

    zmq_server.start();

    // called every time ep_radar_base_get_frame_data method is called to return measured time domain signals
    auto received_frame_data = [&zmq_server](const Frame_Info_t* frame_info)
    {
        auto t = std::chrono::system_clock::now();
        if (frame_info->frame_number % 50 == 0)
            cout << "frame " << frame_info->frame_number << endl;
/*
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
                    //cout << fixed << frame_info->sample_data[offset] << "+" << frame_info->sample_data[offset+1] << "j ";
                }
                //cout << endl;
            }
        }*/
        vector<int64_t> dims;
        dims.push_back(frame_info->num_rx_antennas);
        dims.push_back(frame_info->num_chirps);
        dims.push_back(frame_info->num_samples_per_chirp);
        dims.push_back(frame_info->data_format==EP_RADAR_BASE_RX_DATA_REAL ? 1 : 2);

        auto elements = accumulate(begin(dims), end(dims), 1, multiplies<int>());
        vector<float> data;
        data.assign(frame_info->sample_data, frame_info->sample_data+elements);

        auto buf = create_and_serialize_tensor(dims, data);
        zmq_server.send_buffer(buf);
    };

    radar.register_data_received_callback(received_frame_data);

    cout << "done!" << endl << endl;
    radar.update_settings();

    // disable automatic trigger
    radar.stop_measurement();

    cout << "configure...";
    auto fmt = new Frame_Format_t;
    fmt->num_chirps_per_frame = 32;
    fmt->num_samples_per_chirp = 128;
    fmt->rx_mask = 0x03;
    fmt->eSignalPart = EP_RADAR_BASE_SIGNAL_I_AND_Q;
    radar.set_frame_format(fmt);
    radar.set_pga_level(3);
    cout << "done!" << endl;

    cout << radar.settings.dump(4) << endl;

    radar.start_measurement(1500000);

    exit(0);
}