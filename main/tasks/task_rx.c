#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "frame/frame.h"
#include "frame/parser.h"
#include "shared_memory/shared_memory.h"
#include "task_rx.h"

#include "ecu_mode.h"
#include "frame/builder.h"
#include "utils/safe_esp_log.h"

static void handle_frame(const Frame* frame)
{
    const Span payload = get_payload(frame);
    switch (get_type(frame))
    {
    case SPEED: {
        if (payload.size >= sizeof(float))
        {
            const float speed = bytes_to_float(payload.data);
            set_speed(speed);
            ESP_LOGI(__FILE_NAME__, "task_rx: SPEED frame received: %.2f km/h",
                     speed);
        }
        break;
    }
    case SETPOINT: {
        if (payload.size >= sizeof(float))
        {
            const float setpoint = bytes_to_float(payload.data);
            set_setpoint(setpoint);
            ESP_LOGI(__FILE_NAME__,
                     "task_rx: SETPOINT frame received: %.2f km/h", setpoint);
        }
        break;
    }
    case MODE_SET: {
        if (payload.size >= sizeof(uint8_t))
        {
            const uint8_t new_mode = payload.data[0];
            set_mode(new_mode);
            ESP_LOGI(__FILE_NAME__, "task_rx: MODE_SET frame received: mode=%s",
                     mode_to_string(new_mode));
        }
        break;
    }
    default:
        inc_stats_dropped();
        ESP_LOGI(__FILE_NAME__, "task_rx: Unknown frame type received");
        break;
    }
}

_Noreturn void task_rx(__attribute__((unused)) void* params)
{
    ESP_LOGI(__FILE_NAME__, "task_rx: Starting frame reception");
    Parser parser = init_parser();

    while (true)
    {
        uint8_t byte;
        const int received =
            uart_read_bytes(UART_NUM_0, &byte, 1, portMAX_DELAY);
        if (received <= 0)
        {
            inc_stats_corrupted();
            continue;
        }

        const Frame *frame_ptr = push_byte(&parser, byte);
        if (!frame_ptr)
        {
            continue;
        }
        const Frame frame = *frame_ptr;


        set_last_rx_tick(xTaskGetTickCount());
        inc_stats_valid();

        handle_frame(&frame);
    }
}