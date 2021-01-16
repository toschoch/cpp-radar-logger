//
// Created by tobi on 15.01.21.
//

#ifndef RADARREADER_RADAR_H
#define RADARREADER_RADAR_H

#include <nlohmann/json.hpp>
#include "EndpointRadarBase.h"

using json = nlohmann::json;

class Radar {

    const static std::string settings_file;

    int protocolHandle = -1;
    int endpointBaseRadar = -1;
    int endpointFmcwRadar = -1;
    int endpointAdcRadar = -1;
    int endpointP2GRadar = -1;

    json settings;

public:

    Radar();

    void store_settings();
    void restore_settings();

    bool connect();
    void disconnect();

    bool is_connected() { return (protocolHandle >= 0); };

    void identify_available_apis();
    void register_callbacks();

    void request_device_info();

    void start_measurement(int interval_us);
    void stop_measurement();

    void set_frame_format(const Frame_Format_t* fmt);

};


#endif //RADARREADER_RADAR_H
