#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace tclient {

enum class TorrentStatus {
    kNoTorrent,
    kLoading,
    kDownloading,
    kPaused,
    kCompleted,
    kStopped,
    kError,
    kConnected
};

class PieceStorage;

struct TorrentTask {
    std::string filename;
    TorrentStatus status = TorrentStatus::kNoTorrent;
    double progress = 0.0;

    uint64_t total_size = 0;
    uint64_t downloaded = 0;
    uint64_t uploaded = 0;

    int connected_peers = 0;
    int total_peers_count = 0;

    std::string info_hash;
    std::string announce_url;
    std::string output_file_path;

    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point last_update;

    std::vector<size_t> missing_pieces;
    size_t total_pieces_count = 0;
    size_t downloaded_pieces_count = 0;

    void SetConnectedPeers(int new_count);
    std::string GetStatusString() const;

    void UpdateFromPieceStorage(
        const PieceStorage& storage,
        size_t default_piece_length
    );
};

} // namespace tclient

