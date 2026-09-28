#ifndef ECU_MODE_H
#define ECU_MODE_H

#include <stdint.h>

enum ECUMode
{
    OFF = 0x00,
    MANUAL = 0x01,
    AUTO = 0x02,
};

inline const char* mode_to_string(const uint8_t mode)
{
    switch (mode)
    {
    case OFF:
        return "OFF";
    case MANUAL:
        return "MANUAL";
    case AUTO:
        return "AUTO";
    default:
        return "UNKNOWN";
    }
}

#endif // ECU_MODE_H
