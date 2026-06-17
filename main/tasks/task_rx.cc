#include "frame/builder.hh"

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
#include "utils/safe_esp_log.hh"

static void handle_frame(const Frame* frame)
{
    auto payload = frame->get_payload();
    switch (frame->get_type())
    {
    case Frame::MsgType::SPEED: {
        if (payload.size() >= sizeof(float))
        {
            float speed = reinterpret_cast<const float*>(payload.data())[0];
            SharedMemory::set_speed(speed);
            ESP_LOGI(__FILE_NAME__, "task_rx: SPEED frame received: %.2f km/h",
                     speed);
        }
        break;
    }
    case Frame::MsgType::SETPOINT: {
        if (payload.size() >= sizeof(float))
        {
            float setpoint = reinterpret_cast<const float*>(payload.data())[0];
            SharedMemory::set_setpoint(setpoint);
            ESP_LOGI(__FILE_NAME__,
                     "task_rx: SETPOINT frame received: %.2f km/h", setpoint);
        }
        break;
    }
    case Frame::MsgType::MODE_SET: {
        if (payload.size() >= sizeof(uint8_t))
        {
            const auto new_mode = payload[0];
            SharedMemory::set_mode(new_mode);
            ESP_LOGI(__FILE_NAME__, "task_rx: MODE_SET frame received: mode=%d",
                     new_mode);
        }
        break;
    }
    default:
        SharedMemory::inc_stats_dropped();
        ESP_LOGI(__FILE_NAME__, "task_rx: Unknown frame type received");
        break;
    }
}

[[noreturn]] void task_rx(__attribute__((unused)) void* params)
{
    ESP_LOGI(__FILE_NAME__, "task_rx: Starting frame reception");
    Parser::Parser parser{};

    while (true)
    {
        uint8_t byte;
        const int received =
            uart_read_bytes(UART_NUM_0, &byte, 1, portMAX_DELAY);
        if (received <= 0)
        {
            SharedMemory::inc_stats_corrupted();
            continue;
        }

        const auto frame = parser.push_byte(byte);
        if (!frame.has_value())
        {
            continue;
        }

        SharedMemory::set_last_rx_tick(xTaskGetTickCount());
        SharedMemory::inc_stats_valid();

        handle_frame(&frame.value());
    }
}