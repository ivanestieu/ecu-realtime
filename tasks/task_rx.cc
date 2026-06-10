#include "task_rx.hh"
#include "frame/parser.hh"
#include "shared_state.hh"
#include "driver/uart.hh"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
 
 
void task_rx(void *params) {
    parser_ctx_t ctx;
    frame_t frame;
    parser_init(&ctx);
 
    while (1) {
        // 1. lire un octet depuis l'UART (bloquant)
        uint8_t byte;
        int received = uart_read_bytes(UART_NUM_0, &byte, 1, portMAX_DELAY);
        if (received <= 0) continue;
 
        // 2. passer l'octet au parser
        bool complete = parser_push_byte(byte, &ctx, &frame);
        if (!complete) continue;
 
        // 3. trame complète et valide
        set_last_rx_tick(xTaskGetTickCount());
        inc_stats_valid();
 
        // 4. dispatcher selon le type
        switch (frame.type) {
            case MSG_SPEED: {
                float speed;
                memcpy(&speed, frame.payload, sizeof(float));
                set_speed(speed);
                break;
            }
            case MSG_SETPOINT: {
                float setpoint;
                memcpy(&setpoint, frame.payload, sizeof(float));
                set_setpoint(setpoint);
                break;
            }
            case MSG_MODE_SET: {
                set_mode(frame.payload[0]);
                break;
            }
            default:
                // type inconnu → on ignore silencieusement
                break;
        }
    }
}