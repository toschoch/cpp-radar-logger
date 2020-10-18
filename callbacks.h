//
// Created by tobi on 26.09.20.
//

#ifndef RADARREADER_CALLBACKS_H
#define RADARREADER_CALLBACKS_H

#include <EndpointRadarBase.h>
#include <EndpointRadarFmcw.h>

int radar_auto_connect(void);

void on_frame_format_setting_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Frame_Format_t *frame_format);
void on_device_info_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Device_Info_t *device_info);
void on_chirp_duration_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint32_t chirp_duration_ns);
void on_fmcw_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Fmcw_Configuration_t *fmcw_configuration);
void on_adc_gain_level_received(void *context, int32_t protocol_handle, uint8_t endpoint, uint16_t pga_level_val);
void on_adc_config_received(void *context, int32_t protocol_handle, uint8_t endpoint, const Adc_Xmc_Configuration_t *adc_configuration);

#endif //RADARREADER_CALLBACKS_H
