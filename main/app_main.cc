extern "C"
{
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/task.h"
}
#include "utils/safe_esp_log.hh"
#include "shared_memory/shared_memory.hh"
#include "tasks/task_failsafe.hh"
#include "tasks/task_pid.hh"
#include "tasks/task_rx.hh"
#include "tasks/task_telemetry.hh"

static constexpr int KiB = 1 << 10;

/**
 * Initialize UART0 for communication with PC
 * Baud rate: 115200, data bits: 8, stop bits: 1, parity: none
 */
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

    // Configure UART0
    uart_driver_install(UART_NUM_0, 256, 256, 0, nullptr, 0);
    uart_param_config(UART_NUM_0, &uart_config);

    // Set UART pins (TX=GPIO1, RX=GPIO3 on ESP32)
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(__FILE_NAME__, "UART initialized (115200 baud)");
}

/**
 * Initialize GPIO for failsafe emergency button
 * Uses GPIO4 as input with falling edge interrupt
 */
static void gpio_init()
{
    // Configure GPIO4 as input
    constexpr gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_4),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE, // Trigger on falling edge
    };
    gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    ESP_LOGI(__FILE_NAME__, "GPIO initialized (failsafe on GPIO4)");
}

/**
 * app_main — Entry point for ESP-IDF application
 * Initializes hardware and launches FreeRTOS tasks
 */
extern "C" void app_main(void)
{
    ESP_LOGI(__FILE_NAME__, "=== ECU RealTime Cruise Control Starting ===");

    // 1. Initialize shared state (mutex, variables)
    SharedMemory::init();
    ESP_LOGI(__FILE_NAME__, "Shared state initialized");

    // 2. Initialize UART
    uart_init();

    // 3. Initialize GPIO
    gpio_init();

    // 4. Create FreeRTOS tasks

    // task_rx: high priority (4), continuous UART reading
    xTaskCreate(task_rx, "task_rx", 4 * KiB, nullptr, 3, nullptr);
    ESP_LOGI(__FILE_NAME__, "task_rx created (priority 3, stack 4KB)");

    // task_pid: high priority (4), strict 100ms period
    xTaskCreate(task_pid, "task_pid", 3 * KiB, nullptr, 4, nullptr);
    ESP_LOGI(__FILE_NAME__,
             "task_pid created (priority 4, stack 3KB, 100ms period)");

    // task_failsafe: max priority (5), event-driven safety
    xTaskCreate(task_failsafe, "task_failsafe", 2 * KiB, nullptr, 5, nullptr);
    ESP_LOGI(__FILE_NAME__, "task_failsafe created (priority 5, stack 2KB)");

    // task_telemetry: low priority (2), 1s statistics
    xTaskCreate(task_telemetry, "task_telemetry", 2 * KiB, nullptr, 2, nullptr);
    ESP_LOGI(__FILE_NAME__,
             "task_telemetry created (priority 2, stack 2KB, 1s period)");

    ESP_LOGI(__FILE_NAME__, "=== All tasks launched. System ready ===");
}
