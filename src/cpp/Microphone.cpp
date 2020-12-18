#include "Microphone.h"

/* Initialization stuff */

Microphone::Microphone(int tcp_socket, jack_client_t *jack_client) : 
    circular_buffer_(BUFFER_SIZE),
    tcp_socket_(tcp_socket),
    jack_client_(jack_client)
{
    last_packet_ = get_time();
    std::thread (init_task,this).detach();
    
    // Get address of the target microphone
    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);
    getpeername(tcp_socket_, (sockaddr *)&address, &address_len);
    address_ = address.sin_addr.s_addr;
}

void Microphone::init_task(Microphone *mic) {
    init_packet_t packet;

    while(1) {
        recv(mic->tcp_socket_, mic->tcp_buffer_, TCP_BUFFER_SIZE, 0);
        if(mic->tcp_buffer_[0] == INIT_CODE) {
            memcpy(&packet, mic->tcp_buffer_, sizeof(init_packet_t));
            break;
        }
    }
    mic->last_packet_ = get_time();

    char name[18];
    sprintf(name, "%02X:%02X:%02X:%02X:%02X:%02X", packet.mac[0], packet.mac[1], packet.mac[2], packet.mac[3], packet.mac[4], packet.mac[5]);
    mic->jack_port_ = jack_port_register(mic->jack_client_, name,
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput | JackPortIsPhysical | JackPortIsTerminal, 0);
    if(mic->jack_port_ == NULL) {
        mic->shutdown_ = true;
        return;
    }

    memcpy(mic->mac, packet.mac, 6);
    mic->sample_rate_ = packet.sample_rate;
    mic->frame_size_ = packet.frame_size;

    mic->jack_buffer_ = new float[jack_get_buffer_size(mic->jack_client_)];
    mic->float_buffer_ = new float[mic->frame_size_];
    mic->int_buffer_ = new int16_t[mic->frame_size_];

    int error;
    mic->decoder_ = opus_decoder_create(48000, 1, &error);
    opus_decoder_ctl(mic->decoder_, OPUS_SET_GAIN(GAIN));
    if(error < 0) {
		printf("Error creating decoder.\n");
	}

    printf("New microphone with MAC: %02X:%02X:%02X:%02X:%02X:%02X!\n", packet.mac[0], packet.mac[1], packet.mac[2], packet.mac[3], packet.mac[4], packet.mac[5]);
    printf("Sample Rate: %i, Frame Size: %i\n", packet.sample_rate, packet.frame_size);
}



/* Audio stuff */

void Microphone::audio_receive(unsigned char *data, size_t len, in_addr_t address) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    if(address == address_) {
        last_packet_ = get_time();
        memcpy(packet_.data, data, len);

        if(packet_.packet.packet_number <= last_packet_number_) {
            printf("Warning: Packets coming in the wrong order.\n");
        } else {
            if(packet_.packet.packet_number != last_packet_number_ + 1) {
                printf("Warning: %i packets lost!\n", packet_.packet.packet_number-(last_packet_number_+1));
                decode_packet_loss(packet_.packet.packet_number-(last_packet_number_+1));
            }
        }

        decode(packet_.packet.data, packet_.packet.data_length);

        last_packet_number_ = packet_.packet.packet_number;
    }
}

void Microphone::decode(unsigned char *data, uint8_t data_length) {
    decoded_samples_ = opus_decode(decoder_, data, data_length, int_buffer_, frame_size_, 0);
    
    for(int i = 0; i < frame_size_; i++) {
        float_buffer_[i] = (float)int_buffer_[i] / 32768.0;
    }
    push_audio(float_buffer_, frame_size_);
}

void Microphone::decode_packet_loss(uint16_t packets_lost) {
    uint16_t frames_lost = packets_lost * frame_size_;
    int16_t buffer[frames_lost];
    decoded_samples_ = opus_decode(decoder_, NULL, 0, buffer, frames_lost, 1);
    if(frames_lost < BUFFER_SIZE) {
        float float_buffer[frames_lost];
        for(int i = 0; i < frames_lost; i++) {
            float_buffer_[i] = (float)buffer[i] / 32768.0;
        }
        push_audio(float_buffer, frames_lost);
    } else {
        float float_buffer[BUFFER_SIZE];
        for(int i = 0; i < BUFFER_SIZE; i++) {
            float_buffer[i] = (float)buffer[i + (frames_lost - BUFFER_SIZE)] /  32768.0;
        }
        push_audio(float_buffer, BUFFER_SIZE);
    }
}

void Microphone::push_audio(float *pcm, size_t len) {
    circular_buffer_.push(float_buffer_, len);
    if(circular_buffer_.size() >= BUFFER_SIZE * 0.8) {
        playback_ready_ = true;
    }
}

void Microphone::audio_play(jack_nframes_t nframes) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    output_ = (jack_default_audio_sample_t*)jack_port_get_buffer (jack_port_, nframes);
    if(playback_ready_) {
        // printf("Buffer size: %li\n", circular_buffer_.size());
        circular_buffer_.pop(jack_buffer_, nframes);
        memcpy(output_, jack_buffer_, sizeof(float) * nframes);
    }
}



/* Misc stuff */

bool Microphone::remove() {
    if(get_time() - last_packet_ > TIMEOUT || shutdown_) {
        shutdown_ = true;
        shutdown();
        return true;
    } else {
        return false;
    }
}

void Microphone::shutdown() {
    jack_port_unregister(jack_client_, jack_port_);
    close(tcp_socket_);
}
uint64_t Microphone::get_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}