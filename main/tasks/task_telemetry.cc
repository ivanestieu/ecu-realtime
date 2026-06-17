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
#include "task_uart_emitter.hh"

[[noreturn]] void task_telemetry(__attribute__((unused)) void* params)
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

        ESP_LOGI(
            __FILE_NAME__,
            "task_telemetry: valid=%u, corrupted=%u, dropped=%u, output=%u",
            stats[0], stats[1], stats[2], stats[3]);

        uart_emit_frame(builder::stats(stats));
    }
}