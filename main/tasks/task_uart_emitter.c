#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/uart.h>
#include <hal/uart_types.h>

#include "task_uart_emitter.h"
#include "utils/safe_esp_log.h"

static QueueHandle_t tx_queue = NULL;

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

static bool frame_to_txframe(const Frame *frame, TxFrame *out)
{
    const Span full = get_full_frame(frame);
    if (full.size > FRAME_MAX_SIZE)
    {
        ESP_LOGW(__FILE_NAME__, "frame too large: %u > %u", (unsigned)full.size,
                 (unsigned)FRAME_MAX_SIZE);
        return false;
    }
    out->high_priority = false;
    out->size = full.size;
    memcpy(out->data, full.data, out->size);
    return true;
}

bool uart_emit_frame(const Frame *frame)
{
    TxFrame tx = { 0 };
    if (!frame_to_txframe(frame, &tx))
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

bool uart_emit_failsafe_frame(const Frame *frame)
{
    TxFrame tx = { 0 };
    if (!frame_to_txframe(frame, &tx))
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
    TxFrame tx = { 0 };
    QueueHandle_t q = get_tx_queue();
    if (!q)
    {
        ESP_LOGE(__FILE_NAME__, "task_uart_emitter: tx_queue not available");
        vTaskDelete(NULL);
        return;
    }

    while (true)
    {
        if (xQueueReceive(q, &tx, portMAX_DELAY) == pdTRUE)
        {
            const int written = uart_write_bytes(UART_NUM_0, tx.data, tx.size);
            if (written != tx.size)
            {
                ESP_LOGW(__FILE_NAME__,
                         "task_uart_emitter: uart_write_bytes wrote %d / %u",
                         written, (unsigned)tx.size);
            }
        }
    }
}