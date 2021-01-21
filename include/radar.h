//
// Created by tobi on 15.01.21.
//

#ifndef RADARREADER_RADAR_H
#define RADARREADER_RADAR_H

#include <chrono>
#include <atomic>
#include <memory>
#include <exception>
#include <nlohmann/json.hpp>
#include <EndpointRadarBase.h>
#include <EndpointRadarFmcw.h>
#include <EndpointRadarAdcxmc.h>

#include "../include/shared_queue.h"

using json = nlohmann::json;
using namespace std;

class Radar {

    const chrono::microseconds send_settings_wait_time;

    string settings_file;
    atomic<float> reconnection_interval_s;

    // thread safety
    atomic<bool> frame_triggering_activated;
    atomic<bool> main_loop;
    SharedQueue<function<void(void)>> queue;

    int protocolHandle = -1;
    int endpointBaseRadar = -1;
    int endpointFmcwRadar = -1;
    int endpointAdcRadar = -1;
    int endpointP2GRadar = -1;

    void start_automatic_frame_triggering();
    void stop_automatic_frame_triggering();
    void send_settings_to_radar();

    void unsafe_set_frame_interval(int interval_us);
    void unsafe_set_pga_level(uint16_t ppa_level);
    void unsafe_set_frame_format(shared_ptr<Frame_Format_t> fmt);
    void unsafe_set_fmcw_configuration(shared_ptr<Fmcw_Configuration_t> config);
    void unsafe_set_adc_configuration(shared_ptr<Adc_Xmc_Configuration_t> config);

public:

    Radar();
    ~Radar();

    // should be protected, but because of c callbacks
    json settings;
    function<void(const Frame_Info_t*)> data_callback;

    void store_settings();
    void restore_settings();
    void update_settings() const;

    bool connect();
    void disconnect() const;

    bool is_connected() const { return (protocolHandle >= 0); };

    void identify_available_apis();

    void register_data_received_callback(const function<void(const Frame_Info_t*)>& callback);
    void register_settings_callbacks();

    void start_measurement_loop();
    void stop_measurement();


    // settings getters
    shared_ptr<Frame_Format_t> get_settings_frame_format();
    shared_ptr<Fmcw_Configuration_t> get_settings_fmcw_configuration();
    shared_ptr<Adc_Xmc_Configuration_t> get_settings_adc_configuration();

    // setters (thread safe)
    void set_frame_interval(int interval_us);
    void set_reconnection_interval(int interval_s);
    void set_frame_format(shared_ptr<Frame_Format_t> fmt);
    void set_fmcw_configuration(shared_ptr<Fmcw_Configuration_t> config);
    void set_pga_level(uint16_t ppa_level);
    void set_adc_configuration(shared_ptr<Adc_Xmc_Configuration_t> config);


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


struct RadarConnectionLost : public virtual runtime_error {
    using runtime_error::runtime_error ;
};


#endif //RADARREADER_RADAR_H
