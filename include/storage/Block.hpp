#pragma once

#include <string>

namespace tclient {

struct Block {
    static constexpr size_t kSize = 1 << 14; // 16KB

    enum class Status {
        kMissing = 0,
        kPending,
        kRetrieved,
    };

    size_t piece;
    size_t offset;
    size_t length;
    Status status = Status::kMissing;
    std::string data;
};

} // namespace tclient

