#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "task_pid.h"

#include "pid/pid.h"
#include "shared_memory/shared_memory.h"
#include "task_uart_emitter.h"
#include "frame/builder.h"
#include "utils/ecu_mode.h"
#include "utils/safe_esp_log.h"

_Noreturn void task_pid(__attribute__((unused)) void* params)
{
    ESP_LOGI(__FILE_NAME__, "task_pid: Starting PID control loop");
    PID pid = init_pid();
    TickType_t last_wake = xTaskGetTickCount();

    while (true)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));

        const uint8_t mode = get_mode();
        float out = 0.0f;

        if (mode == AUTO)
        {
            const float speed = get_speed();
            const float setpoint = get_setpoint();
            out = compute(&pid, setpoint, speed, 0.1f);

            ESP_LOGI(__FILE_NAME__,
                     "task_pid: speed=%.2f, setpoint=%.2f, output=%.2f", speed,
                     setpoint, out);
        }
        else
        {
            ESP_LOGI(__FILE_NAME__,
                     "task_pid: mode is not AUTO, forcing output to 0.0");
        }

        set_output(out);
        if (mode == AUTO)
        {
            inc_stats_output();
        }
        Frame frame = output(out);
        uart_emit_frame(&frame);
    }
}