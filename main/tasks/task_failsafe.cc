extern "C"
{
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}
#include <string>

#include "frame/builder.hh"
#include "frame/frame.hh"
#include "shared_memory/shared_memory.hh"
#include "task_failsafe.hh"
#include "task_uart_emitter.hh"
#include "utils/ecu_mode.hh"
#include "utils/safe_esp_log.hh"

static TaskHandle_t failsafe_task_handle = nullptr;

static constexpr unsigned int NOTIFY_TIMEOUT = (1 << 0);
static constexpr unsigned int NOTIFY_GPIO = (1 << 1);
static constexpr unsigned int FRAME_TIMEOUT_MS = 2000;

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

    SharedMemory::set_output(0.0f);

    SharedMemory::set_mode(ECUMode::OFF);

    const Frame frame = builder::alarm(std::string(reason));
    uart_emit_failsafe_frame(frame);
    const auto& frame_bytes = frame.get_full_frame();
    ESP_LOGI(__FILE_NAME__, "Failsafe alarm frame sent (%d bytes)",
             frame_bytes.size());
}

[[noreturn]] void task_failsafe(__attribute__((unused)) void* params)
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
        const uint32_t last_rx = SharedMemory::get_last_rx_tick();
        const uint32_t elapsed = (now - last_rx) * portTICK_PERIOD_MS;

        if (elapsed > FRAME_TIMEOUT_MS
            && SharedMemory::get_mode() != ECUMode::OFF)
        {
            trigger_failsafe("FAILSAFE: no valid frame for 2s");
        }
    }
}