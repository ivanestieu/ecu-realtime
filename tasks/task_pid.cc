#include "task_pid.hh"
#include "pid.hh"
#include "shared_state.hh"
#include "frame/builder.hh"
#include "frame/parser.hh"
#include "driver/uart.hh"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
 
void task_pid(void *params) {
    pid_t pid = pid_init();
    TickType_t last_wake = xTaskGetTickCount();
 
    while (1) {
        // 1. attendre jusqu'au prochain cycle (période stricte 100ms)
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));
 
        // 2. vérifier le mode — on calcule seulement en AUTO
        if (get_mode() != MODE_AUTO) continue;
 
        // 3. lire l'état depuis shared_state
        float speed    = get_speed();
        float setpoint = get_setpoint();
 
        // 4. calculer la commande
        float output = pid_compute(&pid, setpoint, speed, 0.1f);
 
        // 5. stocker le résultat
        set_output(output);
        inc_stats_output();
 
        // 6. construire et envoyer la trame OUTPUT
        uint8_t buf[32];
        int len = builder_output(buf, output);
        uart_write_bytes(UART_NUM_0, buf, len);
    }
}