#ifndef SAFE_ESP_LOG_H
#define SAFE_ESP_LOG_H

#if __has_include(<esp_log.h>)
#    include <esp_log.h>
#else
#    define ESP_LOGI(tag, format, ...) ((void)0)
#endif

#endif // SAFE_ESP_LOG_H
