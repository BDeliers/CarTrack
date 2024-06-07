#pragma once

#include <stdint.h>
#include <array>

class CircularBuffer
{
public:
    bool Push(char& c);
    bool Pop(char& c);
    bool IsFull(void);
    bool IsEmpty(void);

private:
    const uint16_t size{256};
    std::array<char, 256> buffer;
    uint16_t head{0};
    uint16_t tail{0};
    uint16_t usage{0};
};
