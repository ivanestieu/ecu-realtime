#pragma once
#include <stdint.h>

void shared_state_init();

void set_speed(float v);
float get_speed();

void set_setpoint(float v);
float get_setpoint();

void set_mode(uint8_t v);
uint8_t get_mode();

void set_output(float v);
float get_output();

void set_last_rx_tick(uint32_t t);
uint32_t get_last_rx_tick();

void inc_stats_valid();
void inc_stats_corrupted();
void inc_stats_dropped();
void inc_stats_output();
uint32_t get_stats_valid();
uint32_t get_stats_corrupted();
uint32_t get_stats_dropped();
uint32_t get_stats_output();
