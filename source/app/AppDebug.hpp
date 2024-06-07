#pragma once

#include <array>

#include "fsl_lpuart.h"

#include "log.h"

class AppDebug
{

public:
	static AppDebug& GetInstance(void)
	{
		static AppDebug instance;
		return instance;
	}

    void Init(void);

    bool Push(char c);
    void Flush(void);

    void UartIrqHandler(void);

private:
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

    CircularBuffer fifo;

    LPUART_Type* uart_ptr{NULL};
    volatile bool tx_ongoing{false};

};
