//
// Created by tobi on 15.01.21.
//

#ifndef RADARREADER_RADAR_H
#define RADARREADER_RADAR_H

#include <exception>
#include <nlohmann/json.hpp>
#include "EndpointRadarBase.h"
#include <EndpointRadarFmcw.h>
#include <EndpointRadarAdcxmc.h>

using json = nlohmann::json;

class Radar {

    std::string settings_file;
    int reconnection_interval_s = 0;

    int protocolHandle = -1;
    int endpointBaseRadar = -1;
    int endpointFmcwRadar = -1;
    int endpointAdcRadar = -1;
    int endpointP2GRadar = -1;

    // radar enums
    const std::map<Signal_Part_t, std::string> signal_part_names =
            {{Signal_Part_t::EP_RADAR_BASE_SIGNAL_I_AND_Q, "I/Q"},
             {Signal_Part_t ::EP_RADAR_BASE_SIGNAL_ONLY_I, "I"},
             {Signal_Part_t ::EP_RADAR_BASE_SIGNAL_ONLY_Q, "Q"}};

    const std::map<Rx_Data_Format_t, std::string> data_format_names =
            {{Rx_Data_Format_t::EP_RADAR_BASE_RX_DATA_COMPLEX, "Complex"},
             {Rx_Data_Format_t ::EP_RADAR_BASE_RX_DATA_COMPLEX_INTERLEAVED, "Complex interleaved"},
             {Rx_Data_Format_t ::EP_RADAR_BASE_RX_DATA_REAL, "Real"}};

    const std::map<Chirp_Direction_t, std::string> chirp_direction_names =
            {{Chirp_Direction_t::EP_RADAR_FMCW_DIR_UPCHIRP_ONLY, "up"},
             {Chirp_Direction_t::EP_RADAR_FMCW_DIR_DOWNCHIRP_ONLY, "down"},
             {Chirp_Direction_t::EP_RADAR_FMCW_DIR_ALTERNATING_FIRST_UP, "alternating first up"},
             {Chirp_Direction_t::EP_RADAR_FMCW_DIR_ALTERNATING_FIRST_DOWN, "alternating first down"}};

    void send_settings_to_radar();

public:

    Radar();
    ~Radar();

    // should be protected, but because of c callbacks
    json settings;
    std::function<void(const Frame_Info_t*)> data_callback;

    void store_settings();
    void restore_settings();
    void update_settings() const;

    bool connect();
    void disconnect() const;

    bool is_connected() const { return (protocolHandle >= 0); };

    void identify_available_apis();

    void register_data_received_callback(const std::function<void(const Frame_Info_t*)>& callback);
    void register_settings_callbacks();

    void start_measurement();
    void stop_measurement() const;

    // setters
    void set_frame_interval(int interval_us);
    void set_reconnection_interval(int interval_s);
    void set_frame_format(const Frame_Format_t* fmt) const;
    void set_fmcw_configuration(const Fmcw_Configuration_t* config) const;
    void set_pga_level(uint16_t ppa_level) const;
    void set_adc_configuration(const Adc_Xmc_Configuration_t* config) const;

    // requests
    void request_data(uint8_t wait) const;

    void request_device_info() const;
    void request_chirp_duration() const;
    void request_minimal_frame_interval() const;
    void request_frame_format() const;
    void request_fmcw_configuration() const;
    void request_chirp_velocity() const;
    void request_adc_configuration() const;
    void request_adc_gain_level() const;


    // callbacks
    static void on_frame_format_setting_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Frame_Format_t *frame_format);
    static void on_device_info_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Device_Info_t *device_info);
    static void on_chirp_duration_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint32_t chirp_duration_ns);
    static void on_fmcw_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Fmcw_Configuration_t *fmcw_configuration);
    static void on_fmcw_bandwith_per_second_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint32_t bandwidth_per_second);
    static void on_adc_gain_level_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint16_t pga_level_val);
    static void on_adc_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Adc_Xmc_Configuration_t *adc_configuration);
    static void on_minimal_frame_interval_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint32_t min_frame_interval_us);

    static void handle_error_codes(int32_t error_code);

};

int radar_auto_connect(void);


struct RadarConnectionLost : public virtual std::runtime_error {
    using std::runtime_error::runtime_error ;
};


#endif //RADARREADER_RADAR_H
