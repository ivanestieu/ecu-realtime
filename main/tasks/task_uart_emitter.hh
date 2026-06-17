#pragma once

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
}

#include <cstdint>
#include "utils/safe_esp_log.hh"
#include "frame/frame.hh"

static constexpr size_t TX_QUEUE_LENGTH = 16;

struct TxFrame
{
    bool high_priority;
    std::uint16_t size;
    std::uint8_t data[Frame::FRAME_MAX_SIZE];
};

QueueHandle_t get_tx_queue();

bool uart_emit_failsafe_frame(const Frame& frame);
bool uart_emit_frame(const Frame& frame);

void task_uart_emitter(void* params);