extern "C"
{
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}
#include <string>

#include "ecu_mode.hh"
#include "frame/builder.hh"
#include "frame/frame.hh"
#include "frame/parser.hh"
#include "shared_memory/shared_memory.hh"
#include "task_failsafe.hh"

// Handle de la tâche — nécessaire pour notifier depuis l'ISR
static TaskHandle_t failsafe_task_handle = nullptr;

// Notification values pour distinguer les deux déclencheurs
static constexpr unsigned int NOTIFY_TIMEOUT = (1 << 0);
static constexpr unsigned int NOTIFY_GPIO = (1 << 1);

void IRAM_ATTR failsafe_gpio_isr_handler(__attribute__((unused))void* arg)
{
    BaseType_t higher_priority_woken = pdFALSE;
    xTaskNotifyFromISR(failsafe_task_handle, NOTIFY_GPIO, eSetBits,
                       &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

static void trigger_failsafe(const char* reason)
{
    SharedMemory::set_mode(ECUMode::OFF);

    const Frame frame = builder::alarm(std::string(reason));
    const auto& frame_bytes = frame.get_full_frame();
    uart_write_bytes(UART_NUM_0, frame_bytes.data(), frame_bytes.size());
}

void task_failsafe(__attribute__((unused))void* params)
{
    failsafe_task_handle = xTaskGetCurrentTaskHandle();

    while (true)
    {
        uint32_t notification = 0;
        xTaskNotifyWait(0, UINT32_MAX, &notification, pdMS_TO_TICKS(100));

        if (notification & NOTIFY_GPIO)
        {
            trigger_failsafe("FAILSAFE: GPIO emergency stop");
            continue;
        }

        const uint32_t now = xTaskGetTickCount();
        const uint32_t last_rx = SharedMemory::get_last_rx_tick();
        const uint32_t elapsed = (now - last_rx) * portTICK_PERIOD_MS;

        if (elapsed > 2000 && SharedMemory::get_mode() != ECUMode::OFF)
        {
            trigger_failsafe("FAILSAFE: no valid frame for 2s");
        }
    }
}