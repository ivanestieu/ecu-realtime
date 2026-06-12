extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
}

#include "shared_memory.hh"

namespace
{
    SemaphoreHandle_t g_mutex;

    class MutexGuard
    {
    public:
        explicit MutexGuard(const SemaphoreHandle_t handle) : handle_{handle}
        {
            xSemaphoreTake(handle_, portMAX_DELAY);
        }

        ~MutexGuard()
        {
            xSemaphoreGive(handle_);
        }

        MutexGuard(const MutexGuard&) = delete;
        MutexGuard& operator=(const MutexGuard&) = delete;
        MutexGuard(MutexGuard&&) = delete;
        MutexGuard& operator=(MutexGuard&&) = delete;

    private:
        SemaphoreHandle_t handle_;
    };

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
        s_mode = v;
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
} // namespaSharedMemory