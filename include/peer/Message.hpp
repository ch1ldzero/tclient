#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace tclient {

enum class MessageId : uint8_t {
    kChoke = 0,
    kUnchoke,
    kInterested,
    kNotInterested,
    kHave,
    kBitField,
    kRequest,
    kPiece,
    kCancel,
    kPort,
    kKeepAlive,
};

struct Message {
    MessageId id;
    size_t message_length;
    std::string payload;

    static Message Parse(std::string_view message_string);
    static Message Init(MessageId id, std::string_view payload);
    std::string ToString() const;
};

} // namespace tclient

