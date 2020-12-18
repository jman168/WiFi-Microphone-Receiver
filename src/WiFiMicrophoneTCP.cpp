#include "Microphone.h"

#include <netinet/in.h> 
#include <sys/socket.h>
#include <thread>
#include <mutex>

#include <jack/jack.h>

#define PERIODIC_LOOP_SLEEP_TIME 500//ms

std::mutex microphones_mutex;
Microphone *microphones[32];
int num_microphones = 0;

int tcp_socket_fd, udp_socket_fd;

jack_client_t *jack_client;

int jack_process(jack_nframes_t nframes, void *arg) {
    std::lock_guard<std::mutex> lock(microphones_mutex);
    for(int i = 0; i < num_microphones; i++) {
        microphones[i]->audio_play(nframes);
    }

    return 0;
}

void periodic_task() {
    std::lock_guard<std::mutex> lock(microphones_mutex);
    for(int i = 0; i < num_microphones; i++) { // Cycle through all microphone
        if(microphones[i]->remove()) { // Check if the remove flag is set
            delete microphones[i];
            microphones[i] = nullptr;
            for(int x = i; x < num_microphones-1; x++) { // Shift all microphone objects over by one starting with but not including the one to be remove
                microphones[x] = microphones[x + 1];
            }
            microphones[num_microphones - 1] = nullptr;
            num_microphones--; // Remove one from the microphone total count
        }
    }
}

void periodic_loop() {
    while(1) {
        periodic_task();
        std::this_thread::sleep_for(std::chrono::milliseconds(PERIODIC_LOOP_SLEEP_TIME));
    }
}

void receive_task(unsigned char *data, size_t len, in_addr_t address) {
    std::lock_guard<std::mutex> lock(microphones_mutex);
    for(int i = 0; i < num_microphones; i++) {
        microphones[i]->audio_receive(data, len, address);
    }
}

void receive_loop() {
    udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); // Create a new tcp socket
    
    // Tell the socket to reuse the address
    int opt = 1; 
    setsockopt(udp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address; 
    int addrlen = sizeof(address); 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(3333);

    bind(udp_socket_fd, (const sockaddr *)&address, sizeof(address));
    
    struct sockaddr_in from_address; 
    socklen_t from_address_length = sizeof(from_address);
    unsigned char buffer[45];

    while(1) {
        recvfrom(udp_socket_fd, buffer, 45, 0, (sockaddr *)&from_address, &from_address_length);
        // printf("Packet from address %u.%u.%u.%u\n", (uint8_t)(from_address.sin_addr.s_addr >> 0), (uint8_t)(from_address.sin_addr.s_addr >> 8), (uint8_t)(from_address.sin_addr.s_addr >> 16),(uint8_t)(from_address.sin_addr.s_addr >> 24));
        receive_task(buffer, 45, from_address.sin_addr.s_addr);
    }
}

void append_microphone(Microphone *microphone) {
    std::lock_guard<std::mutex> lock(microphones_mutex);
    microphones[num_microphones] = microphone;
    num_microphones++;
}

int process (jack_nframes_t nframes, void *arg)
{
	return 0;      
}

void jack_shutdown (void *arg) { exit (1); }

void start_jack() {
    jack_status_t status;
    jack_client = jack_client_open ("WiFi Microphones", JackNullOption, &status);
    jack_on_shutdown(jack_client, jack_shutdown, 0);
    jack_set_process_callback (jack_client, jack_process, 0);
    jack_activate(jack_client);
}

int main (int argc, char *argv[])
{
    start_jack();

    tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0); // Create a new tcp socket
    
    // Tell the socket to reuse the address
    int opt = 1; 
    setsockopt(tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address; 
    int addrlen = sizeof(address); 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(3090);

    bind(tcp_socket_fd, (const sockaddr *)&address, sizeof(address));

    std::thread(periodic_loop).detach();
    std::thread(receive_loop).detach();

    while(1) {
        listen(tcp_socket_fd, 3);
        append_microphone(new Microphone(accept(tcp_socket_fd, (sockaddr *)&address, (socklen_t*)&addrlen), jack_client));
    }
}