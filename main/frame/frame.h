#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>

#define FRAME_MAX_SIZE 64u
#define START_BYTE 0xAA

#define HEADER_SIZE 4u
#define CRC_SIZE 1u

enum MsgType
{
    SETPOINT = 0x01,
    SPEED = 0x02,
    MODE_SET = 0x05,
    OUTPUT = 0x80,
    STATS = 0x83,
    ALARM = 0x85,
    DEBUG = 0xFF,
};

typedef struct
{
    uint16_t frame_len_;
    uint8_t type_;
    uint16_t current_index_;
    uint8_t byte_frame_[FRAME_MAX_SIZE];
} Frame;

typedef struct
{
    const uint8_t *data;
    const uint16_t size;
} Span;

static inline Frame init_frame(const uint8_t payload_type, const uint16_t payload_len)
{
    Frame frame = {
        .frame_len_ = HEADER_SIZE + payload_len + CRC_SIZE,
        .type_= payload_type,
        .current_index_ = 0,
        .byte_frame_ = { 0 }
    };
    frame.byte_frame_[frame.current_index_++] = START_BYTE;
    frame.byte_frame_[frame.current_index_++] = (payload_len + CRC_SIZE) & 0xFF;
    frame.byte_frame_[frame.current_index_++] = ((payload_len + CRC_SIZE) >> 8) & 0xFF;
    frame.byte_frame_[frame.current_index_++] = payload_type;
    return frame;
}

inline void set_type(Frame *frame, const uint8_t type)
{
    frame->type_ = type;
}

inline void payload_append(Frame *frame, const uint8_t byte)
{
    frame->byte_frame_[frame->current_index_++] = byte;
}

[[nodiscard]] inline uint16_t size(const Frame *frame)
{
    return frame->current_index_;
}

static inline void compute_crc(Frame *frame)
{
    uint8_t crc = 0;
    for (size_t i = 1; i < size(frame); ++i)
    {
        crc ^= frame->byte_frame_[i];
    }
    frame->byte_frame_[frame->current_index_++] = crc;
}


[[nodiscard]] inline bool is_payload_complete(const Frame *frame, const uint16_t frame_len)
{
    return size(frame) - HEADER_SIZE == frame_len - CRC_SIZE;
}

[[nodiscard]] inline enum MsgType get_type(const Frame *frame)
{
    return frame->type_;
}

[[nodiscard]] inline uint16_t get_payload_len(const Frame *frame)
{
    return size(frame) - HEADER_SIZE - CRC_SIZE;
}

[[nodiscard]] static inline Span get_payload(const Frame *frame)
{
    const Span span = { .data = frame->byte_frame_ + HEADER_SIZE,.size = get_payload_len(frame) };
    return span;
}

[[nodiscard]] inline Span get_full_frame(const Frame *frame)
{
    const Span span = { .data = frame->byte_frame_ ,.size = size(frame) };
    return span;
}

#endif // FRAME_H
