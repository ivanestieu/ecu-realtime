#include "task_telemetry.hh"
#include "shared_state.hh"
#include "frame/builder.hh"
#include "driver/uart.hh"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
        uint8_t buf[32];
        int len = builder_stats(buf, stats);
        uart_write_bytes(UART_NUM_0, buf, len);
    }
}