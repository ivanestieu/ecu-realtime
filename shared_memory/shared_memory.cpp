#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "shared_state.h"

static SemaphoreHandle_t mutex;

static float s_speed = 0.0f;
static float s_setpoint = 0.0f;
static uint8_t s_mode = 0x00;
static float s_output = 0.0f;
static uint32_t s_last_rx_tick = 0;
static uint32_t s_stats_valid = 0;
static uint32_t s_stats_corrupted = 0;
static uint32_t s_stats_dropped = 0;
static uint32_t s_stats_output = 0;

void shared_state_init()
{
    mutex = xSemaphoreCreateMutex();
}

void set_speed(float v)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_speed = v;
    xSemaphoreGive(mutex);
}

float get_speed()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    float v = s_speed;
    xSemaphoreGive(mutex);
    return v;
}

void set_setpoint(float v)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_setpoint = v;
    xSemaphoreGive(mutex);
}

float get_setpoint()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    float v = s_setpoint;
    xSemaphoreGive(mutex);
    return v;
}

void set_mode(uint8_t v)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_mode = v;
    xSemaphoreGive(mutex);
}

uint8_t get_mode()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint8_t v = s_mode;
    xSemaphoreGive(mutex);
    return v;
}

void set_output(float v)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_output = v;
    xSemaphoreGive(mutex);
}

float get_output()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    float v = s_output;
    xSemaphoreGive(mutex);
    return v;
}

void set_last_rx_tick(uint32_t t)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_last_rx_tick = t;
    xSemaphoreGive(mutex);
}

uint32_t get_last_rx_tick()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint32_t t = s_last_rx_tick;
    xSemaphoreGive(mutex);
    return t;
}

void inc_stats_valid()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_stats_valid++;
    xSemaphoreGive(mutex);
}

void inc_stats_corrupted()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_stats_corrupted++;
    xSemaphoreGive(mutex);
}

void inc_stats_dropped()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_stats_dropped++;
    xSemaphoreGive(mutex);
}

void inc_stats_output()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    s_stats_output++;
    xSemaphoreGive(mutex);
}

uint32_t get_stats_valid()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint32_t v = s_stats_valid;
    xSemaphoreGive(mutex);
    return v;
}

uint32_t get_stats_corrupted()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint32_t v = s_stats_corrupted;
    xSemaphoreGive(mutex);
    return v;
}

uint32_t get_stats_dropped()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint32_t v = s_stats_dropped;
    xSemaphoreGive(mutex);
    return v;
}

uint32_t get_stats_output()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint32_t v = s_stats_output;
    xSemaphoreGive(mutex);
    return v;
}