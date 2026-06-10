#pragma once

#include <iostream>

#include "frame.hh"

inline Frame::Frame(const std::uint8_t payload_type, const std::uint16_t payload_len)
    : frame_len_(HEADER_SIZE + payload_len + CRC_SIZE), type_(payload_type)
{
    byte_frame_.reserve(frame_len_);
    byte_frame_.push_back(START_BYTE);
    byte_frame_.push_back((payload_len + CRC_SIZE) & 0xFF);
    byte_frame_.push_back(((payload_len + CRC_SIZE) >> 8) & 0xFF);
    byte_frame_.push_back(payload_type);
}

inline void Frame::set_type(const std::uint8_t type)
{
    type_ = type;
}

inline void Frame::payload_append(const std::uint8_t byte)
{
    byte_frame_.push_back(byte);
}

inline bool Frame::is_payload_complete(const std::uint16_t frame_len) const
{
    return byte_frame_.size() - HEADER_SIZE == frame_len - CRC_SIZE;
}

inline void Frame::compute_crc()
{
    std::uint8_t crc = 0;
    for (size_t i = 1; i < byte_frame_.size(); ++i)
    {
        crc ^= byte_frame_[i];
    }
    byte_frame_.push_back(crc);
}

inline Frame::MsgType Frame::get_type() const
{
    return static_cast<MsgType>(type_);
}

inline std::uint16_t Frame::get_payload_len() const
{
    return static_cast<uint16_t>(byte_frame_.size() - HEADER_SIZE - CRC_SIZE);
}

inline std::vector<std::uint8_t> Frame::get_payload() const
{
    return std::vector<std::uint8_t>{byte_frame_.begin() + HEADER_SIZE,
                                     byte_frame_.end() - CRC_SIZE};
}

inline const std::vector<uint8_t>& Frame::get_full_frame() const
{
    return byte_frame_;
}
