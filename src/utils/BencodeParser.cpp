#include "utils/BencodeParser.hpp"

#include <fstream>
#include <stdexcept>

#include "utils/Sha1.hpp"

namespace tclient {

std::string BencodeParser::ReadFixedAmount(ParseState& state, size_t amount) {
    if (state.index + amount > state.to_decode.size()) {
        throw std::runtime_error("Not enough data to read fixed amount");
    }

    std::string result = state.to_decode.substr(state.index, amount);
    state.index += amount;
    return result;
}

std::string BencodeParser::ReadUntilDelimiter(ParseState& state, char delimiter) {
    size_t pos = state.to_decode.find(delimiter, state.index);
    if (pos == std::string::npos) {
        throw std::runtime_error("Delimiter not found");
    }

    std::string result = state.to_decode.substr(state.index, pos - state.index);
    state.index = pos + 1;
    return result;
}

std::string BencodeParser::Process(ParseState& state) {
    if (state.index >= state.to_decode.size()) {
        throw std::runtime_error("Unexpected end of input");
    }

    char current_char = state.to_decode[state.index];
    if ('0' <= current_char && current_char <= '9') {
        std::string length_str = ReadUntilDelimiter(state, ':');
        size_t string_length = std::stoull(length_str);
        return ReadFixedAmount(state, string_length);
    }

    if (current_char == 'i') {
        ++state.index;
        return ReadUntilDelimiter(state, 'e');
    }

    if (current_char == 'd') {
        ProcessDict(state);
        return "";
    }

    if (current_char == 'l') {
        ProcessList(state);
        return "";
    }

    throw std::runtime_error(
        "Invalid bencode character: " + std::string(1, current_char)
    );
}

void BencodeParser::ProcessDict(ParseState& state) {
    ++state.index;

    std::string current_key;
    int start_index = -1;
    int end_index = -1;
    bool found_info = false;

    while (
        state.index < state.to_decode.size() &&
        state.to_decode[state.index] != 'e'
    ) {
        if (current_key.empty()) {
            current_key = Process(state);
            if (current_key == "info") {
                start_index = static_cast<int>(state.index);
                found_info = true;
            }
        } else {
            std::string value = Process(state);
            state.info_dict[current_key] = value;

            if (found_info) {
                end_index = static_cast<int>(state.index);
                found_info = false;
            }
            current_key.clear();
        }
    }

    if (state.index >= state.to_decode.size()) {
        throw std::runtime_error("Unexpected end of dictionary");
    }

    ++state.index;

    if (start_index != -1 && end_index != -1) {
        std::string info_section(
            state.to_decode.data() + start_index,
            static_cast<size_t>(end_index) - static_cast<size_t>(start_index)
        );

        auto digest = Sha1::Compute(
            reinterpret_cast<const uint8_t*>(info_section.data()),
            info_section.size()
        );

        state.info_hash = std::string(
            reinterpret_cast<const char*>(digest.data()),
            digest.size()
        );
    }
}

void BencodeParser::ProcessList(ParseState& state) {
    ++state.index;
    while (
        state.index < state.to_decode.size() &&
        state.to_decode[state.index] != 'e'
    ) {
        Process(state);
    }

    if (state.index >= state.to_decode.size()) {
        throw std::runtime_error("Unexpected end of list");
    }

    ++state.index;
}

void BencodeParser::ExtractPieceHashes(ParseState& state) {
    auto it = state.info_dict.find("pieces");
    if (it != state.info_dict.end()) {
        const std::string& pieces_data = it->second;
        static constexpr size_t kHashSize = 20;
        for (
            size_t j = 0;
            j + kHashSize <= pieces_data.size();
            j += kHashSize
        ) {
            state.pieces_hashes.push_back(pieces_data.substr(j, kHashSize));
        }
    }
}

BencodeParseResult BencodeParser::ParseFile(const std::string& filename) {
    std::ifstream input_file(filename, std::ios::binary);
    if (!input_file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    input_file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(input_file.tellg());
    input_file.seekg(0, std::ios::beg);

    ParseState state;
    state.to_decode.resize(size);
    input_file.read(state.to_decode.data(), static_cast<std::streamsize>(size));

    Process(state);
    ExtractPieceHashes(state);

    BencodeParseResult result;
    result.info_dict = std::move(state.info_dict);
    result.info_hash = std::move(state.info_hash);
    result.piece_hashes = std::move(state.pieces_hashes);
    return result;
}

BencodeParseResult BencodeParser::ParseString(std::string_view string) {
    ParseState state;
    state.to_decode = string;
    Process(state);
    ExtractPieceHashes(state);

    BencodeParseResult result;
    result.info_dict = std::move(state.info_dict);
    result.info_hash = std::move(state.info_hash);
    result.piece_hashes = std::move(state.pieces_hashes);
    return result;
}

} // namespace tclient

