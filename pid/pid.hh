#pragma once
 
typedef struct {
    float integral;
    float prev_err;
} pid_t;
 
pid_t pid_init();
float pid_compute(pid_t *pid, float setpoint, float measured, float dt);
 
