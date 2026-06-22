#pragma once

#include <string>
#include <vector>

namespace tclient {

struct TorrentFile {
    std::string announce;
    std::string comment;
    std::vector<std::string> piece_hashes;
    size_t piece_length = 0;
    size_t length = 0;
    std::string name;
    std::string info_hash;

    static TorrentFile Load(const std::string& filename);
};

} // namespace tclient

