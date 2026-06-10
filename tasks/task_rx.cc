#include "task_rx.hh"
#include "frame/parser.hh"
#include "frame/frame.hh"
#include "shared_state.hh"
#include "driver/uart.hh"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <memory>

 
void task_rx(void *params) {
    (void)params;
    Parser::Parser parser;
 
    while (1) {
        // 1. lire un octet depuis l'UART (bloquant)
        uint8_t byte;
        int received = uart_read_bytes(UART_NUM_0, &byte, 1, portMAX_DELAY);
        if (received <= 0) continue;
 
        // 2. passer l'octet au parser
        auto frame = parser.push_byte(byte);
        if (!frame) continue;
 
        // 3. trame complète et valide
        set_last_rx_tick(xTaskGetTickCount());
        inc_stats_valid();
 
        // 4. dispatcher selon le type
        auto payload = frame->get_payload();
        switch (frame->get_type()) {
            case Frame::MsgType::SPEED: {
                if (payload.size() >= sizeof(float)) {
                    float speed;
                    memcpy(&speed, payload.data(), sizeof(float));
                    set_speed(speed);
                }
                break;
            }
            case Frame::MsgType::SETPOINT: {
                if (payload.size() >= sizeof(float)) {
                    float setpoint;
                    memcpy(&setpoint, payload.data(), sizeof(float));
                    set_setpoint(setpoint);
                }
                break;
            }
            case Frame::MsgType::MODE_SET: {
                if (payload.size() >= sizeof(uint8_t)) {
                    set_mode(payload[0]);
                }
                break;
            }
            default:
                // type inconnu → on ignore silencieusement
                break;
        }
    }
}