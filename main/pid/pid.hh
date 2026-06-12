#pragma once

class PID
{
public:
    PID();
    float compute(float setpoint, float measured, float dt);

private:
    float integral_;
    float prev_error_;
};

#include "pid.hxx"