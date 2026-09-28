#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "task_failsafe.h"

#include "frame/builder.h"
#include "frame/frame.h"
#include "shared_memory/shared_memory.h"
#include "task_uart_emitter.h"
#include "utils/ecu_mode.h"
#include "utils/safe_esp_log.h"

static TaskHandle_t failsafe_task_handle = NULL;

#define NOTIFY_TIMEOUT (1 << 0)
#define NOTIFY_GPIO (1 << 1)
#define FRAME_TIMEOUT_MS 2000

void IRAM_ATTR failsafe_gpio_isr_handler(__attribute__((unused)) void* arg)
{
    BaseType_t higher_priority_woken = pdFALSE;
    xTaskNotifyFromISR(failsafe_task_handle, NOTIFY_GPIO, eSetBits,
                       &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

static void trigger_failsafe(const char* reason)
{
    ESP_LOGI(__FILE_NAME__, "FAILSAFE TRIGGERED: %s", reason);

    set_output(0.0f);

    set_mode(OFF);

    const Frame frame = alarm(reason);
    uart_emit_failsafe_frame(&frame);
    const Span frame_bytes = get_full_frame(&frame);
    ESP_LOGI(__FILE_NAME__, "Failsafe alarm frame sent (%d bytes)",
             frame_bytes.size);
}

_Noreturn void task_failsafe(__attribute__((unused)) void* params)
{
    ESP_LOGI(__FILE_NAME__, "task_failsafe: Starting failsafe monitor");
    failsafe_task_handle = xTaskGetCurrentTaskHandle();

    while (true)
    {
        uint32_t notification = 0;
        const BaseType_t notified =
            xTaskNotifyWait(0, UINT32_MAX, &notification, pdMS_TO_TICKS(100));
        if (notified == pdFALSE)
        {
            notification |= NOTIFY_TIMEOUT;
        }

        if (notification & NOTIFY_GPIO)
        {
            trigger_failsafe("FAILSAFE: GPIO emergency stop");
            continue;
        }

        const uint32_t now = xTaskGetTickCount();
        const uint32_t last_rx = get_last_rx_tick();
        const uint32_t elapsed = (now - last_rx) * portTICK_PERIOD_MS;

        if (elapsed > FRAME_TIMEOUT_MS
            && get_mode() != OFF)
        {
            trigger_failsafe("FAILSAFE: no valid frame for 2s");
        }
    }
}