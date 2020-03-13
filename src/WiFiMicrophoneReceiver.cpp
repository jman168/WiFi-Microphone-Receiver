#include "libs/RingBuffer.h"

#define BUFFER_SIZE 15

int main(int argc, char const *argv[])
{
    RingBuffer<uint8_t> buffer(BUFFER_SIZE);
    return 0;
}
