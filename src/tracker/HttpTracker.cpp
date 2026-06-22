#include "tracker/HttpTracker.hpp"

#include <map>
#include <stdexcept>

#include "utils/BencodeParser.hpp"

namespace tclient {

HttpTracker::HttpTracker(std::string_view url) :
    tracker_url(url),
    http_connection(std::make_unique<HttpConnection>())
{}

std::string HttpTracker::GetUrl() const {
    return tracker_url;
}

std::vector<Peer> HttpTracker::Announce(
    const TorrentFile& torrent_file,
    const std::string& peer_id,
    uint64_t downloaded,
    uint64_t left,
    uint64_t uploaded,
    int event,
    uint16_t port
) {
    peers.clear();

    std::map<std::string, std::string> params = {
        {"peer_id", peer_id},
        {"port", std::to_string(port)},
        {"uploaded", std::to_string(uploaded)},
        {"downloaded", std::to_string(downloaded)},
        {"left", std::to_string(left)},
        {"compact", "1"}
    };

    params["info_hash"] = torrent_file.info_hash;

    if (event == 2) {
        params["event"] = "started";
    } else if (event == 1) {
        params["event"] = "completed";
    } else if (event == 3) {
        params["event"] = "stopped";
    }

    std::string response = http_connection->Get(tracker_url, params);
    ParseTrackerResponse(response);
    return peers;
}

void HttpTracker::ParseTrackerResponse(const std::string& response) {
    auto parse_result = BencodeParser::ParseString(response);
    const auto& dict = parse_result.info_dict;

    std::string failure_reason;
    if (auto it = dict.find("failure reason"); it != dict.end()) {
        failure_reason = it->second;
    }

    if (!failure_reason.empty()) {
        throw std::runtime_error("Tracker failure: " + failure_reason);
    }

    std::string peers_data;
    if (auto it = dict.find("peers"); it != dict.end()) {
        peers_data = it->second;
    }

    if (peers_data.empty()) {
        throw std::runtime_error("No peers data in tracker response");
    }

    ParseCompactPeers(peers_data);
}

void HttpTracker::ParseCompactPeers(const std::string& peers_data) {
    peers.clear();

    bool is_binary = (peers_data.size() % 6 == 0 && !peers_data.empty());
    if (is_binary) {
        ParseCompactBinaryPeers(peers_data);
    } else {
        ParseDictionaryPeers(peers_data);
    }
}

void HttpTracker::ParseCompactBinaryPeers(const std::string& peers_data) {
    static constexpr size_t kPeerSize = 6;
    const size_t peer_count = peers_data.size() / kPeerSize;
    peers.reserve(peer_count);

    for (size_t i = 0; i < peers_data.size(); i += kPeerSize) {
        std::string ip =
            std::to_string(static_cast<uint8_t>(peers_data[i])) +
            "." +
            std::to_string(static_cast<uint8_t>(peers_data[i + 1])) +
            "." +
            std::to_string(static_cast<uint8_t>(peers_data[i + 2])) +
            "." +
            std::to_string(static_cast<uint8_t>(peers_data[i + 3]));

        int port = (
                static_cast<uint8_t>(peers_data[i + 4]) << 8
            ) |
            static_cast<uint8_t>(peers_data[i + 5]);

        peers.emplace_back(Peer{ip, port});
    }
}

void HttpTracker::ParseDictionaryPeers(const std::string& peers_data) {
    static_cast<void>(peers_data);
    throw std::runtime_error(
        "Non-compact peer format not supported. Use compact=1 in tracker request."
    );
}

} // namespace tclient

