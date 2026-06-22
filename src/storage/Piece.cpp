#include "storage/Piece.hpp"

#include <algorithm>

#include "utils/Sha1.hpp"
#include "utils/byte_tools.hpp"

namespace tclient {

Piece::Piece(size_t index, size_t length, std::string hash) :
    index(index),
    length(length),
    hash(std::move(hash)),
    bytes_downloaded(0)
{
    size_t offset = 0;
    while (offset < length) {
        size_t block_length = std::min(Block::kSize, length - offset);
        blocks.emplace_back(
            index,
            offset,
            block_length,
            Block::Status::kMissing,
            ""
        );
        offset += block_length;
    }
}

bool Piece::HashMatches() const {
    if (!AllBlocksRetrieved()) {
        return false;
    }
    return Sha1::ComputeHex(GetData()) == bytes_to_hex(hash);
}

Block* Piece::GetFirstMissingBlock() {
    for (auto& block : blocks) {
        if (block.status == Block::Status::kMissing) {
            block.status = Block::Status::kPending;
            return &block;
        }
    }
    return nullptr;
}

size_t Piece::GetIndex() const {
    return index;
}

void Piece::SaveBlock(size_t block_offset, std::string_view data) {
    for (auto& block : blocks) {
        if (block.offset == block_offset) {
            if (block.status != Block::Status::kPending) {
                return;
            }
            block.data = data;
            block.status = Block::Status::kRetrieved;
            bytes_downloaded += block.data.size();
            cached_data.clear();
            return;
        }
    }
}

bool Piece::AllBlocksRetrieved() const {
    for (const auto& block : blocks) {
        if (block.status != Block::Status::kRetrieved) {
            return false;
        }
    }
    return true;
}

const std::string& Piece::GetData() const {
    if (!cached_data.empty()) {
        return cached_data;
    }

    cached_data.reserve(length);
    for (const auto& block : blocks) {
        if (block.status == Block::Status::kRetrieved) {
            cached_data += block.data;
        } else {
            cached_data.append(block.length, '\0');
        }
    }

    return cached_data;
}

std::string Piece::GetDataHash() const {
    return Sha1::ComputeHex(GetData());
}

const std::string& Piece::GetHash() const {
    return hash;
}

void Piece::Reset() {
    bytes_downloaded = 0;
    cached_data.clear();
    for (auto& block : blocks) {
        block.status = Block::Status::kMissing;
        block.data.clear();
        block.data.shrink_to_fit();
    }
}

bool Piece::IsDownloading() const {
    for (const auto& block : blocks) {
        if (block.status == Block::Status::kPending) {
            return true;
        }
    }
    return false;
}

bool Piece::IsComplete() const {
    return AllBlocksRetrieved();
}

size_t Piece::GetLength() const {
    return length;
}

size_t Piece::GetBytesDownloaded() const {
    return bytes_downloaded;
}

} // namespace tclient

