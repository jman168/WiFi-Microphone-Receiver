#include <iostream>
#include <cstring>

#include <jack/jack.h>
#include "opus.h"
#include "rpi_nrf24.h"

#include "AudioCircularBuffer.h"

bool playing = false;

jack_client_t *client;
jack_port_t *output_port;

AudioCircularBuffer buffer(720);

int process (jack_nframes_t nframes, void *arg);
void int2float(int16_t *ipcm, float *fpcm, size_t len);

int main (int argc, char *argv[])
{
    nrf24_t dev;
	nrf24_init(&dev, "/dev/spidev0.0", 25);

	nrf24_set_crc(&dev, NRF24_CRC_1BYTE);
	nrf24_set_data_rate(&dev, NRF24_2MBPS);
	nrf24_set_rf_channel(&dev, 0);

	uint8_t address[5] = {0x7E, 0x7E, 0x7E, 0x7E, 0x71};
	// nrf24_set_rx_address(&dev, NRF24_P0, address, 5);
    nrf24_set_payload_length(&dev, 0);
    nrf24_power_up_rx(&dev);

	const char *client_name = "WiFi Microphone Receiver";
	jack_options_t options = JackNullOption;
	jack_status_t status;
	
	/* open a client connection to the JACK server */

	client = jack_client_open (client_name, options, &status);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

    jack_set_process_callback (client, process, 0);

    output_port = jack_port_register (client, "microphone.0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput | JackPortIsPhysical | JackPortIsTerminal, 0);

    const char **ports = jack_get_ports(client, NULL, NULL, 0);
    int i = 0; 
    while(ports[i] != NULL) {
        std::cout << ports[i] << "\n";
        i++;
    }

    if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

    if(jack_connect(client, "WiFi Microphone Receiver:microphone.0", "system:playback_1") != 0) {
        fprintf (stderr, "cannot connect port to output");
		exit (1);
    }

    int error;
	OpusDecoder *decoder;
	decoder = opus_decoder_create(48000, 1, &error);
	if(error < 0) {
		printf("Error creating decoder.\n");
	}
    opus_decoder_ctl(decoder, OPUS_SET_GAIN(9000));

    int16_t pcm[240];
    float fpcm[240];
    uint8_t data[32];

    while(true) {
        int packet_available = nrf24_get_data_available(&dev);
        if(packet_available == 1) {
            uint8_t len;
            nrf24_get_data(&dev, data, &len);
            opus_decode(decoder, data, len, pcm, 240, 0);
            int2float(pcm, fpcm, 240);
            buffer.push(fpcm, 240);

            if(buffer.size() >= 480)
                playing = true;
        }
    }
}


int process (jack_nframes_t nframes, void *arg)
{
    jack_default_audio_sample_t *out;
	
	out = (jack_default_audio_sample_t*)jack_port_get_buffer (output_port, nframes);
	if(playing) {
		if(buffer.size() <= nframes) {
			printf("WARNING: Underrun detected!\n");
		}
		buffer.pop(out, nframes);
	} else {
        memset(out, 0, nframes * sizeof(float));
	}

	return 0;      
}

void int2float(int16_t *ipcm, float *fpcm, size_t len) {
    for(size_t i = 0; i < len; i++) {
        fpcm[i] = (float)ipcm[i] / 32768.0;
    }
}