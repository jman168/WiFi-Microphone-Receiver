#include "libs/RingBuffer.h"

#define FRAME_SIZE 120
#define BUFFER_SIZE 15

int main(int argc, char const *argv[])
{
    RingBuffer<uint8_t> buffer(FRAME_SIZE, BUFFER_SIZE);
    return 0;
}
