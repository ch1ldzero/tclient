#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace tclient {

struct BencodeParseResult {
    std::unordered_map<std::string, std::string> info_dict;
    std::string info_hash;
    std::vector<std::string> piece_hashes;
};

class BencodeParser {
public:
    static BencodeParseResult ParseFile(const std::string& filename);
    static BencodeParseResult ParseString(std::string_view string);

private:
    struct ParseState {
        std::string to_decode;
        std::string info_hash;
        std::unordered_map<std::string, std::string> info_dict;
        std::vector<std::string> pieces_hashes;
        size_t index = 0;
    };

private:
    static std::string ReadFixedAmount(ParseState& state, size_t amount);
    static std::string ReadUntilDelimiter(ParseState& state, char delimiter);

    static std::string Process(ParseState& state);
    static void ProcessDict(ParseState& state);
    static void ProcessList(ParseState& state);

    static void ExtractPieceHashes(ParseState& state);
};

} // namespace tclient

