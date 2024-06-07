#include "CircularBuffer.hpp"

bool CircularBuffer::Push(char& c)
{
    if (this->IsFull())
    {
        return false;
    }

    buffer[tail++] = c;

    tail %= size;
    usage++;

    return true;
}

bool CircularBuffer::Pop(char& c)
{
    if (this->IsEmpty())
    {
        return false;
    }

    c = buffer[head++];

    head %= size;
    usage--;

    return true;
}

bool CircularBuffer::IsFull(void)
{
    return (usage == size);
}

bool CircularBuffer::IsEmpty(void)
{
    return (usage == 0);
}
