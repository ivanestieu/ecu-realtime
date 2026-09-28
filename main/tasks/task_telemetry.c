#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "frame/builder.h"
#include "frame/frame.h"
#include "shared_memory/shared_memory.h"
#include "task_telemetry.h"
#include "task_uart_emitter.h"

_Noreturn void task_telemetry(__attribute__((unused)) void* params)
{
    TickType_t last_wake = xTaskGetTickCount();

    while (true)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));

        const uint32_t stats_data[STATS_SIZE] = {
            get_stats_valid(),
            get_stats_corrupted(),
            get_stats_dropped(),
            get_stats_output(),
        };

        ESP_LOGI(
            __FILE_NAME__,
            "task_telemetry: valid=%u, corrupted=%u, dropped=%u, output=%u",
            stats_data[0], stats_data[1], stats_data[2], stats_data[3]);

        Frame frame = stats(stats_data);
        uart_emit_frame(&frame);
    }
}