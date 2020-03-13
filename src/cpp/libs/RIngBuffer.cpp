#include "libs/RingBuffer.h"

template class RingBuffer<uint8_t>;
template class RingBuffer<uint16_t>;

template <class T> RingBuffer<T>::RingBuffer(uint16_t _bufferSize)
{
    bufferSize = _bufferSize;
    readHead = 0;
    writeHead = 0;
}

template <class T> void RingBuffer<T>::advanceWriteHead()
{
    writeHead += 1;
    if(writeHead >= bufferSize)
        writeHead = 0;
}

template <class T> void RingBuffer<T>::advanceReadHead()
{
    readHead += 1;
    if(readHead >= bufferSize)
        readHead = 0;
}

template <class T> T RingBuffer<T>::read()
{
    return buffer[readHead];
}

template <class T> void RingBuffer<T>::write(T data)
{
    buffer[writeHead] = data;
}