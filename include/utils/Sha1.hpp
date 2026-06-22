#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace tclient {

class Sha1 {
public:
    static std::array<uint8_t, 20> Compute(const uint8_t* data, size_t length);
    static std::string ComputeHex(std::string_view data);

private:
    static constexpr size_t kBlockSize = 64;
    static constexpr size_t kHashStateSize = 5;
    static constexpr size_t kDigestSize = 20;
    static constexpr size_t kWordScheduleSize = 80;
    static constexpr size_t kChunkWords = 16;
    static constexpr size_t kBytesInHashContextBlock = 56;

    struct HashContext {
        std::array<uint8_t, kBlockSize> message_block;
        uint32_t hash_state[kHashStateSize];
        size_t bytes_in_block = 0;
        uint64_t total_bits_processed = 0;
    };

private:
    static uint32_t RotateLeft(uint32_t value, uint32_t bits);
    static void Reset(HashContext& ctx);
    static void Update(HashContext& ctx, const uint8_t* data, size_t length);
    static void ProcessBlock(HashContext& ctx, const uint8_t* block);
    static void ApplyPadding(HashContext& ctx);
    static std::array<uint8_t, kDigestSize> Final(HashContext& ctx);
};

} // namespace tclient

