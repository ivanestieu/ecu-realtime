#ifndef TASK_UART_EMITTER_H
#define TASK_UART_EMITTER_H

#include <freertos/queue.h>

#include "frame/frame.h"

#define TX_QUEUE_LENGTH 16

typedef struct
{
    bool high_priority;
    uint16_t size;
    uint8_t data[FRAME_MAX_SIZE];
} TxFrame;

QueueHandle_t get_tx_queue();

bool uart_emit_failsafe_frame(const Frame *frame);
bool uart_emit_frame(const Frame *frame);

void task_uart_emitter(void* params);

#endif // TASK_UART_EMITTER_H
