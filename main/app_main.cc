extern "C"
{
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/task.h"
}
#include "shared_memory/shared_memory.hh"
#include "tasks/task_failsafe.hh"
#include "tasks/task_pid.hh"
#include "tasks/task_rx.hh"
#include "tasks/task_telemetry.hh"
#include "tasks/task_uart_emitter.hh"
#include "utils/safe_esp_log.hh"

static constexpr int KiB = 1 << 10;

static void uart_init()
{
    constexpr uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .rx_glitch_filt_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {},
    };

    uart_driver_install(UART_NUM_0, 256, 256, 0, nullptr, 0);
    uart_param_config(UART_NUM_0, &uart_config);

    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(__FILE_NAME__, "UART initialized (115200 baud)");
}

static void gpio_init()
{
    constexpr gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_4),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    ESP_LOGI(__FILE_NAME__, "GPIO initialized (failsafe on GPIO4)");
}

extern "C" void app_main(void)
{
    ESP_LOGI(__FILE_NAME__, "=== ECU RealTime Cruise Control Starting ===");

    SharedMemory::init();
    ESP_LOGI(__FILE_NAME__, "Shared state initialized");

    uart_init();

    gpio_init();

    xTaskCreate(task_uart_emitter, "uart_emitter", 4 * KiB, nullptr, 3,
                nullptr);
    ESP_LOGI(__FILE_NAME__, " uart_emitter created (priority 3, stack 4KB)");

    xTaskCreate(task_rx, "task_rx", 4 * KiB, nullptr, 3, nullptr);
    ESP_LOGI(__FILE_NAME__, "task_rx created (priority 3, stack 4KB)");

    xTaskCreate(task_pid, "task_pid", 3 * KiB, nullptr,4, nullptr);
    ESP_LOGI(__FILE_NAME__,
             "task_pid created (priority 4, stack 3KB, 100ms period)");

    xTaskCreate(task_failsafe, "task_failsafe", 2 * KiB, nullptr, 5, nullptr);
    ESP_LOGI(__FILE_NAME__, "task_failsafe created (priority 5, stack 2KB)");

    xTaskCreate(task_telemetry, "task_telemetry", 2 * KiB, nullptr, 2, nullptr);
    ESP_LOGI(__FILE_NAME__,
             "task_telemetry created (priority 2, stack 2KB, 1s period)");

    ESP_LOGI(__FILE_NAME__, "=== All tasks launched. System ready ===");
}
