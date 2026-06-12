extern "C"
{
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}
#include <cstring>

#include "frame/frame.hh"
#include "frame/parser.hh"
#include "shared_memory/shared_memory.hh"
#include "task_rx.hh"

void task_rx(__attribute__((unused))void* params)
{
    Parser::Parser parser{};

    while (true)
    {
        uint8_t byte;
        const int received =
            uart_read_bytes(UART_NUM_0, &byte, 1, portMAX_DELAY);
        if (received <= 0)
        {
            continue;
        }

        const auto frame = parser.push_byte(byte);
        if (!frame.has_value())
        {
            continue;
        }

        SharedMemory::set_last_rx_tick(xTaskGetTickCount());
        SharedMemory::inc_stats_valid();

        auto payload = frame->get_payload();
        switch (frame->get_type())
        {
        case Frame::MsgType::SPEED: {
            if (payload.size() >= sizeof(float))
            {
                float speed;
                memcpy(&speed, payload.data(), sizeof(float));
                SharedMemory::set_speed(speed);
            }
            break;
        }
        case Frame::MsgType::SETPOINT: {
            if (payload.size() >= sizeof(float))
            {
                float setpoint;
                memcpy(&setpoint, payload.data(), sizeof(float));
                SharedMemory::set_setpoint(setpoint);
            }
            break;
        }
        case Frame::MsgType::MODE_SET: {
            if (payload.size() >= sizeof(uint8_t))
            {
                SharedMemory::set_mode(payload[0]);
            }
            break;
        }
        default:
            break;
        }
    }
}