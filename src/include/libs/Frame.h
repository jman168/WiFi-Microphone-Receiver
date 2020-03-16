#ifndef WIFI_MICROPHONE_RECEIVER_LIBS_FRAME
#define WIFI_MICROPHONE_RECEIVER_LIBS_FRAME

#include <stdint.h> // uint*_t

template <class T> class Frame {
    public:
        void setSize(uint8_t _size);

    private:
        uint8_t size;
        T* buffer;
};

#endif