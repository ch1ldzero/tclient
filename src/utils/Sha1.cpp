#include "utils/Sha1.hpp"

#include "utils/byte_tools.hpp"

namespace tclient {

std::array<uint8_t, 20> Sha1::Compute(const uint8_t* data, size_t length) {
    HashContext ctx;
    Reset(ctx);
    Update(ctx, data, length);
    return Final(ctx);
}

std::string Sha1::ComputeHex(std::string_view data) {
    auto digest = Compute(
        reinterpret_cast<const uint8_t*>(data.data()),
        data.size()
    );

    std::string raw(
        reinterpret_cast<const char*>(digest.data()),
        digest.size()
    );

    return bytes_to_hex(raw);
}

uint32_t Sha1::RotateLeft(uint32_t value, uint32_t bits) {
    return (value << bits) | (value >> (32 - bits));
}

void Sha1::Reset(HashContext& context) {
    context.hash_state[0] = 0x67452301;
    context.hash_state[1] = 0xEFCDAB89;
    context.hash_state[2] = 0x98BADCFE;
    context.hash_state[3] = 0x10325476;
    context.hash_state[4] = 0xC3D2E1F0;
    context.bytes_in_block = 0;
    context.total_bits_processed = 0;
}

void Sha1::Update(HashContext& ctx, const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        ctx.message_block[ctx.bytes_in_block] = data[i];
        ++ctx.bytes_in_block;
        ctx.total_bits_processed += 8;

        if (ctx.bytes_in_block == kBlockSize) {
            ProcessBlock(ctx, ctx.message_block.data());
            ctx.bytes_in_block = 0;
        }
    }
}

void Sha1::ProcessBlock(HashContext& ctx, const uint8_t* block) {
    uint32_t word_schedule[kWordScheduleSize];

    for (size_t i = 0; i < kChunkWords; ++i) {
        word_schedule[i] = (static_cast<uint32_t>(block[(i << 2) + 0]) << 24) |
            (static_cast<uint32_t>(block[(i << 2) + 1]) << 16) |
            (static_cast<uint32_t>(block[(i << 2) + 2]) << 8) |
            (static_cast<uint32_t>(block[(i << 2) + 3]));
    }

    for (size_t i = kChunkWords; i < kWordScheduleSize; ++i) {
        word_schedule[i] = RotateLeft(
            word_schedule[i - 3] ^ word_schedule[i - 8] ^
                word_schedule[i - 14] ^ word_schedule[i - 16],
            1
        );
    }

    uint32_t a = ctx.hash_state[0];
    uint32_t b = ctx.hash_state[1];
    uint32_t c = ctx.hash_state[2];
    uint32_t d = ctx.hash_state[3];
    uint32_t e = ctx.hash_state[4];

    for (size_t i = 0; i < kWordScheduleSize; ++i) {
        uint32_t function_value;
        uint32_t round_constant;

        if (i < 20) {
            function_value = (b & c) | (~b & d);
            round_constant = 0x5A827999;
        } else if (i < 40) {
            function_value = b ^ c ^ d;
            round_constant = 0x6ED9EBA1;
        } else if (i < 60) {
            function_value = (b & c) | (b & d) | (c & d);
            round_constant = 0x8F1BBCDC;
        } else {
            function_value = b ^ c ^ d;
            round_constant = 0xCA62C1D6;
        }

        uint32_t tmp = RotateLeft(a, 5) +
            function_value +
            e +
            round_constant +
            word_schedule[i];

        e = d;
        d = c;
        c = RotateLeft(b, 30);
        b = a;
        a = tmp;
    }

    ctx.hash_state[0] += a;
    ctx.hash_state[1] += b;
    ctx.hash_state[2] += c;
    ctx.hash_state[3] += d;
    ctx.hash_state[4] += e;
}

void Sha1::ApplyPadding(HashContext& ctx) {
    uint64_t original_bit_length = ctx.total_bits_processed;

    ctx.message_block[ctx.bytes_in_block] = 0x80;
    ++ctx.bytes_in_block;

    if (ctx.bytes_in_block > kBytesInHashContextBlock) {
        while (ctx.bytes_in_block < kBlockSize) {
            ctx.message_block[ctx.bytes_in_block] = 0;
            ++ctx.bytes_in_block;
        }
        ProcessBlock(ctx, ctx.message_block.data());
        ctx.bytes_in_block = 0;
    }

    while (ctx.bytes_in_block < kBytesInHashContextBlock) {
        ctx.message_block[ctx.bytes_in_block] = 0;
        ++ctx.bytes_in_block;
    }

    for (size_t i = 7; i != static_cast<size_t>(-1); --i) {
        ctx.message_block[ctx.bytes_in_block] = static_cast<uint8_t>(
            (original_bit_length >> (i * 8)) & 0xFF
        );
        ++ctx.bytes_in_block;
    }

    ProcessBlock(ctx, ctx.message_block.data());
}

std::array<uint8_t, Sha1::kDigestSize> Sha1::Final(HashContext& ctx) {
    ApplyPadding(ctx);
    std::array<uint8_t, kDigestSize> digest;

    for (size_t i = 0; i < kHashStateSize; ++i) {
        digest[(i << 2) + 0] =
            static_cast<uint8_t>((ctx.hash_state[i] >> 24) & 0xFF);
        digest[(i << 2) + 1] =
            static_cast<uint8_t>((ctx.hash_state[i] >> 16) & 0xFF);
        digest[(i << 2) + 2] =
            static_cast<uint8_t>((ctx.hash_state[i] >> 8) & 0xFF);
        digest[(i << 2) + 3] =
            static_cast<uint8_t>(ctx.hash_state[i] & 0xFF);
    }

    return digest;
}

} // namespace tclient

