#pragma once
#include <memory>
#include <vector>

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

    enum ECUMode
    {
        OFF = 0x00,
        MANUAL = 0x01,
        AUTO = 0x02,
    };

    std::uint8_t compute_crc(const std::vector<uint8_t>& data);

    class Parser
    {
    public:
        Parser();
        std::unique_ptr<Frame> push_byte(std::uint8_t byte);

    private:
        void reinitialize();
        std::unique_ptr<Frame> handle_idle(std::uint8_t byte);
        std::unique_ptr<Frame> handle_len_low(std::uint8_t byte);
        std::unique_ptr<Frame> handle_len_high(std::uint8_t byte);
        std::unique_ptr<Frame> handle_type(std::uint8_t byte);
        std::unique_ptr<Frame> handle_payload(uint8_t byte);
        std::unique_ptr<Frame> handle_crc(std::uint8_t byte);

        State state_;
        std::uint16_t frame_len_; // Expected length
        std::uint8_t crc_calc_; // CRC calculated along frame reading
        std::uint8_t type_; // Message type
        std::unique_ptr<Frame> current_frame_;
    };
} // namespace Parser

#include "parser.hxx"
