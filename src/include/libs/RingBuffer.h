#ifndef WIFI_MICROPHONE_RECEIVER_LIBS_RINGBUFFER
#define WIFI_MICROPHONE_RECEIVER_LIBS_RINGBUFFER

#include "libs/Frame.h"

#include <stdint.h> // uint*_t

template <class T> class RingBuffer {
    public:
        RingBuffer(uint8_t _frameSize, uint16_t _bufferSize);
        void advanceWriteHead();
        void advanceReadHead();

        Frame<T> read();
        void write(Frame<T> data);

    private:
        uint16_t bufferSize;

        Frame<T>* buffer;
        uint16_t readHead, writeHead;
};

#endif