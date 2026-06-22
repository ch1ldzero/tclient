#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace tclient {

enum class TorrentState {
    kNoTorrent,
    kLoading,
    kDownloading,
    kPaused,
    kCompleted,
    kStopped,
    kError,
    kConnected
};

struct TorrentTask;

struct TorrentSnapshot {
    std::string filename;
    std::string info_hash;
    std::string output_file_path;

    TorrentState state = TorrentState::kNoTorrent;
    std::string state_string;

    double progress = 0.0;
    uint64_t total_size = 0;
    uint64_t downloaded = 0;
    uint64_t uploaded = 0;

    int connected_peers = 0;
    int total_peers = 0;

    size_t total_pieces = 0;
    size_t downloaded_pieces = 0;

    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point last_update;
    std::chrono::seconds elapsed_time = std::chrono::seconds(0);

    bool is_finished = false;
    bool has_error = false;
    std::string error_message;

    static TorrentSnapshot CreateFromTask(
        const TorrentTask& task,
        std::chrono::seconds elapsed
    );
};

} // namespace tclient

