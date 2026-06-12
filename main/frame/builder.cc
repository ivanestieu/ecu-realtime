#include <array>
#include <bit>

#if __has_include(<esp_log.h>)
extern "C"
{
#    include <esp_log.h>
}
#else
#    define ESP_LOGI(tag, format, ...) ((void)0)
#endif

#include "builder.hh"

namespace builder
{
    std::array<std::uint8_t, sizeof(float)> float_to_bytes(const float f)
    {
        return std::bit_cast<std::array<std::uint8_t, sizeof(float)>>(f);
    }

    float bytes_to_float(const std::array<std::uint8_t, sizeof(float)> bytes)
    {
        return std::bit_cast<float>(bytes);
    }

    Frame output(const float command)
    {
        Frame frame{ Frame::MsgType::OUTPUT, sizeof(float) };
        for (const auto& byte : float_to_bytes(command))
        {
            frame.payload_append(byte);
        }
        frame.compute_crc();
        ESP_LOGI(__FILE_NAME__, "builder::output: frame built");
        return frame;
    }

    Frame stats(const std::array<std::uint32_t, STATS_SIZE> &stats)
    {
        unsigned int expected_size = sizeof(std::uint32_t) * stats.size();
        if (expected_size >= Frame::FRAME_MAX_SIZE)
        {
            ESP_LOGI(__FILE_NAME__,
                     "builder::stats: trimming payload size to stay underneath "
                     "Frame::FRAME_MAX_SIZE (%d)",
                     Frame::FRAME_MAX_SIZE);
            expected_size = Frame::FRAME_MAX_SIZE;
        }
        Frame frame{ Frame::MsgType::STATS,
                     static_cast<uint16_t>(expected_size) };

        for (const uint32_t& val : stats)
        {
            frame.payload_append((val >> 24) & 0xFF);
            frame.payload_append((val >> 16) & 0xFF);
            frame.payload_append((val >> 8) & 0xFF);
            frame.payload_append(val & 0xFF);
        }
        frame.compute_crc();
        ESP_LOGI(__FILE_NAME__, "builder::stats: frame built");
        return frame;
    }

    static Frame message(const std::string& msg, const Frame::MsgType type)
    {
        unsigned int expected_size = sizeof(char) * msg.length();
        if (expected_size >= Frame::FRAME_MAX_SIZE)
        {
            ESP_LOGI(__FILE_NAME__,
                     "builder::message: trimming payload size to stay "
                     "underneath Frame::FRAME_MAX_SIZE (%d)",
                     Frame::FRAME_MAX_SIZE);
            expected_size = Frame::FRAME_MAX_SIZE;
        }
        Frame frame{ static_cast<std::uint8_t>(type),
                     static_cast<uint16_t>(expected_size) };

        for (const char& c : msg)
        {
            frame.payload_append(c);
        }
        frame.compute_crc();
        ESP_LOGI(__FILE_NAME__, "builder::message: frame built");
        return frame;
    }

    Frame alarm(const std::string& message)
    {
        return builder::message(message, Frame::MsgType::ALARM);
    }
    Frame debug(const std::string& message)
    {
        return builder::message(message, Frame::MsgType::DEBUG);
    }
} // namespace builder