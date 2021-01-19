
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

// called every time ep_radar_base_get_frame_data method is called to return measured time domain signals
void received_frame_data(const Frame_Info_t* frame_info)
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

int main(void)
{
    int res = -1;

    auto zmq_server = ZMQ();
    auto radar = Radar();

    cout << "try to find connected radar..." << endl;
    radar.connect();

    radar.register_data_received_callback(received_frame_data);

    if (radar.is_connected())
    {
        cout << "success! connecting...";
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


    vector<int64_t> dims = {2, 512, 256, 2};
    vector<float> data = generate_data(524288);

    auto buf = create_and_serialize_tensor(dims, data);

    cout << "serialized data as tensor: " << buf->size() << " bytes" << endl;

    zmq_server.start();

    auto success = zmq_server.send_buffer(buf);

    cout << "sent data through buffer " << success << endl;

    return res;
}