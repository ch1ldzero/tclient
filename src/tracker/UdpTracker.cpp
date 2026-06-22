#include "tracker/UdpTracker.hpp"

#include <cstring>
#include <random>
#include <stdexcept>

#include "utils/byte_tools.hpp"

namespace tclient {

UdpTracker::UdpTracker(std::string_view host, int port) :
    host(host),
    port(port)
{
    udp_connection = std::make_unique<UdpConnection>(host, port);
}

std::string UdpTracker::GetUrl() const {
    return "udp://" + host + ":" + std::to_string(port);
}

uint32_t UdpTracker::GetInterval() const {
    return interval;
}

uint32_t UdpTracker::GetSeeders() const {
    return seeders;
}

uint32_t UdpTracker::GetLeechers() const {
    return leechers;
}

std::vector<Peer> UdpTracker::Announce(
    const TorrentFile& torrent_file,
    const std::string& peer_id,
    uint64_t downloaded,
    uint64_t left,
    uint64_t uploaded,
    int event,
    uint16_t port
) {
    uint64_t connection_id = Connect();
    auto tracker_peers = AnnounceWithConnection(
        connection_id,
        torrent_file.info_hash,
        peer_id,
        downloaded,
        left,
        uploaded,
        event,
        200,
        port
    );

    std::vector<Peer> result;
    result.reserve(tracker_peers.size());
    for (const auto& peer : tracker_peers) {
        result.push_back(ConvertTrackerPeer(peer));
    }
    return result;
}

uint64_t UdpTracker::Connect() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int32_t> dist(0, INT32_MAX);

    uint64_t protocol_id = 0x41727101980;
    uint32_t action = 0;
    uint32_t transaction_id = static_cast<uint32_t>(dist(gen));

    std::string request;
    request.reserve(16);
    request += uint64_to_bytes(protocol_id);
    request += int32_to_bytes(static_cast<int32_t>(action));
    request += int32_to_bytes(static_cast<int32_t>(transaction_id));

    udp_connection->Send(request);
    std::string response = udp_connection->Receive();

    if (response.size() < 16) {
        throw std::runtime_error(
            "CONNECT response too small: " + std::to_string(response.size())
        );
    }

    uint32_t resp_action = static_cast<uint32_t>(
        bytes_to_int32(std::string_view(response).substr(0, 4))
    );
    uint32_t resp_trans = static_cast<uint32_t>(
        bytes_to_int32(std::string_view(response).substr(4, 4))
    );

    if (resp_action != 0) {
        throw std::runtime_error(
            "CONNECT failed: action=" + std::to_string(resp_action)
        );
    }

    if (resp_trans != transaction_id) {
        throw std::runtime_error("CONNECT transaction_id mismatch");
    }

    return bytes_to_uint64(std::string_view(response).substr(8, 8));
}

std::vector<UdpTracker::TrackerPeer> UdpTracker::AnnounceWithConnection(
    uint64_t connection_id,
    const std::string& info_hash,
    const std::string& peer_id,
    uint64_t downloaded,
    uint64_t left,
    uint64_t uploaded,
    int event,
    int num_want,
    uint16_t port
) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int32_t> dist(0, INT32_MAX);
    static std::uniform_int_distribution<int32_t> key_dist;

    if (info_hash.size() != 20) {
        throw std::runtime_error("Info_hash must be 20 bytes");
    }
    if (peer_id.size() != 20) {
        throw std::runtime_error("Peer_id must be 20 bytes");
    }

    uint32_t action = 1;
    uint32_t transaction_id = static_cast<uint32_t>(dist(gen));

    std::string request;
    request.reserve(98);
    request += uint64_to_bytes(connection_id);
    request += int32_to_bytes(static_cast<int32_t>(action));
    request += int32_to_bytes(static_cast<int32_t>(transaction_id));
    request += info_hash;
    request += peer_id;
    request += uint64_to_bytes(downloaded);
    request += uint64_to_bytes(left);
    request += uint64_to_bytes(uploaded);
    request += int32_to_bytes(static_cast<int32_t>(event));
    request += int32_to_bytes(0);
    request += int32_to_bytes(key_dist(gen));
    request += int32_to_bytes(static_cast<int32_t>(num_want));
    request += int32_to_bytes(static_cast<int32_t>(port));

    udp_connection->Send(request);
    std::string response = udp_connection->Receive();

    if (response.size() < 20) {
        throw std::runtime_error(
            "ANNOUNCE response too small: " + std::to_string(response.size())
        );
    }

    uint32_t resp_action = static_cast<uint32_t>(
        bytes_to_int32(std::string_view(response).substr(0, 4))
    );
    uint32_t resp_trans = static_cast<uint32_t>(
        bytes_to_int32(std::string_view(response).substr(4, 4))
    );

    if (resp_action == 3) {
        std::string err_msg = response.substr(8);
        throw std::runtime_error("Tracker error: " + err_msg);
    }

    if (resp_action != 1) {
        throw std::runtime_error(
            "ANNOUNCE failed: wrong action=" + std::to_string(resp_action)
        );
    }

    if (resp_trans != transaction_id) {
        throw std::runtime_error("ANNOUNCE transaction_id mismatch");
    }

    interval = static_cast<uint32_t>(
        bytes_to_int32(std::string_view(response).substr(8, 4))
    );
    leechers = static_cast<uint32_t>(
        bytes_to_int32(std::string_view(response).substr(12, 4))
    );
    seeders = static_cast<uint32_t>(
        bytes_to_int32(std::string_view(response).substr(16, 4))
    );

    std::vector<TrackerPeer> result;
    size_t offset = 20;
    while (offset + 6 <= response.size()) {
        TrackerPeer peer;
        std::string_view resp_view(response);
        peer.ip = static_cast<uint32_t>(
            bytes_to_int32(resp_view.substr(offset, 4))
        );
        peer.port = static_cast<uint16_t>(
            (static_cast<unsigned char>(response[offset + 4]) << 8) |
                static_cast<unsigned char>(response[offset + 5])
        );
        result.push_back(peer);
        offset += 6;
    }

    return result;
}

Peer UdpTracker::ConvertTrackerPeer(const TrackerPeer& tracker_peer) {
    Peer peer;
    peer.ip = std::to_string((tracker_peer.ip >> 24) & 0xFF) +
        "." +
        std::to_string((tracker_peer.ip >> 16) & 0xFF) +
        "." +
        std::to_string((tracker_peer.ip >> 8) & 0xFF) +
        "." +
        std::to_string(tracker_peer.ip & 0xFF);
    peer.port = static_cast<int>(tracker_peer.port);
    return peer;
}

} // namespace tclient

