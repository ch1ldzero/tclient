#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "core/TorrentFile.hpp"
#include "net/HttpConnection.hpp"
#include "peer/Peer.hpp"
#include "tracker/ITracker.hpp"

namespace tclient {

class HttpTracker : public ITracker {
public:
    explicit HttpTracker(std::string_view url);

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

private:
    std::string tracker_url;
    std::unique_ptr<HttpConnection> http_connection;
    std::vector<Peer> peers;

private:
    void ParseTrackerResponse(const std::string& response);
    void ParseCompactPeers(const std::string& peers_data);
    void ParseCompactBinaryPeers(const std::string& peers_data);
    void ParseDictionaryPeers(const std::string& peers_data);
};

} // namespace tclient

