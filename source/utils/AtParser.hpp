#pragma once

#include <stdint.h>
#include <stdarg.h>

enum CmdType  {
    TEST,               // AT+<cmd>=?
    READ,               // AT+<cmd>?
    WRITE,              // AT+<cmd>=<p1>...<pn>
    EXEC,               // AT+<cmd>
    CMD_TYPE_COUNT,
};

uint32_t AtParser_MakeCommand(const CmdType type, char* buffer, uint32_t buffer_size, const char* cmd, uint8_t params_count, ...);
uint32_t AtParser_MakeCommand(const CmdType type, char* buffer, uint32_t buffer_size, const char* cmd, uint8_t params_count, va_list params);
