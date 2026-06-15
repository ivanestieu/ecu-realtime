#pragma once

#if __has_include(<esp_log.h>)
extern "C"
{
#    include <esp_log.h>
}
#else
#    define ESP_LOGI(tag, format, ...) ((void)0)
#endif

