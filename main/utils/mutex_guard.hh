#pragma once

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
}

class MutexGuard
{
public:
    explicit MutexGuard(const SemaphoreHandle_t handle) : handle_{handle}
    {
        xSemaphoreTake(handle_, portMAX_DELAY);
    }

    ~MutexGuard()
    {
        xSemaphoreGive(handle_);
    }

    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;
    MutexGuard(MutexGuard&&) = delete;
    MutexGuard& operator=(MutexGuard&&) = delete;

private:
    SemaphoreHandle_t handle_;
};
