#include "peer/Message.hpp"

#include <stdexcept>

#include "utils/byte_tools.hpp"

namespace tclient {

Message Message::Parse(std::string_view message_string) {
    if (message_string.size() < 4) {
        throw std::runtime_error("Message too short to parse");
    }

    size_t length = static_cast<size_t>(
        bytes_to_int32(message_string.substr(0, 4))
    );

    if (length == 0) {
        return {MessageId::kKeepAlive, 0, ""};
    }

    if (message_string.size() < 5) {
        throw std::runtime_error("Message too short for ID");
    }

    uint8_t id = static_cast<uint8_t>(message_string[4]);
    std::string payload;
    if (message_string.size() > 5) {
        payload = message_string.substr(5);
    }

    return {static_cast<MessageId>(id), length, std::move(payload)};
}

Message Message::Init(MessageId id, std::string_view payload) {
    return {id, payload.size() + 1, std::string(payload)};
}

std::string Message::ToString() const {
    std::string result;
    result.reserve(4 + 1 + payload.size());
    result += int32_to_bytes(static_cast<int32_t>(message_length));
    result += static_cast<char>(static_cast<uint8_t>(id));
    result += payload;
    return result;
}

} // namespace tclient

