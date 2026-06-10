#include "pid.h"

pid_t pid_init() {
    pid_t pid;
    pid.integral = 0.0f;
    pid.prev_err = 0.0f;
    return pid;
}
 
float pid_compute(pid_t *pid, float setpoint, float measured, float dt) {
    float err                = setpoint - measured;
    float integral_candidate = pid->integral + err * dt;
    float derivative         = (err - pid->prev_err) / dt;
 
    float output = (1 * err)
                 + (0.1 * integral_candidate)
                 + (0.01 * derivative);
 
    if (output > 255.0f) {
        output = 255.0f;
        if (err > 0.0f) integral_candidate = pid->integral;
    } else if (output < 0.0f) {
        output = 0.0f;
        if (err < 0.0f) integral_candidate = pid->integral;
    }
 
    pid->integral = integral_candidate;
    pid->prev_err = err;
    return output;
}
 