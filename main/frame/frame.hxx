#pragma once

#include <algorithm>
#include <iostream>

#include "frame.hh"

inline Frame::Frame(const std::uint8_t payload_type,
                    const std::uint16_t payload_len)
    : frame_len_(HEADER_SIZE + payload_len + CRC_SIZE)
    , type_{ payload_type }
    , current_index_{ 0 }
    , byte_frame_{}
{
    byte_frame_[current_index_++] = START_BYTE;
    byte_frame_[current_index_++] = (payload_len + CRC_SIZE) & 0xFF;
    byte_frame_[current_index_++] = ((payload_len + CRC_SIZE) >> 8) & 0xFF;
    byte_frame_[current_index_++] = payload_type;
}

inline void Frame::set_type(const std::uint8_t type)
{
    type_ = type;
}

inline void Frame::payload_append(const std::uint8_t byte)
{
    byte_frame_[current_index_++] = byte;
}

inline bool Frame::is_payload_complete(const std::uint16_t frame_len) const
{
    return this->size() - HEADER_SIZE == frame_len - CRC_SIZE;
}

inline void Frame::compute_crc()
{
    std::uint8_t crc = 0;
    for (size_t i = 1; i < this->size(); ++i)
    {
        crc ^= byte_frame_[i];
    }
    byte_frame_[current_index_++] = crc;
}

inline std::uint16_t Frame::size() const
{
    return current_index_;
}

inline Frame::MsgType Frame::get_type() const
{
    return static_cast<MsgType>(type_);
}

inline std::uint16_t Frame::get_payload_len() const
{
    return this->size() - HEADER_SIZE - CRC_SIZE;
}

inline std::span<const uint8_t> Frame::get_payload() const
{
    return std::span<const uint8_t>{ byte_frame_.cbegin() + HEADER_SIZE,
                                     this->get_payload_len() };
}

inline std::span<const uint8_t> Frame::get_full_frame() const
{
    return std::span<const uint8_t>{ byte_frame_.cbegin(), this->size() };
}
