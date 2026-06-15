#pragma once

[[noreturn]] void task_failsafe(void* params);
void failsafe_gpio_isr_handler(void* arg);