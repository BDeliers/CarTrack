#include "Base64.hpp"

/// @note Highly inspired by MBedTLS base64.c
bool EncodeBase64(uint8_t* dest, size_t dest_size, const uint8_t* src, size_t src_size, size_t* bytes_written)
{
    static const unsigned char base64_enc_map[64] =
    {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
        'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '+', '/'
    };

    size_t i, n;
    int C1, C2, C3;
    unsigned char *p;

    if(src_size == 0)
    {
        *bytes_written = 0;
        return true;;
    }

    n = src_size / 3 + (src_size % 3 != 0);

    if(n > (((size_t) -1) - 1) / 4)
    {
        *bytes_written = ((size_t) -1);
        return false;
    }

    n *= 4;

    if(dest_size < n + 1)
    {
        *bytes_written = n + 1;
        return false;
    }

    n = (src_size / 3) * 3;

    for(i = 0, p = dest; i < n; i += 3)
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if(i < src_size)
    {
        C1 = *src++;
        C2 = ((i + 1) < src_size) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if((i + 1) < src_size)
             *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else *p++ = '=';

        *p++ = '=';
    }

    *bytes_written = p - dest;
    *p = 0;

    return true;;
}

bool DecodeBase64(uint8_t* dest, size_t dest_size, const uint8_t* src, size_t src_size, size_t* bytes_written)
{
    static const unsigned char base64_dec_map[128] =
    {
        127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
        127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
        127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
        127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
        127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
        54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
        127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
        5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
        15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
        25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
        29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
        39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
        49,  50,  51, 127, 127, 127, 127, 127
    };

    size_t i, n;
    uint32_t j, x;
    unsigned char *p;

    /* First pass: check for validity and get output length */
    for(i = n = j = 0; i < src_size; i++)
    {
        /* Skip spaces before checking for EOL */
        x = 0;
        while(i < src_size && src[i] == ' ')
        {
            ++i;
            ++x;
        }

        /* Spaces at end of buffer are OK */
        if(i == src_size)
            break;

        if((src_size - i) >= 2 &&
            src[i] == '\r' && src[i + 1] == '\n')
            continue;

        if(src[i] == '\n')
            continue;

        /* Space inside a line is an error */
        if(x != 0)
            return false;

        if(src[i] == '=' && ++j > 2)
            return false;

        if(src[i] > 127 || base64_dec_map[src[i]] == 127)
            return false;

        if(base64_dec_map[src[i]] < 64 && j != 0)
            return false;

        n++;
    }

    if(n == 0)
    {
        *bytes_written = 0;
        return true;;
    }

    n = ((n * 6) + 7) >> 3;
    n -= j;

    if(dest == NULL || dest_size < n)
    {
        *bytes_written = n;
        return false;
    }

   for(j = 3, n = x = 0, p = dest; i > 0; i--, src++)
   {
        if(*src == '\r' || *src == '\n' || *src == ' ')
            continue;

        j -= (base64_dec_map[*src] == 64);
        x  = (x << 6) | (base64_dec_map[*src] & 0x3F);

        if(++n == 4)
        {
            n = 0;
            if(j > 0) *p++ = (unsigned char)(x >> 16);
            if(j > 1) *p++ = (unsigned char)(x >>  8);
            if(j > 2) *p++ = (unsigned char)(x);
        }
    }

    *bytes_written = p - dest;

    return true;
}
