#pragma once

#include "utils/GpsInfo.hpp"

size_t PayloadFormatter_MakePayload(const GpsInfo& gps, uint8_t power, int16_t temperature, uint8_t* buffer, size_t buffer_size);
