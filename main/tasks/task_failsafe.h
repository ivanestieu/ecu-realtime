#ifndef TASK_FAILSAFE_H
#define TASK_FAILSAFE_H

[[noreturn]] void task_failsafe(void* params);
void failsafe_gpio_isr_handler(void* arg);

#endif // TASK_FAILSAFE_H