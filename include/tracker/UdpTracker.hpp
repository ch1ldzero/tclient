#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "core/TorrentFile.hpp"
#include "net/UdpConnection.hpp"
#include "peer/Peer.hpp"
#include "tracker/ITracker.hpp"

namespace tclient {

class UdpTracker : public ITracker {
public:
    UdpTracker(std::string_view host, int port);

    std::vector<Peer> Announce(
        const TorrentFile& torrent_file,
        const std::string& peer_id,
        uint64_t downloaded,
        uint64_t left,
        uint64_t uploaded,
        int event,
        uint16_t port
    ) override;

    std::string GetUrl() const override;
    uint32_t GetInterval() const override;
    uint32_t GetSeeders() const override;
    uint32_t GetLeechers() const override;

private:
    std::string host;
    int port;
    uint32_t interval = 0;
    uint32_t seeders = 0;
    uint32_t leechers = 0;
    std::unique_ptr<UdpConnection> udp_connection;

    struct TrackerPeer {
        uint32_t ip;
        uint16_t port;
    };

private:
    uint64_t Connect();

    std::vector<TrackerPeer> AnnounceWithConnection(
        uint64_t connection_id,
        const std::string& info_hash,
        const std::string& peer_id,
        uint64_t downloaded,
        uint64_t left,
        uint64_t uploaded,
        int event,
        int num_want,
        uint16_t port
    );

    Peer ConvertTrackerPeer(const TrackerPeer& tracker_peer);
};

} // namespace tclient

