#ifndef AUDIO_CIRCULAR_BUFFER
#define AUDIO_CIRCULAR_BUFFER

#include <cstddef> // size_t
#include <mutex> // std::mutex
#include <exception> // std::range_error
#include <cstring> // memcpy
#include <iostream> // printf


#define MAX_INDEX(size) (size-1)

class AudioCircularBuffer {
    public:
        AudioCircularBuffer(size_t capacity);

        void push(float *pcm, size_t samples);
        void pop(float *pcm, size_t samples);
        long size(); // Gets how many novel samples are in the buffer
        void show();

    private:
        std::mutex mutex_;
        float *buffer_;

        size_t samples_commited_ = 0;
        size_t head_ = 0;
        size_t tail_ = 0;

        const size_t capacity_;
        long size_ = 0;
};

#endif