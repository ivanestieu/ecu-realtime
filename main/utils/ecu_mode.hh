#pragma once

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
    case ECUMode::OFF:
        return "OFF";
    case ECUMode::MANUAL:
        return "MANUAL";
    case ECUMode::AUTO:
        return "AUTO";
    default:
        return "UNKNOWN";
    }
}
