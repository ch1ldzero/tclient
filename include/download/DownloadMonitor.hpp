#pragma once

#include <atomic>
#include <chrono>

#include "core/TorrentTask.hpp"

namespace tclient {

class PieceStorage;
class PeerConnector;

class DownloadMonitor {
public:
    DownloadMonitor(
        TorrentTask& task,
        PieceStorage& piece_storage,
        PeerConnector& peer_connector
    );

    void WaitForCompletion();
    void UpdateTask();
    bool IsFinished() const;

private:
    static constexpr auto kStatusUpdateInterval = std::chrono::milliseconds(250);
    static constexpr std::chrono::seconds kRequeueInterval =
        std::chrono::seconds(10);
    static constexpr size_t kEndgameThreshold = 20;

    TorrentTask& task;
    PieceStorage& piece_storage;
    PeerConnector& peer_connector;

    std::atomic<bool> is_finished = false;

private:
    void ManageEndgameMode(
        bool& endgame_mode,
        std::chrono::steady_clock::time_point& last_requeue_time
    );
};

} // namespace tclient

