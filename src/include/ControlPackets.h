#ifndef CONTROL_PACKETS_H
#define CONTROL_PACKETS_H

#include <inttypes.h>

#define INIT_CODE 0
struct init_packet_t {
    uint8_t packet_code = 0;
    uint8_t mac[6];
    uint16_t frame_size;
    uint32_t sample_rate;
};

#endif