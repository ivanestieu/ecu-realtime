#pragma once

#include "parser.hh"

namespace Parser
{
    inline std::optional<Frame> Parser::handle_idle(const std::uint8_t byte)
    {
        if (byte == Frame::START_BYTE)
        {
            reinitialize();
            state_ = State::LEN_LOW;
        }
        return std::nullopt;
    }

    inline std::optional<Frame> Parser::handle_len_low(const std::uint8_t byte)
    {
        frame_len_ = byte;
        crc_calc_ = byte;
        state_ = State::LEN_HIGH;
        return std::nullopt;
    }

    inline std::optional<Frame> Parser::handle_len_high(const std::uint8_t byte)
    {
        frame_len_ |= static_cast<std::uint16_t>(byte) << 8;
        crc_calc_ ^= byte;

        if (frame_len_ == 0)
        {
            state_ = State::IDLE;
            return std::nullopt;
        }

        state_ = State::TYPE;
        return std::nullopt;
    }

    inline std::optional<Frame> Parser::handle_type(const std::uint8_t byte)
    {
        type_ = byte;
        crc_calc_ ^= byte;

        if (frame_len_ == 1)
        {
            state_ = State::CRC;
        }
        else
        {
            state_ = State::PAYLOAD;
        }
        return std::nullopt;
    }

    inline std::optional<Frame> Parser::handle_payload(const std::uint8_t byte)
    {
        if (!current_frame_.has_value())
        {
            current_frame_ =
                Frame{ type_, static_cast<std::uint16_t>(frame_len_ - 1u) };
        }
        if (!current_frame_->is_payload_complete(frame_len_))
        {
            current_frame_->payload_append(byte);
        }
        crc_calc_ ^= byte;

        if (current_frame_->is_payload_complete(frame_len_))
        {
            state_ = State::CRC;
        }
        return std::nullopt;
    }

    inline std::optional<Frame> Parser::handle_crc(const std::uint8_t byte)
    {
        const uint8_t crc_received = byte;

        current_frame_->set_type(type_);
        current_frame_->compute_crc();

        if (crc_received == crc_calc_)
        {
            state_ = State::IDLE;
            const auto res = current_frame_;
            current_frame_ = std::nullopt;
            return res;
        }
        state_ = State::IDLE;
        return std::nullopt;
    }
} // namespace Parser
