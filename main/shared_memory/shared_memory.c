#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "shared_memory.h"
#include "utils/safe_esp_log.h"

static SemaphoreHandle_t g_mutex;

static float s_speed = 0.0f;
static float s_setpoint = 0.0f;
static uint8_t s_mode = 0x00;
static float s_output = 0.0f;
static uint32_t s_last_rx_tick = 0;
static uint32_t s_stats_valid = 0;
static uint32_t s_stats_corrupted = 0;
static uint32_t s_stats_dropped = 0;
static uint32_t s_stats_output = 0;

void init_shared_memory()
{
    g_mutex = xSemaphoreCreateMutex();
}

void set_speed(const float v)
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_speed = v;
    xSemaphoreGive(g_mutex);
}

float get_speed()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const float res = s_speed;
    xSemaphoreGive(g_mutex);
    return res;
}

void set_setpoint(const float v)
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_setpoint = v;
    xSemaphoreGive(g_mutex);
}

float get_setpoint()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const float res = s_setpoint;
    xSemaphoreGive(g_mutex);
    return res;
}

void set_mode(const uint8_t v)
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_mode = v;
    xSemaphoreGive(g_mutex);
}

uint8_t get_mode()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const uint8_t res = s_mode;
    xSemaphoreGive(g_mutex);
    return res;
}

void set_output(const float v)
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_output = v;
    xSemaphoreGive(g_mutex);
}

float get_output()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const float res = s_output;
    xSemaphoreGive(g_mutex);
    return res;
}

void set_last_rx_tick(const uint32_t t)
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_last_rx_tick = t;
    xSemaphoreGive(g_mutex);
}

uint32_t get_last_rx_tick()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const uint32_t res = s_last_rx_tick;
    xSemaphoreGive(g_mutex);
    return res;
}

void inc_stats_valid()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_stats_valid += 1;
    xSemaphoreGive(g_mutex);
}

void inc_stats_corrupted()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_stats_corrupted += 1;
    xSemaphoreGive(g_mutex);
}

void inc_stats_dropped()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_stats_dropped += 1;
    xSemaphoreGive(g_mutex);
}

void inc_stats_output()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    s_stats_output += 1;
    xSemaphoreGive(g_mutex);
}

uint32_t get_stats_valid()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const uint32_t res = s_stats_valid;
    xSemaphoreGive(g_mutex);
    return res;
}

uint32_t get_stats_corrupted()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const uint32_t res = s_stats_corrupted;
    xSemaphoreGive(g_mutex);
    return res;
}

uint32_t get_stats_dropped()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const uint32_t res = s_stats_dropped;
    xSemaphoreGive(g_mutex);
    return res;
}

uint32_t get_stats_output()
{
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    const uint32_t res = s_stats_output;
    xSemaphoreGive(g_mutex);
    return res;
}