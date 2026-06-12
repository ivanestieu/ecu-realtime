extern "C"
{
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

#include "ecu_mode.hh"
#include "frame/builder.hh"
#include "frame/frame.hh"
#include "pid/pid.hh"
#include "shared_memory/shared_memory.hh"
#include "task_pid.hh"

void task_pid(__attribute__((unused))void* params)
{
    PID pid{};
    TickType_t last_wake = xTaskGetTickCount();

    while (true)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));

        if (SharedMemory::get_mode() != ECUMode::AUTO)
        {
            continue;
        }

        const float speed = SharedMemory::get_speed();
        const float setpoint = SharedMemory::get_setpoint();

        const float output = pid.compute(setpoint, speed, 0.1f);

        SharedMemory::set_output(output);
        SharedMemory::inc_stats_output();

        Frame frame = builder::output(output);
        const auto& frame_bytes = frame.get_full_frame();
        uart_write_bytes(UART_NUM_0, frame_bytes.data(), frame_bytes.size());
    }
}