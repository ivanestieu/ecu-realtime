#pragma once
#include <cstdint>

namespace SharedMemory
{
    void init();

    void set_speed(float v);
    float get_speed();

    void set_setpoint(float v);
    float get_setpoint();

    void set_mode(std::uint8_t v);
    std::uint8_t get_mode();

    void set_output(float v);
    float get_output();

    void set_last_rx_tick(std::uint32_t t);
    std::uint32_t get_last_rx_tick();

    void inc_stats_valid();
    void inc_stats_corrupted();
    void inc_stats_dropped();
    void inc_stats_output();
    std::uint32_t get_stats_valid();
    std::uint32_t get_stats_corrupted();
    std::uint32_t get_stats_dropped();
    std::uint32_t get_stats_output();
} // namespace SharedMemory