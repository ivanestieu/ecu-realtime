#pragma once

#include <array>

#include "parser.hh"

namespace builder
{
    Frame output(float command);
    Frame stats(const std::vector<std::uint32_t>& stats);
    Frame alarm(const std::string& message);
    Frame debug(const std::string& message);
    std::array<std::uint8_t, sizeof(float)> float_to_bytes(float f);
    float bytes_to_float(std::array<std::uint8_t, sizeof(float)> bytes);
} // namespace builder
