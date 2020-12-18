#ifndef AUDIO_PACKET_H
#define AUDIO_PACKET_H

#include <inttypes.h>

struct AudioPacket_t {
  uint32_t packet_number;
  uint8_t data_length;
  unsigned char data[40];
};

union AudioPacket {
  AudioPacket_t packet;
  unsigned char data[sizeof(AudioPacket_t)];
};

#endif