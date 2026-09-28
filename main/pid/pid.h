#ifndef PID_H
#define PID_H

#include "utils/safe_esp_log.h"

typedef struct
{
    float integral_;
    float prev_error_;
} PID;

inline PID init_pid()
{
    const PID pid = {
        .integral_ = 0.0f,
        .prev_error_ = 0.0f
    };
    return pid;
}

static inline float compute(PID *pid, const float setpoint, const float measured,
                          const float dt)
{
    const float err = setpoint - measured;
    float integral_candidate = pid->integral_ + err * dt;
    const float derivative = (err - pid->prev_error_) / dt;

    float output = err + (0.1 * integral_candidate) + (0.01 * derivative);

    if (output > 255.0f)
    {
        output = 255.0f;
        if (err > 0.0f)
        {
            integral_candidate = pid->integral_;
        }
    }
    else if (output < 0.0f)
    {
        output = 0.0f;
        if (err < 0.0f)
        {
            integral_candidate = pid->integral_;
        }
    }

    pid->integral_ = integral_candidate;
    pid->prev_error_ = err;
    ESP_LOGI(__FILE_NAME__, "computed output: %d", output);
    return output;
}

#endif // PID_H
