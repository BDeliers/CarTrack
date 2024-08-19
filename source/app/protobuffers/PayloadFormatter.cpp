#include "PayloadFormatter.hpp"

#include <pb_encode.h>
#include "CarTrack.pb.h"

size_t PayloadFormatter_MakePayload(const GpsInfo& gps, uint8_t power, int16_t temperature, uint8_t* buffer, size_t buffer_size)
{
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    CarTrack_Point msg = CarTrack_Point_init_zero;
    msg.latitude    = gps.GetLatitude();
    msg.longitude   = gps.GetLongitude();
    msg.altitude    = gps.GetAltitude();
    msg.speed       = static_cast<uint32_t>(0x0000FFFF & gps.GetSpeed());
    msg.has_speed   = (msg.speed > 0);
    msg.timestamp   = gps.GetTimestamp();
    msg.power       = static_cast<uint32_t>(0x000000FF & power);
    msg.has_power   = (msg.power != 0xFF);
    msg.temperature |= (temperature) & 0x0000FFFF;

    if (pb_encode(&stream, CarTrack_Point_fields, &msg))
    {
        return stream.bytes_written;
    }

    return 0;
}
