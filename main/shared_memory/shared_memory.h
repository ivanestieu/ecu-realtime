#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdint.h>

void init_shared_memory();

float get_speed();

float get_setpoint();
uint8_t get_mode();
float get_output();
uint32_t get_last_rx_tick();
uint32_t get_stats_valid();
uint32_t get_stats_corrupted();
uint32_t get_stats_dropped();
uint32_t get_stats_output();

void set_speed(float v);

void set_setpoint(float v);

void set_mode(uint8_t v);

void set_output(float v);

void set_last_rx_tick(uint32_t t);

void inc_stats_valid();
void inc_stats_corrupted();
void inc_stats_dropped();
void inc_stats_output();

#endif // SHARED_MEMORY_H
