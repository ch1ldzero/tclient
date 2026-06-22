#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "storage/Block.hpp"

namespace tclient {

class Piece {
public:
    Piece(size_t index, size_t length, std::string hash);

    bool HashMatches() const;
    Block* GetFirstMissingBlock();
    size_t GetIndex() const;
    void SaveBlock(size_t blockOffset, std::string_view data);
    bool AllBlocksRetrieved() const;
    const std::string& GetData() const;
    std::string GetDataHash() const;
    const std::string& GetHash() const;
    void Reset();

    bool IsDownloading() const;
    bool IsComplete() const;
    size_t GetLength() const;
    size_t GetBytesDownloaded() const;

private:
    size_t index;
    size_t length;
    std::string hash;
    std::vector<Block> blocks;
    size_t bytes_downloaded = 0;
    mutable std::string cached_data;
};

using PiecePtr = std::shared_ptr<Piece>;

} // namespace tclient

