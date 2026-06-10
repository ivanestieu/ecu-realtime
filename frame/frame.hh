#pragma once
#include <vector>

class Frame
{
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

    static constexpr std::uint8_t START_BYTE = 0xAA;
    Frame() = delete;
    Frame(std::uint8_t payload_type, std::uint16_t payload_len);
    void set_type(std::uint8_t type);
    void payload_append(std::uint8_t byte);
    bool is_payload_complete(std::uint16_t frame_len) const;
    void compute_crc();
    MsgType get_type() const;
    std::uint16_t get_payload_len() const;
    std::vector<std::uint8_t> get_payload() const;
    const std::vector<uint8_t>& get_full_frame() const;

private:
    static constexpr unsigned int HEADER_SIZE = 4;
    static constexpr unsigned int CRC_SIZE = 1;
    std::uint16_t frame_len_;
    std::uint8_t type_;
    std::vector<std::uint8_t> byte_frame_;
};

#include "frame.hxx"