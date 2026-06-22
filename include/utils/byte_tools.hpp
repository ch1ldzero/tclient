#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace tclient {

std::int32_t bytes_to_int32(std::string_view bytes);
std::string int32_to_bytes(std::int32_t value);

std::uint64_t bytes_to_uint64(std::string_view bytes);
std::string uint64_to_bytes(std::uint64_t value);

std::string bytes_to_hex(std::string_view bytes);
std::string hex_encode(std::string_view input);

} // namespace tclient

