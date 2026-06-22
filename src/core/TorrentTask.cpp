#include "core/TorrentTask.hpp"

#include "storage/PieceStorage.hpp"

namespace tclient {

void TorrentTask::SetConnectedPeers(int new_count) {
    connected_peers = new_count;
}

std::string TorrentTask::GetStatusString() const {
    switch (status) {

    case TorrentStatus::kNoTorrent:
        return "No Torrent";

    case TorrentStatus::kLoading:
        return "Loading";

    case TorrentStatus::kDownloading:
        return "Downloading";

    case TorrentStatus::kPaused:
        return "Paused";

    case TorrentStatus::kCompleted:
        return "Completed";

    case TorrentStatus::kError:
        return "Error";

    case TorrentStatus::kConnected:
        return "Connecting";

    default:
        return "Unknown";
    }
}

void TorrentTask::UpdateFromPieceStorage(
    const PieceStorage& storage,
    size_t default_piece_length
) {
    total_pieces_count = storage.TotalPiecesCount();
    downloaded_pieces_count = storage.PiecesSavedToDiskCount();
    missing_pieces = storage.GetMissingPieces();

    if (total_pieces_count > 0) {
        progress = (
            static_cast<double>(downloaded_pieces_count) /
                static_cast<double>(total_pieces_count)
        ) * 100.0;

        downloaded = downloaded_pieces_count * default_piece_length;

        if (downloaded > total_size) {
            if (total_size > 0) {
                downloaded = total_size;
            }
        }
    }
}

} // namespace tclient

