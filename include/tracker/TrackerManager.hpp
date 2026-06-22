#pragma once

#include <array>
#include <atomic>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "core/TorrentFile.hpp"
#include "peer/PeerManager.hpp"

namespace tclient {

class PeerConnector;

class TrackerManager {
public:
    TrackerManager(
        std::string_view self_peer_id,
        int listen_port,
        PeerManager& peer_manager
    );

    ~TrackerManager();

    void SetPeerConnector(PeerConnector* connector);
    bool FetchInitialPeers(const TorrentFile& torrent_file);
    void StartBackgroundUpdates(const TorrentFile& torrent_file);
    void Stop();
    size_t TrackerCount() const;

private:
    static constexpr std::array kDefaultTrackers = {
        "udp://exodus.desync.com:6969/announce",
        "udp://explodie.org:6969/announce",
        "udp://open.stealth.si:80/announce",
        "udp://opentracker.io:6969/announce",
        "udp://tracker-udp.gbitt.info:80/announce"
        "udp://tracker.bittor.pw:1337/announce",
        "udp://tracker.dler.org:6969/announce",
        "udp://tracker.dump.cl:6969/announce",
        "udp://tracker.filemail.com:6969/announce",
        "udp://tracker.internetwarriors.net:1337/announce",
        "udp://tracker.leechers-paradise.org:6969/announce",
        "udp://tracker.moeking.me:6969/announce",
        "udp://tracker.openbittorrent.com:80/announce",
        "udp://tracker.opentrackr.org:1337/announce",
        "udp://tracker.torrent.eu.org:451/announce",
        "udp://tracker.tryhackx.org:6969/announce",
        "udp://tracker.uw0.xyz:6969/announce",
    };

    static constexpr std::chrono::seconds kUpdateInterval =
        std::chrono::seconds(15);

    std::string self_peer_id;
    int listen_port;
    PeerManager& peer_manager;
    PeerConnector* peer_connector;

    std::vector<std::string> tracker_urls;
    std::atomic<bool> is_stopped = false;
    std::thread background_thread;

private:
    std::vector<std::string> CollectTrackerUrls(const TorrentFile& torrent_file);

    bool TryAnnounceToTracker(
        std::string_view url,
        const TorrentFile& torrent_file
    );

    void BackgroundUpdateLoop(TorrentFile torrent_file);
};

} // namespace tclient

