#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
 
void task_failsafe(void *params);
void failsafe_gpio_isr_handler(void *arg);