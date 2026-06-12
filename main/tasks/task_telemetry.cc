extern "C"
{
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

#include "frame/builder.hh"
#include "frame/frame.hh"
#include "shared_memory/shared_memory.hh"
#include "task_telemetry.hh"

void task_telemetry(__attribute__((unused))void* params)
{
    TickType_t last_wake = xTaskGetTickCount();

    while (true)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));

        std::array<uint32_t, builder::STATS_SIZE> stats{
            SharedMemory::get_stats_valid(),
            SharedMemory::get_stats_corrupted(),
            SharedMemory::get_stats_dropped(),
            SharedMemory::get_stats_output(),
        };

        Frame frame = builder::stats(stats);
        const auto& frame_bytes = frame.get_full_frame();
        uart_write_bytes(UART_NUM_0, frame_bytes.data(), frame_bytes.size());
    }
}