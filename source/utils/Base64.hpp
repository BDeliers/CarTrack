#pragma once

#include <stdint.h>
#include <stddef.h>

/// @note Highly inspired by MBedTLS base64.c
bool EncodeBase64(uint8_t* dest, size_t dest_size, const uint8_t* src, size_t src_size, size_t* bytes_written);
bool DecodeBase64(uint8_t* dest, size_t dest_size, const uint8_t* src, size_t src_size, size_t* bytes_written);
