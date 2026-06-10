#include "task_failsafe.hh"
#include "shared_state.hh"
#include "frame/builder.hh"
#include "frame/parser.hh"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
 
// Handle de la tâche — nécessaire pour notifier depuis l'ISR
static TaskHandle_t failsafe_task_handle = NULL;
 
// Notification values pour distinguer les deux déclencheurs
#define NOTIFY_TIMEOUT  (1 << 0)
#define NOTIFY_GPIO     (1 << 1)
 
/**
 * ISR GPIO — appelée automatiquement par le hardware
 * On fait le minimum : notifier la tâche failsafe
 */
void IRAM_ATTR failsafe_gpio_isr_handler(void *arg) {
    BaseType_t higher_priority_woken = pdFALSE;
    xTaskNotifyFromISR(failsafe_task_handle,
                       NOTIFY_GPIO,
                       eSetBits,
                       &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}
 
/**
 * Déclenche le failsafe : force MODE_OFF et envoie une ALARM
 */
static void trigger_failsafe(const char *reason) {
    set_mode(MODE_OFF);
 
    uint8_t buf[80];
    int len = builder_alarm(buf, reason);
    uart_write_bytes(UART_NUM_0, buf, len);
}
 
void task_failsafe(void *params) {
    (void)params;
 
    // Sauvegarder le handle pour l'ISR
    failsafe_task_handle = xTaskGetCurrentTaskHandle();
 
    while (1) {
        // Attendre une notification (timeout ou GPIO)
        // ou vérifier le timeout toutes les 100ms
        uint32_t notification = 0;
        xTaskNotifyWait(0, UINT32_MAX, &notification, pdMS_TO_TICKS(100));
 
        // Déclencheur GPIO
        if (notification & NOTIFY_GPIO) {
            trigger_failsafe("FAILSAFE: GPIO emergency stop");
            continue;
        }
 
        // Déclencheur timeout — vérifier si silence > 2s
        uint32_t now     = xTaskGetTickCount();
        uint32_t last_rx = get_last_rx_tick();
        uint32_t elapsed = (now - last_rx) * portTICK_PERIOD_MS;
 
        if (elapsed > 2000 && get_mode() != MODE_OFF) {
            trigger_failsafe("FAILSAFE: no valid frame for 2s");
        }
    }
}