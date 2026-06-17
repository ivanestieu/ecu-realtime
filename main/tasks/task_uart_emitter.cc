extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "hal/uart_types.h"
}

#include <cstring>
#include "task_uart_emitter.hh"

static QueueHandle_t tx_queue = nullptr;

QueueHandle_t get_tx_queue()
{
    if (!tx_queue)
    {
        tx_queue = xQueueCreate(TX_QUEUE_LENGTH, sizeof(TxFrame));
        if (!tx_queue)
        {
            ESP_LOGE(__FILE_NAME__, "Failed to create tx_queue");
        }
    }
    return tx_queue;
}

static bool frame_to_txframe(const Frame& frame, TxFrame& out)
{
    const auto full = frame.get_full_frame();
    if (full.size() > Frame::FRAME_MAX_SIZE)
    {
        ESP_LOGW(__FILE_NAME__, "frame too large: %u > %u", (unsigned)full.size(),
                 (unsigned)Frame::FRAME_MAX_SIZE);
        return false;
    }
    out.high_priority = false;
    out.size = static_cast<std::uint16_t>(full.size());
    std::memcpy(out.data, full.data(), out.size);
    return true;
}

bool uart_emit_frame(const Frame& frame)
{
    TxFrame tx{};
    if (!frame_to_txframe(frame, tx))
        return false;
    QueueHandle_t q = get_tx_queue();
    if (!q)
        return false;
    if (xQueueSend(q, &tx, (TickType_t)0) != pdPASS)
    {
        ESP_LOGW(__FILE_NAME__, "uart_emit_frame: tx queue full");
        return false;
    }
    return true;
}

bool uart_emit_failsafe_frame(const Frame& frame)
{
    TxFrame tx{};
    if (!frame_to_txframe(frame, tx))
        return false;
    tx.high_priority = true;
    QueueHandle_t q = get_tx_queue();
    if (!q)
        return false;
    if (xQueueSendToFront(q, &tx, (TickType_t)0) != pdPASS)
    {
        ESP_LOGW(__FILE_NAME__, "uart_emit_failsafe_frame: tx queue full");
        return false;
    }
    return true;
}

void task_uart_emitter(__attribute__((unused)) void* params)
{
    TxFrame tx{};
    QueueHandle_t q = get_tx_queue();
    if (!q)
    {
        ESP_LOGE(__FILE_NAME__, "task_uart_emitter: tx_queue not available");
        vTaskDelete(nullptr);
        return;
    }

    constexpr uart_port_t UART_PORT = UART_NUM_0;

    while (true)
    {
        if (xQueueReceive(q, &tx, portMAX_DELAY) == pdTRUE)
        {
            int written = uart_write_bytes(UART_PORT, tx.data, tx.size);
            if (written != static_cast<int>(tx.size))
            {
                ESP_LOGW(__FILE_NAME__,
                         "task_uart_emitter: uart_write_bytes wrote %d / %u",
                         written, (unsigned)tx.size);
            }
        }
    }
}