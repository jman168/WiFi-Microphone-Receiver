#include "AudioCircularBuffer.h"

AudioCircularBuffer::AudioCircularBuffer(size_t capacity) : capacity_(capacity) {
    buffer_ = new float[capacity_];
}

void AudioCircularBuffer::push(float *pcm, size_t samples) {
    std::lock_guard<std::mutex> lock(mutex_);

    if(samples > capacity_) { // If the source buffer is bigger than the circular buffer
        throw std::range_error("Cannot push an array of data larger than the circular buffer.");
    }

    if(head_ + samples > MAX_INDEX(capacity_)) { // If the head will cross over the max index (this can only happen once in a given write)
        samples_commited_ = capacity_ - head_; // Find the number of samples required to get to the max index
        memcpy(&buffer_[head_], pcm, samples_commited_ * sizeof(float)); // Commit those to the circular buffer
        head_ = 0; // Set the head to 0
        memcpy(&buffer_[head_], &pcm[samples_commited_], (samples - samples_commited_) * sizeof(float)); // Finish commiting the remaining samples
        head_ = (head_ + (samples - samples_commited_)) % capacity_; // Advance the head by the remaining samples commited plus one (so the next write will start on an "empty" sample)
    } else { // If the head will not cross over the max index
        memcpy(&buffer_[head_], pcm, samples * sizeof(float)); // Commit the source buffer to the circular buffer
        head_ = (head_ + samples) % capacity_; // Advance the head by the samples commited plus one (so the next write will start on an "empty" sample)
    }

    if(size_ + samples > capacity_) { // If writing to the buffer will loop over
        size_ = capacity_; // Set the size to it's maximum
        tail_ = head_; // Set the tail equal to the head (max it the maximum it can be)
    } else { // else
        size_ = size_ + samples;
    }
}

void AudioCircularBuffer::pop(float *pcm, size_t samples) {
    std::lock_guard<std::mutex> lock(mutex_);

    if(samples > capacity_) { // If the destination buffer is bigger than the circular buffer
        throw std::range_error("Cannot pop an array of data larger than the circular buffer.");
    }

    if(size_ - (long)samples < 0) { // If there is not enough data to read
        for(int i = 0; i < samples; i++) { pcm[i] = 0.0; } // zero the destination buffer
        samples = size_; // tell the next chunk of code only to read the available samples
    }

    if(tail_ + samples > MAX_INDEX(capacity_)) { // If the tail will cross over the max index (this can only happen once in a given write)
        samples_commited_ = capacity_ - tail_; // Find the number of samples required to get to the max index
        memcpy(pcm, &buffer_[tail_], samples_commited_ * sizeof(float)); // Commit those to the destination buffer
        tail_ = 0; // Set the tail to 0
        memcpy(&pcm[samples_commited_], &buffer_[tail_], (samples - samples_commited_) * sizeof(float)); // Finish commiting the remaining samples
        tail_ = (tail_ + (samples - samples_commited_)) % capacity_; // Advance the tail by the remaining samples commited plus one (so the next read will start on an "empty" sample)
    } else { // If the tail will not cross over the max index
        memcpy(pcm, &buffer_[tail_], samples * sizeof(float)); // Commit the circular buffer to the destination buffer
        tail_ = (tail_ + samples) % capacity_; // Advance the tail by the samples commited plus one (so the next read will start on an "empty" sample)
    }

    size_ = size_ - samples; // Set the size
}

long AudioCircularBuffer::size() {
    return size_;
}

void AudioCircularBuffer::show() {
    for(int i = 0; i < capacity_; i++) {
        printf("%f,", buffer_[i]);
    }
    printf("\n");
}