#pragma once

#if __has_include(<esp_log.h>)
extern "C"
{
#    include <esp_log.h>
}
#else
#    define ESP_LOGI(tag, format, ...) ((void)0)
#endif

#include "pid.hh"

inline PID::PID()
    : integral_(0.0f)
    , prev_error_(0.0f)
{}

inline float PID::compute(const float setpoint, const float measured,
                          const float dt)
{
    const float err = setpoint - measured;
    float integral_candidate = integral_ + err * dt;
    const float derivative = (err - prev_error_) / dt;

    float output = err + (0.1 * integral_candidate) + (0.01 * derivative);

    if (output > 255.0f)
    {
        output = 255.0f;
        if (err > 0.0f)
        {
            integral_candidate = integral_;
        }
    }
    else if (output < 0.0f)
    {
        output = 0.0f;
        if (err < 0.0f)
        {
            integral_candidate = integral_;
        }
    }

    integral_ = integral_candidate;
    prev_error_ = err;
    ESP_LOGI(__FILE_NAME__, "computed output: %d", output);
    return output;
}
