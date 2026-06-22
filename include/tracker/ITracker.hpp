#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "peer/Peer.hpp"

namespace tclient {

class TorrentFile;

class ITracker {
public:
    virtual ~ITracker() = default;

    virtual std::vector<Peer> Announce(
        const TorrentFile& torrent_file,
        const std::string& peer_id,
        uint64_t downloaded,
        uint64_t left,
        uint64_t uploaded,
        int event,
        uint16_t port
    ) = 0;

    virtual std::string GetUrl() const = 0;
    virtual uint32_t GetInterval() const;
    virtual uint32_t GetSeeders() const;
    virtual uint32_t GetLeechers() const;
};

} // namespace tclient

