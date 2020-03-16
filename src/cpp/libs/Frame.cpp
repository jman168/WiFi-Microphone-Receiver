#include "libs/Frame.h"

template class Frame<uint8_t>;
template class Frame<uint16_t>;

template <class T> void Frame<T>::setSize(uint8_t _size)
{
    size = _size;
    buffer = new T[_size];
}
