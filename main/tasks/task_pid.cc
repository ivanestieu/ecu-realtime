#include "task_uart_emitter.hh"

extern "C"
{
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

#include "frame/builder.hh"
#include "frame/frame.hh"
#include "pid/pid.hh"
#include "shared_memory/shared_memory.hh"
#include "task_pid.hh"
#include "utils/ecu_mode.hh"
#include "utils/safe_esp_log.hh"

[[noreturn]] void task_pid(__attribute__((unused)) void* params)
{
    ESP_LOGI(__FILE_NAME__, "task_pid: Starting PID control loop");
    PID pid{};
    TickType_t last_wake = xTaskGetTickCount();

    while (true)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));

        const auto mode = SharedMemory::get_mode();
        float output = 0.0f;

        if (mode == ECUMode::AUTO)
        {
            const float speed = SharedMemory::get_speed();
            const float setpoint = SharedMemory::get_setpoint();
            output = pid.compute(setpoint, speed, 0.1f);

            ESP_LOGI(__FILE_NAME__,
                     "task_pid: speed=%.2f, setpoint=%.2f, output=%.2f", speed,
                     setpoint, output);
        }
        else
        {
            ESP_LOGI(__FILE_NAME__,
                     "task_pid: mode is not AUTO, forcing output to 0.0");
        }

        SharedMemory::set_output(output);
        if (mode == ECUMode::AUTO)
        {
            SharedMemory::inc_stats_output();
        }
        uart_emit_frame(builder::output(output));
    }
}