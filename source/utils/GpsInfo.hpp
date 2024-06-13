#include <ctime>
#include <cstring>
#include <stdint.h>

/// @brief Class to handle GPS data
class GpsInfo
{
public:
    void SetFix(bool has_fix)                               { this->has_fix             = has_fix;              };
    void SetTimestamp(time_t timestamp)                     { this->timestamp           = timestamp;            };
    void SetLatitude(int32_t latitude)                      { this->latitude            = latitude;             };
    void SetLongitude(int32_t longitude)                    { this->longitude           = longitude;            };
    void SetAltitude(uint32_t altitude)                     { this->altitude            = altitude;             };
    void SetSpeed(uint16_t speed)                           { this->speed               = speed;                };
    void SetCourse(uint16_t course)                         { this->course              = course;               };
    void SetHdop(uint8_t hdop)                              { this->hdop                = hdop;                 };
    void SetPdop(uint8_t pdop)                              { this->pdop                = pdop;                 };
    void SetVdop(uint8_t vdop)                              { this->vdop                = vdop;                 };
    void SetSatellitesInView(uint8_t satellites_in_view)    { this->satellites_in_view  = satellites_in_view;   };
    void SetSatellitesInUse(uint16_t satellites_in_use)     { this->satellites_in_use   = satellites_in_use;    };
    void SetCn0(uint8_t cn0)                                { this->cn0                 = cn0;                  };

    /// @brief  Getter
    /// @return True if a valid GPS fix 
    bool        GetFix(void)                    const { return this->has_fix;               };

    /// @brief  Getter
    /// @return time_t of current UTC timestamp
    time_t      GetTimestamp(void)              const { return this->timestamp;             };

    /// @brief  Getter
    /// @return Latitude in decimal degrees *1 000 000
    int32_t     GetLatitude(void)               const { return this->latitude;              };

    /// @brief  Getter
    /// @return Longitude in decimal degrees *1 000 000
    int32_t     GetLongitude(void)              const { return this->longitude;             };

    /// @brief  Getter
    /// @return Altitude compared to sea level, in milimeters
    uint32_t    GetAltitude(void)               const { return this->altitude;              };

    /// @brief  Getter
    /// @return Speed over ground, in decameters per hour
    uint16_t    GetSpeed(void)                  const { return this->speed;                 };

    /// @brief  Getter
    /// @return Course over earth, in degrees *100
    uint16_t    GetCourse(void)                 const { return this->course;                };

    /// @brief  Getter
    /// @return HDOP *10
    uint8_t     GetHdop(void)                   const { return this->hdop;                  };

    /// @brief  Getter
    /// @return PDOP *10
    uint8_t     GetPdop(void)                   const { return this->pdop;                  };

    /// @brief  Getter
    /// @return VDOP *10
    uint8_t     GetVdop(void)                   const { return this->vdop;                  };

    /// @brief  Getter
    /// @return Number of satellites in view
    uint8_t     GetSatellitesInView(void)       const { return this->satellites_in_view;    };

    /// @brief  Getter
    /// @return Sum of GPS and GLONASS satellites in use
    uint16_t    GetSatellitesInUse(void)        const { return this->satellites_in_use;     };

    /// @brief  Getter
    /// @return Carrier-to-noise density, in decibel Hertz
    uint8_t     GetCn0(void)                    const { return this->cn0;                   };

    /// @brief  Getter
    /// @param  tm struct to fill with current UTC time
    void        GetUtcDateTime(struct tm* ptr)  const { memcpy(ptr, gmtime(&this->timestamp), sizeof(struct tm)); };

private:
    bool        has_fix{false};
    time_t      timestamp{0};
    int32_t     latitude{0};
    int32_t     longitude{0};
    uint32_t    altitude{0};
    uint16_t    speed{0};
    uint16_t    course{0};
    uint8_t     hdop{0};
    uint8_t     pdop{0};
    uint8_t     vdop{0};
    uint8_t     satellites_in_view{0};
    uint16_t    satellites_in_use{0};
    uint8_t     cn0{0};
};
