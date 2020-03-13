#ifndef WIFI_MICROPHONE_RECEIVER_LIBS_RINGBUFFER
#define WIFI_MICROPHONE_RECEIVER_LIBS_RINGBUFFER

#include <stdint.h> // uint*_t

template <class T> class RingBuffer {
    public:
        RingBuffer(uint16_t bufferSize);
        void advanceWriteHead();
        void advanceReadHead();

        T read();
        void write(T data);

    private:
        uint16_t bufferSize;

        T* buffer;
        uint16_t readHead, writeHead;
};

#endif