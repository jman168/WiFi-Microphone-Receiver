#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h>
#include <thread>
#include <cstring>
#include <chrono>
#include <jack/jack.h>
#include <errno.h>
#include <mutex>

#include "ControlPackets.h"
#include "AudioPacket.h"
#include "AudioCircularBuffer.h"
#include "opus.h"

#define TCP_BUFFER_SIZE 1024
#define TIMEOUT 10000 //ms

#define BUFFER_SIZE 240 * 8

#define GAIN 7000 // Q8 dB

class Microphone {
    public:
        /* The constructor */
        Microphone(int tcp_socket, jack_client_t *jack_client);
        /* Audio receive callback. This function should be called for every received packet, the function will figure out which ones to ignore. */
        void audio_receive(unsigned char *data, size_t len, in_addr_t address);
        /* Audio playback callback. */
        void audio_play(jack_nframes_t nframes);
        /* A flag indicating if this microphone is dead and should be removed from any records of it. */
        bool remove();

    private:
        /* The shutdown routine */
        void shutdown();
        /* The initialization task which will be run in a thread */
        static void init_task(Microphone *mic);
        /* A function to get the current time in milliseconds */
        static uint64_t get_time();
        /* Encodes the data and puts it into the circular buffer */
        void decode(unsigned char *data, uint8_t data_length);
        void decode_packet_loss(uint16_t packets_lost);
        void push_audio(float *pcm, size_t len);

        // Audio stuff
        std::mutex buffer_mutex_;
        AudioCircularBuffer circular_buffer_;
        float *jack_buffer_;
        float *float_buffer_;
        int16_t *int_buffer_;
        int decoded_samples_;
        OpusDecoder *decoder_;
        
        jack_client_t *jack_client_;
        jack_port_t *jack_port_;
        jack_default_audio_sample_t *output_;
        bool playback_ready_ = false;

        
        // Network stuff
        AudioPacket packet_;
        uint64_t last_packet_;
        uint32_t last_packet_number_ = 0;
        const int tcp_socket_;
        in_addr_t address_;
        unsigned char tcp_buffer_[TCP_BUFFER_SIZE];


        // Info about the microphone 
        bool shutdown_ = false;
        size_t frame_size_, sample_rate_;
        char mac[6];
};

#endif