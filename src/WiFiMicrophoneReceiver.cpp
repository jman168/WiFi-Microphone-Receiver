/** @file simple_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <math.h>

#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include <jack/jack.h>
#include <opus/opus.h>

#include "RingBuffer.h"

jack_port_t *output_port;
jack_client_t *client;

#define PORT     3333

bool playback = false;

struct UDPPacket_t {
  uint32_t packet_number;
  uint8_t data_length;
  unsigned char data[40];
};

union UDPPacket {
  UDPPacket_t packet;
  unsigned char data[sizeof(UDPPacket_t)];
};

struct Frame {
	float samples[256];
	Frame() {
		for(int i = 0; i < 256; i++) {
			samples[i] = 0.0;
		}
	}
};

int sockfd; 

RingBuffer<Frame> buffer(30);
Frame empty;

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int process (jack_nframes_t nframes, void *arg)
{
    jack_default_audio_sample_t *out;
	
	out = (jack_default_audio_sample_t*)jack_port_get_buffer (output_port, nframes);
	if(playback) {
		if(buffer.empty()) {
			printf("WARNING: Underrun detected!\n");
		}
		memcpy(out, buffer.pop().samples, sizeof(float) * 256);
	} else {
		memcpy(out, empty.samples, sizeof(float) * 256);
	}

	return 0;      
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
	exit (1);
}

bool active = false;
void activate() {
	if(!active) {
		/* Tell the JACK server that we are ready to roll.  Our
		 * process() callback will start running now. */

		if (jack_activate (client)) {
			fprintf (stderr, "cannot activate client");
			exit (1);
		}
		active = true;
	}
}

void start_socket() {
	struct sockaddr_in servaddr; 

	memset(&servaddr, 0, sizeof(servaddr)); 

	servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

	if ( bind(sockfd, (const struct sockaddr *)&servaddr,  sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
}

int main (int argc, char *argv[])
{
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

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (client, process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. 
	 */

	printf ("engine sample rate: %" PRIu32 "\n",
		jack_get_sample_rate (client));

	/* create two ports */

	output_port = jack_port_register (client, "output",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput | JackPortIsPhysical | JackPortIsTerminal, 0);

	if ((output_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}

	UDPPacket packet = {};
	Frame frame = {};	

	uint32_t last_packet = 0;

	// activate();

	start_socket();

	int error;
	OpusDecoder *decoder;
	decoder = opus_decoder_create(48000, 1, &error);
	if(error < 0) {
		printf("Error creating decoder.\n");
	}

	int16_t pcm[240];

	std::ofstream output;
	output.open("audio.raw");

	while(1) {
		recv(sockfd, packet.data, sizeof(UDPPacket_t), 0);
		printf("Data size: %i\n", packet.packet.data_length);
		opus_decode(decoder, packet.packet.data, packet.packet.data_length, pcm, 240, 0);
		output.write((const char*)pcm, 240*sizeof(int16_t));



		// if(buffer.full()) {
		// 	printf("WARNING: Overrun detected!\n");
		// }

		// if(last_packet + 1 != packet.packet.packet_number)
		// 	printf("WARNING: Packet loss detected!");
		// last_packet = packet.packet.packet_number;

		// if(buffer.size() == 22) {
		// 	playback = true;
		// }

		// buffer.push(frame);


		// output.write((const char *)frame.samples, 256*4);
	}

	// std::ofstream output;
	// output.open("audio.raw");
	// while(1) {
	// 	recv(sockfd, packet.data, sizeof(UDPPacket_t), MSG_WAITALL);
	// 	output.write((const char *)packet.packet.samples, 256*2);
	// }

	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/

	jack_client_close (client);
	exit (0);
}
