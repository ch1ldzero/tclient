#include "core/TorrentFile.hpp"

#include "utils/BencodeParser.hpp"

namespace tclient {

TorrentFile TorrentFile::Load(const std::string& filename) {
    TorrentFile result;

    auto parse_result = BencodeParser::ParseFile(filename);
    const auto& dict = parse_result.info_dict;

    if (auto it = dict.find("announce"); it != dict.end()) {
        result.announce = it->second;
    }
    if (auto it = dict.find("comment"); it != dict.end()) {
        result.comment = it->second;
    }
    if (auto it = dict.find("piece length"); it != dict.end()) {
        result.piece_length = static_cast<size_t>(std::stoull(it->second));
    }
    if (auto it = dict.find("length"); it != dict.end()) {
        result.length = static_cast<size_t>(std::stoull(it->second));
    }
    if (auto it = dict.find("name"); it != dict.end()) {
        result.name = it->second;
    }

    result.info_hash = parse_result.info_hash;
    result.piece_hashes = parse_result.piece_hashes;
    return result;
}

} // namespace tclient

