#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "core/TorrentFile.hpp"
#include "peer/Peer.hpp"
#include "peer/PeerManager.hpp"

namespace tclient {

class PieceStorage;
class PeerSession;

class PeerConnector {
    friend class DownloadMonitor;

public:
    PeerConnector(
        std::string_view self_peer_id,
        PeerManager& peer_manager,
        PieceStorage& piece_storage
    );

    ~PeerConnector();

    PeerConnector(const PeerConnector&) = delete;
    PeerConnector& operator=(const PeerConnector&) = delete;

    void StartDiscovery(const TorrentFile& torrent_file);
    void Stop();
    bool ConnectToPeer(const Peer& peer, const TorrentFile& torrent_file);
    size_t ActiveConnectionCount() const;
    size_t GetMaxConnections() const;

    std::vector<std::string> GetActivePeerIds() const;

private:
    static constexpr std::chrono::seconds kPeerDiscoveryInterval =
        std::chrono::seconds(2);

    static constexpr size_t kMaxConnections = 70;

    std::string self_peer_id;
    PeerManager& peer_manager;
    PieceStorage& piece_storage;

    mutable std::mutex sessions_mutex;
    std::vector<std::shared_ptr<PeerSession>> sessions;

    std::atomic<bool> is_stopped = false;
    std::thread discovery_thread;

private:
    void DiscoveryLoop(TorrentFile torrent_file);
    void CleanupTerminatedConnections();
};

} // namespace tclient

