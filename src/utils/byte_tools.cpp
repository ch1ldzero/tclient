#include "utils/byte_tools.hpp"

#include <stdexcept>

namespace tclient {

std::int32_t bytes_to_int32(std::string_view bytes) {
    if (bytes.size() < 4) {
        throw std::runtime_error("BytesToInt32: not enough bytes");
    }

    return 
        (static_cast<std::uint32_t>(
            static_cast<unsigned char>(bytes[0])
        ) << 24) |
        (static_cast<std::uint32_t>(
            static_cast<unsigned char>(bytes[1])
        ) << 16) |
        (static_cast<std::uint32_t>(
            static_cast<unsigned char>(bytes[2])
        ) << 8) |
        static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[3]));
}

std::string int32_to_bytes(std::int32_t value) {
    std::string result(4, '\0');
    result[0] = static_cast<char>((value >> 24) & 0xFF);
    result[1] = static_cast<char>((value >> 16) & 0xFF);
    result[2] = static_cast<char>((value >> 8) & 0xFF);
    result[3] = static_cast<char>(value & 0xFF);
    return result;
}

std::uint64_t bytes_to_uint64(std::string_view bytes) {
    if (bytes.size() < 8) {
        throw std::runtime_error("BytesToInt64: not enough bytes");
    }

    uint64_t result = 0;
    for (size_t i = 0; i < 8; ++i) {
        result = (result << 8) | static_cast<unsigned char>(bytes[i]);
    }
    return result;
}

std::string uint64_to_bytes(std::uint64_t value) {
    std::string result(8, '\0');
    for (size_t i = 7; i != static_cast<size_t>(-1); --i) {
        result[i] = static_cast<unsigned char>(value & 0xFF);
        value >>= 8;
    }
    return result;
}

std::string bytes_to_hex(std::string_view bytes) {
    return hex_encode(bytes);
}

std::string hex_encode(std::string_view input) {
    static constexpr char hex_table[] = "0123456789abcdef";

    std::string result;
    result.resize(input.size() << 1);

    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(input[i]);
        result[(i << 1)] = hex_table[ch >> 4];
        result[(i << 1) | 1] = hex_table[ch & 0x0F];
    }

    return result;
}

} // namespace tclient

