#include "task_telemetry.hh"
#include "shared_state.hh"
#include "frame/builder.hh"
#include "frame/frame.hh"
#include "driver/uart.hh"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <vector>

void task_telemetry(void *params) {
    (void)params;
    TickType_t last_wake = xTaskGetTickCount();
 
    while (1) {
        // attendre 1 seconde
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));
 
        // lire les compteurs depuis shared_state
        uint32_t stats[4] = {
            get_stats_valid(),
            get_stats_corrupted(),
            get_stats_dropped(),
            get_stats_output(),
        };
 
        // construire et envoyer la trame STATS
        std::vector<std::uint32_t> stats_vec(stats, stats + 4);
        Frame frame = builder::stats(stats_vec);
        const auto& frame_bytes = frame.get_full_frame();
        uart_write_bytes(UART_NUM_0, (uint8_t*)frame_bytes.data(), frame_bytes.size());
    }
}