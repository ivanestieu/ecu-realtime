# ECU RealTime – Embedded Cruise Control Regulator

> A robust real-time Electronic Control Unit (ECU) implementing deterministic PID-based speed regulation on ESP32 with FreeRTOS
>
> **Grade: 17/20** | EPITA 2026 Embedded Systems & Real-Time Engineering

---

## Project Overview

This project demonstrates the design and implementation of a safety-critical embedded control system. The ECU is a cruise control regulator that:

- **Receives** speed setpoint and sensor data via UART communication
- **Computes** motor commands using a discrete PID controller
- **Outputs** control signals with strict real-time guarantees
- **Ensures** safe failsafe behavior under sensor loss or communication failures
- **Handles** message corruption, floods, and hardware stress with zero control interruption

### Key Technical Challenge

Implementing hard real-time constraints (100ms cycle time, <5ms jitter) on a resource-constrained microcontroller while maintaining communication robustness and system safety—even under adversarial conditions (malformed packets, CPU overload, network silence).

---

## Highlights

- **Deterministic real-time control**: Guaranteed ≤5ms jitter on PID regulation cycle
- **Failsafe mechanism**: Automatic shutdown if no valid message received for 2s
- **Robustness under stress**: Handles packet fragmentation, CRC errors, and flood attacks
- **Priority inversion prevention**: Carefully designed FreeRTOS task architecture
- **Comprehensive telemetry**: Real-time statistics (RX valid/corrupt/dropped, TX count)
- **Production-quality design**: Clear separation of concerns with multi-threaded UART handling

---

## Architecture

### Task Structure

| Task | Priority | Period | Role |
|------|----------|--------|------|
| **ControlTask** | HIGH | 100 ms | PID calculation, motor command output |
| **UARTRxTask** | CRITICAL | Event-driven | Frame reception & parsing, CRC validation |
| **TelemetryTask** | LOW | 1 s | Statistics collection & emission |
| **FailsafeTask** | HIGH | 100 ms | Timeout detection, safety shutdown |
| **GPIOMonitorTask** | CRITICAL | Event-driven | External error signal (optional) |

### Design Principles

- **Priority-based preemption**: UART reception cannot delay control loop
- **Message queue isolation**: Malformed frames dropped without affecting regulation
- **Dual failsafe triggers**: Both timeout and external GPIO independently trigger shutdown
- **Anti-windup integration**: PID integral term clipped when saturated to prevent overshoot
- **Atomic frame emission**: Mutex protection prevents byte interleaving on UART

---

## Technical Implementation

### PID Controller

```c
// Discrete PID with saturation and anti-windup
output = (Kp * error) + (Ki * ∫error·dt) + (Kd * d(error)/dt)
// Saturated to [0, 255]
// Integral clipping when output is saturated

// Default coefficients:
Kp = 1.0, Ki = 0.1, Kd = 0.01
```

### Communication Protocol

Custom variable-length frame format (inspired by CAN):
```
[START: 0xAA] [LEN: 2B] [TYPE: 1B] [PAYLOAD: N bytes] [CRC: 1B]
```

**Message Types:**
- `0x01` SETPOINT (float) – Target speed
- `0x02` SPEED (float) – Measured speed  
- `0x05` MODE_SET (u8) – OFF / MANUAL / AUTO
- `0x80` OUTPUT (float) – Motor command
- `0x83` STATS (u32[4]) – Telemetry counters
- `0x85` ALARM (string) – Critical fault messages

### Real-Time Guarantees

- **Cycle time**: Exactly 100ms ±5ms (hardware timer + task scheduling)
- **Failsafe response**: <5ms from GPIO interrupt or message timeout
- **Frame integrity**: Mutex-protected UART writes, no byte interleaving
- **Jitter control**: Dedicated high-priority control task, preemption points analyzed

---

## Getting Started

### Prerequisites

- **Hardware**: ESP32 DevKit
- **Framework**: ESP-IDF 5.x
- **OS**: Ubuntu/Linux for build


### Testing

The repository includes a Python test suite that validates:

---

**Test scenarios:**
- Normal operation (30s)
- Packet fragmentation (pauses mid-frame)
- CRC corruption injection
- CPU overload (50 random message flood)
- Failsafe timeout (2.5s silence)

## Key Lessons & Design Decisions

### Why Multi-Task Architecture?

A monolithic design would have failed under the real-time constraints:
- **UART blocking** would delay PID calculation
- **Message parsing** errors could corrupt regulation loop
- **Failsafe detection** might miss timeout windows

**Solution**: Dedicated threads with strict priority discipline and queue-based decoupling.

### Handling Priority Inversion

**Challenge**: Low-priority telemetry task acquiring UART mutex could block high-priority control task.

**Solution**: Use FreeRTOS priority inheritance mutexes + spinlock for critical sections <1ms.

### Robustness Under Packet Loss

**Challenge**: Malformed frames could leave parser in inconsistent state.

**Solution**: 
- Frame boundary detection using 0xAA start marker
- CRC validation on complete frames only
- Silent rejection with error counter (reported in telemetry)

---

## Technologies & Tools

- **Microcontroller**: ESP32
- **RTOS**: FreeRTOS with full preemption
- **Framework**: ESP-IDF (Espressif IoT Development Framework)
- **Language**: C++
- **Testing**: Python 3.9 + PySerial
- **Version Control**: Git

---

## Performance & Limitations

### Known Limitations

1. **Fixed PID coefficients**: Tuned for simulation model; real vehicle may require recalibration
2. **Single ECU**: No multi-vehicle communication or CAN bus integration
3. **No persistent logging**: Telemetry lost on power cycle (could add SPIFFS/SD card)
4. **GPIO failsafe optional**: Implemented but requires external circuit

---

## Learning Outcomes

This project reinforced expertise in:

- **Real-time system design**: Task scheduling, timing analysis, jitter control
- **Embedded C++ programming**: Memory-constrained, zero-allocation design patterns
- **RTOS fundamentals**: Mutexes, semaphores, queues, priority inheritance
- **Safety engineering**: Failsafe mechanisms, defensive programming, robustness testing
- **Hardware-software co-design**: Interrupt handling, timer configuration, serial protocols

---

## Author

**Ivan Estieu** – Embedded Systems Engineer Student  
**Iban Peyret** - Embedded Systems Engineer Student
EPITA 2026 | Specialized in Embedded Systems

---

**Last Updated**: August 2026  
**Grade**: 17/20
