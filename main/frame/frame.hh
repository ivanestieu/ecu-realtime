#pragma once

#include <cstdint>
#include <span>

class Frame
{
public:
    static constexpr std::uint16_t FRAME_MAX_SIZE = 64u;
    static constexpr std::uint8_t START_BYTE = 0xAA;

private:
    static constexpr std::uint16_t HEADER_SIZE = 4u;
    static constexpr std::uint16_t CRC_SIZE = 1u;

public:
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

    Frame() = delete;
    Frame(std::uint8_t payload_type, std::uint16_t payload_len);

    void set_type(std::uint8_t type);
    void payload_append(std::uint8_t byte);

    void compute_crc();

    [[nodiscard]] bool is_payload_complete(std::uint16_t frame_len) const;
    [[nodiscard]] std::uint16_t size() const;
    [[nodiscard]] MsgType get_type() const;
    [[nodiscard]] std::uint16_t get_payload_len() const;

    [[nodiscard]] std::span<const uint8_t> get_payload() const;
    [[nodiscard]] std::span<const uint8_t> get_full_frame() const;

private:
    std::uint16_t frame_len_;
    std::uint8_t type_;
    std::uint16_t current_index_;
    std::array<std::uint8_t, FRAME_MAX_SIZE> byte_frame_;
};

#include "../frame/frame.hxx"
