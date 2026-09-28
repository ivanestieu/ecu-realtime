#include "parser.h"

Parser init_parser()
{
    const Parser parser = {
        .state_ = IDLE,
        .frame_len_ = 0,
        .crc_calc_ = 0,
        .type_= 0,
        .current_frame_ = empty_optional()
    };
    return parser;
}

void reinitialize(Parser* parser)
{
    parser->state_ = IDLE;
    parser->frame_len_ = 0;
    parser->crc_calc_ = 0;
    parser->type_ = 0;
    parser->current_frame_ = empty_optional();
}

Frame *push_byte(Parser *parser, const uint8_t byte)
{
    switch (parser->state_)
    {
    case IDLE:
        return handle_idle(parser, byte);
    case LEN_LOW:
        return handle_len_low(parser, byte);
    case LEN_HIGH:
        return handle_len_high(parser, byte);
    case TYPE:
        return handle_type(parser, byte);
    case PAYLOAD:
        return handle_payload(parser, byte);
    case CRC:
        return handle_crc(parser, byte);
    }
    return NULL;
}
