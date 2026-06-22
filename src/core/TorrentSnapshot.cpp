#include "core/TorrentSnapshot.hpp"

#include "core/TorrentTask.hpp"

namespace tclient {

TorrentSnapshot TorrentSnapshot::CreateFromTask(
    const TorrentTask& task,
    std::chrono::seconds elapsed
) {
    TorrentSnapshot snapshot;

    snapshot.filename = task.filename;
    snapshot.info_hash = task.info_hash;
    snapshot.output_file_path = task.output_file_path;

    switch (task.status) {

    case TorrentStatus::kNoTorrent: {
        snapshot.state = TorrentState::kNoTorrent;
        break;
    }

    case TorrentStatus::kLoading: {
        snapshot.state = TorrentState::kLoading;
        break;
    }

    case TorrentStatus::kDownloading: {
        snapshot.state = TorrentState::kDownloading;
        break;
    }

    case TorrentStatus::kPaused: {
        snapshot.state = TorrentState::kPaused;
        break;
    }

    case TorrentStatus::kCompleted: {
        snapshot.state = TorrentState::kCompleted;
        break;
    }

    case TorrentStatus::kStopped: {
        snapshot.state = TorrentState::kStopped;
        break;
    }

    case TorrentStatus::kError: {
        snapshot.state = TorrentState::kError;
        break;
    }

    case TorrentStatus::kConnected: {
        snapshot.state = TorrentState::kConnected;
        break;
    }

    }

    snapshot.state_string = task.GetStatusString();

    snapshot.progress = task.progress;
    snapshot.total_size = task.total_size;
    snapshot.downloaded = task.downloaded;
    snapshot.uploaded = task.uploaded;

    snapshot.connected_peers = task.connected_peers;
    snapshot.total_peers = task.total_peers_count;

    snapshot.total_pieces = task.total_pieces_count;
    snapshot.downloaded_pieces = task.downloaded_pieces_count;

    snapshot.start_time = task.start_time;
    snapshot.last_update = task.last_update;
    snapshot.elapsed_time = elapsed;

    snapshot.is_finished = (
        task.status == TorrentStatus::kCompleted ||
        task.status == TorrentStatus::kError ||
        task.status == TorrentStatus::kStopped
    );

    snapshot.has_error = task.status == TorrentStatus::kError;

    return snapshot;
}

} // namespace tclient

