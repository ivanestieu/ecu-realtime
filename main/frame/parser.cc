#include "parser.hh"

namespace Parser
{
    Parser::Parser()
        : state_(State::IDLE)
        , frame_len_(0)
        , crc_calc_(0)
        , type_(0)
        , current_frame_{ std::nullopt }
    {
    }

    std::uint8_t compute_crc(const std::array<std::uint8_t, Frame::FRAME_MAX_SIZE>& data, const unsigned int size)
    {
        std::uint8_t crc = 0;
        for (size_t i = 1; i < size; ++i)
        {
            crc ^= data[i];
        }
        return crc;
    }

    void Parser::reinitialize()
    {
        state_ = State::IDLE;
        frame_len_ = 0;
        crc_calc_ = 0;
        type_ = 0;
        current_frame_ = std::nullopt;
    }

    std::optional<Frame> Parser::push_byte(const uint8_t byte)
    {
        switch (state_)
        {
        case State::IDLE:
            return handle_idle(byte);
        case State::LEN_LOW:
            return handle_len_low(byte);
        case State::LEN_HIGH:
            return handle_len_high(byte);
        case State::TYPE:
            return handle_type(byte);
        case State::PAYLOAD:
            return handle_payload(byte);
        case State::CRC:
            return handle_crc(byte);
        }
        return std::nullopt;
    }
} // namespace Parser
