extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
}

#include "utils/ecu_mode.hh"
#include "utils/mutex_guard.hh"
#include "utils/safe_esp_log.hh"
#include "shared_memory.hh"

namespace
{
    SemaphoreHandle_t g_mutex;

    float s_speed = 0.0f;
    float s_setpoint = 0.0f;
    std::uint8_t s_mode = 0x00;
    float s_output = 0.0f;
    std::uint32_t s_last_rx_tick = 0;
    std::uint32_t s_stats_valid = 0;
    std::uint32_t s_stats_corrupted = 0;
    std::uint32_t s_stats_dropped = 0;
    std::uint32_t s_stats_output = 0;
} // namespace

namespace SharedMemory
{
    void init()
    {
        g_mutex = xSemaphoreCreateMutex();
    }

    void set_speed(const float v)
    {
        MutexGuard lock(g_mutex);
        s_speed = v;
    }

    float get_speed()
    {
        MutexGuard lock(g_mutex);
        return s_speed;
    }

    void set_setpoint(const float v)
    {
        MutexGuard lock(g_mutex);
        s_setpoint = v;
    }

    float get_setpoint()
    {
        MutexGuard lock(g_mutex);
        return s_setpoint;
    }

    void set_mode(const std::uint8_t v)
    {
        MutexGuard lock(g_mutex);
        const auto old_mode = s_mode;
        s_mode = v;
        if (old_mode != v)
        {
            ESP_LOGI(__FILE_NAME__, "SharedMemory: Mode changed from %s to %s",
                     mode_to_string(old_mode), mode_to_string(v));
        }
    }

    std::uint8_t get_mode()
    {
        MutexGuard lock(g_mutex);
        return s_mode;
    }

    void set_output(const float v)
    {
        MutexGuard lock(g_mutex);
        s_output = v;
    }

    float get_output()
    {
        MutexGuard lock(g_mutex);
        return s_output;
    }

    void set_last_rx_tick(const std::uint32_t t)
    {
        MutexGuard lock(g_mutex);
        s_last_rx_tick = t;
    }

    std::uint32_t get_last_rx_tick()
    {
        MutexGuard lock(g_mutex);
        return s_last_rx_tick;
    }

    void inc_stats_valid()
    {
        MutexGuard lock(g_mutex);
        s_stats_valid++;
    }

    void inc_stats_corrupted()
    {
        MutexGuard lock(g_mutex);
        s_stats_corrupted++;
    }

    void inc_stats_dropped()
    {
        MutexGuard lock(g_mutex);
        s_stats_dropped++;
    }

    void inc_stats_output()
    {
        MutexGuard lock(g_mutex);
        s_stats_output++;
    }

    std::uint32_t get_stats_valid()
    {
        MutexGuard lock(g_mutex);
        return s_stats_valid;
    }

    std::uint32_t get_stats_corrupted()
    {
        MutexGuard lock(g_mutex);
        return s_stats_corrupted;
    }

    std::uint32_t get_stats_dropped()
    {
        MutexGuard lock(g_mutex);
        return s_stats_dropped;
    }

    std::uint32_t get_stats_output()
    {
        MutexGuard lock(g_mutex);
        return s_stats_output;
    }
} // namespace SharedMemory