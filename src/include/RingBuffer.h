#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <mutex>

template <class T>
class RingBuffer {
    public:
        RingBuffer(size_t size);

        void push(T item);
        T pop();

        void reset();
        
        bool empty();
        bool full();
        
        size_t capacity();
        size_t size();

    private:
        std::mutex mutex_;
        T *buf_;
        size_t head_ = 0;
        size_t tail_ = 0;
        const size_t max_size_;
        bool full_ = 0;  
};

template <class T>
RingBuffer<T>::RingBuffer(size_t size) : buf_(new T[size]), max_size_(size) {}


template <class T>
void RingBuffer<T>::push(T item)
{
	std::lock_guard<std::mutex> lock(mutex_);

	buf_[head_] = item;

	if(full_)
	{
		tail_ = (tail_ + 1) % max_size_;
	}

	head_ = (head_ + 1) % max_size_;

	full_ = head_ == tail_;
}

template <class T>
T RingBuffer<T>::pop()
{
	std::lock_guard<std::mutex> lock(mutex_);

	if(empty())
	{
		return T();
	}

	//Read data and advance the tail (we now have a free space)
	auto val = buf_[tail_];
	full_ = false;
	tail_ = (tail_ + 1) % max_size_;

	return val;
}


template <class T>
void RingBuffer<T>::reset()
{
	std::lock_guard<std::mutex> lock(mutex_);
	head_ = tail_;
	full_ = false;
}


template <class T>
bool RingBuffer<T>::empty()
{
	//if head and tail are equal, we are empty
	return (!full_ && (head_ == tail_));
}

template <class T>
bool RingBuffer<T>::full()
{
	//If tail is ahead the head by 1, we are full
	return full_;
}


template <class T>
size_t RingBuffer<T>::capacity()
{
	return max_size_;
}

template <class T>
size_t RingBuffer<T>::size()
{
	size_t size = max_size_;

	if(!full_)
	{
		if(head_ >= tail_)
		{
			size = head_ - tail_;
		}
		else
		{
			size = max_size_ + head_ - tail_;
		}
	}

	return size;
}

#endif