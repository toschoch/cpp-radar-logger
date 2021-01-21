//
// Created by tobias.schoch on 21/01/2021.
//

#ifndef RADARREADER_RADAR_ENUMS_H
#define RADARREADER_RADAR_ENUMS_H

#include <map>
#include <string>

#include <EndpointRadarBase.h>
#include <EndpointRadarFmcw.h>

#include "../include/utils.h"

// radar enums
const map<Signal_Part_t, string> signal_part_names =
        {{Signal_Part_t::EP_RADAR_BASE_SIGNAL_I_AND_Q, "I/Q"},
         {Signal_Part_t ::EP_RADAR_BASE_SIGNAL_ONLY_I, "I"},
         {Signal_Part_t ::EP_RADAR_BASE_SIGNAL_ONLY_Q, "Q"}};

const auto signal_part_enums = reverse_map(signal_part_names);

const map<Rx_Data_Format_t, string> data_format_names =
        {{Rx_Data_Format_t::EP_RADAR_BASE_RX_DATA_COMPLEX, "Complex"},
         {Rx_Data_Format_t ::EP_RADAR_BASE_RX_DATA_COMPLEX_INTERLEAVED, "Complex interleaved"},
         {Rx_Data_Format_t ::EP_RADAR_BASE_RX_DATA_REAL, "Real"}};

const auto data_format_enums = reverse_map(data_format_names);

const map<Chirp_Direction_t, string> chirp_direction_names =
        {{Chirp_Direction_t::EP_RADAR_FMCW_DIR_UPCHIRP_ONLY, "up"},
         {Chirp_Direction_t::EP_RADAR_FMCW_DIR_DOWNCHIRP_ONLY, "down"},
         {Chirp_Direction_t::EP_RADAR_FMCW_DIR_ALTERNATING_FIRST_UP, "alternating first up"},
         {Chirp_Direction_t::EP_RADAR_FMCW_DIR_ALTERNATING_FIRST_DOWN, "alternating first down"}};

const auto chirp_direction_enums = reverse_map(chirp_direction_names);

#endif //RADARREADER_RADAR_ENUMS_H
