#pragma once

#include <optional>

#include "frame.hh"

namespace Parser
{
    enum State
    {
        IDLE, // Waiting for starting 0xAA byte
        LEN_LOW, // Read bottom length byte (least significant bit)
        LEN_HIGH, // Read top length byte (least significant bit)
        TYPE, // Read message type
        PAYLOAD, // Read payload
        CRC, // Read and verify CRC
    };

    std::uint8_t compute_crc(const std::array<std::uint8_t, Frame::FRAME_MAX_SIZE>& data, unsigned int size);

    class Parser
    {
    public:
        Parser();
        std::optional<Frame> push_byte(std::uint8_t byte);

    private:
        void reinitialize();
        std::optional<Frame> handle_idle(std::uint8_t byte);
        std::optional<Frame> handle_len_low(std::uint8_t byte);
        std::optional<Frame> handle_len_high(std::uint8_t byte);
        std::optional<Frame> handle_type(std::uint8_t byte);
        std::optional<Frame> handle_payload(uint8_t byte);
        std::optional<Frame> handle_crc(std::uint8_t byte);

        State state_;
        std::uint16_t frame_len_; // Expected length
        std::uint8_t crc_calc_; // CRC calculated along frame reading
        std::uint8_t type_; // Message type
        std::optional<Frame> current_frame_;
    };
} // namespace Parser

#include "parser.hxx"
