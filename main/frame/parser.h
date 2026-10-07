#ifndef PARSER_H
#define PARSER_H

#include "frame.h"

enum State
{
    IDLE, // Waiting for starting 0xAA byte
    LEN_LOW, // Read bottom length byte (least significant bit)
    LEN_HIGH, // Read top length byte (least significant bit)
    TYPE, // Read message type
    PAYLOAD, // Read payload
    CRC, // Read and verify CRC
};

typedef struct
{
    bool has_value;
    Frame frame;
} OptionalFrame;

[[nodiscard]] inline OptionalFrame empty_optional()
{
    const OptionalFrame frame = { 0 };
    return frame;
}

typedef struct
{
    enum State state_;
    uint16_t frame_len_; // Expected length
    uint8_t crc_calc_; // CRC calculated along frame reading
    uint8_t type_; // Message type
    OptionalFrame current_frame_;
} Parser;

Parser init_parser();
Frame *push_byte(Parser *parser,uint8_t byte);

void reinitialize(Parser *parser);
inline Frame *handle_idle(Parser *parser, const uint8_t byte)
{
    if (byte == START_BYTE)
    {
        reinitialize(parser);
        parser->state_ = LEN_LOW;
    }
    return NULL;
}

inline Frame *handle_len_low(Parser *parser, const uint8_t byte)
{
    parser->frame_len_ = byte;
    parser->crc_calc_ = byte;
    parser->state_ = LEN_HIGH;
    return NULL;
}

inline Frame *handle_len_high(Parser *parser, const uint8_t byte)
{
    const uint16_t double_byte = byte;
    parser->frame_len_ |= double_byte << 8;
    parser->crc_calc_ ^= byte;

    if (parser->frame_len_ == 0)
    {
        parser->state_ = IDLE;
        return NULL;
    }

    parser->state_ = TYPE;
    return NULL;
}

inline Frame *handle_type(Parser *parser, const uint8_t byte)
{
    parser->type_ = byte;
    parser->crc_calc_ ^= byte;

    if (parser->frame_len_ == 1)
    {
        parser->state_ = CRC;
    }
    else
    {
        parser->state_ = PAYLOAD;
    }
    return NULL;
}

static inline Frame *handle_payload(Parser *parser, const uint8_t byte)
{
    if (!parser->current_frame_.has_value)
    {
        parser->current_frame_.frame =
            init_frame(parser->type_, parser->frame_len_ - 1u);
        parser->current_frame_.has_value = true;
    }
    if (!is_payload_complete(&parser->current_frame_.frame, parser->frame_len_))
    {
        payload_append(&parser->current_frame_.frame, byte);
    }
    parser->crc_calc_ ^= byte;

    if (is_payload_complete(&parser->current_frame_.frame, parser->frame_len_))
    {
        parser->state_ = CRC;
    }
    return NULL;
}

static inline Frame *handle_crc(Parser *parser, uint8_t byte)
{
    const uint8_t crc_received = byte;

    set_type(&parser->current_frame_.frame, parser->type_);
    compute_crc(&parser->current_frame_.frame);

    if (crc_received == parser->crc_calc_)
    {
        parser->state_ = IDLE;
        Frame *res = &parser->current_frame_.frame;
        parser->current_frame_.has_value = false;
        return res;
    }
    parser->state_ = IDLE;
    return NULL;
}

#endif // PARSER_H
