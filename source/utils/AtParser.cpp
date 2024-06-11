#include "AtParser.hpp"

#include <cstring>

constexpr const char AT[]                         = "AT+";
constexpr const char CR[]                         = "\r";
constexpr const char SEPARATOR[]                  = ",";
constexpr const char* DELIMITERS[CMD_TYPE_COUNT]  = { "=?", "?", "=", ""};

uint32_t AtParser_MakeCommand(const CmdType type, char* buffer, uint32_t buffer_size, const char* cmd, uint8_t params_count, ...)
{
    va_list params;
    va_start(params, params_count);
    uint32_t ret = AtParser_MakeCommand(type, buffer, buffer_size, cmd, params_count, params);
    va_end(params);

    return ret;
}

uint32_t AtParser_MakeCommand(const CmdType type, char* buffer, uint32_t buffer_size, const char* cmd, uint8_t params_count, va_list params)
{
    char* ptr = buffer;

    if (buffer_size < (sizeof(AT) + sizeof(CR) + strlen(cmd) + strlen(DELIMITERS[type])))
    {
        return 0;
    }

    memset(buffer, 0, buffer_size);

    strcat(ptr, AT);
    ptr += strlen(AT);
    
    strcat(ptr, cmd);
    ptr+= strlen(cmd);

    strcat(ptr, DELIMITERS[type]);
    ptr+= strlen(DELIMITERS[type]);

    bool buffer_too_small = false;

    if (params_count > 0)
    {
        char* param_ptr;

        while (params_count--)
        {
            param_ptr = va_arg(params, char*);

            if ((strlen(buffer) + strlen(param_ptr) + 1) < buffer_size)
            {
                strcat(buffer, param_ptr);
                ptr += strlen(param_ptr);

                if (params_count > 0)
                {
                    strcat(ptr, SEPARATOR);
                    ptr += strlen(SEPARATOR);
                }
            }
            else
            {
                buffer_too_small = true;
                ptr = buffer;
                break;
            }
        }
    }

    if (!buffer_too_small)
    {
        strcat(ptr, CR);
        ptr += strlen(CR);
    }

    return (ptr - buffer);
}
