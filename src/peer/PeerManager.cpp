#include "peer/PeerManager.hpp"

namespace tclient {

std::string PeerManager::MakeKey(const Peer& peer) {
    return peer.ip + ":" + std::to_string(peer.port);
}

PeerState PeerManager::GetPeerState(const std::string& key) const {
    auto it = peer_states.find(key);

    if (it != peer_states.end()) {
        return it->second;
    }
    return PeerState::kFailed;
}

void PeerManager::SetPeerState(const std::string& key, PeerState state) {
    peer_states[key] = state;
}

void PeerManager::AddPeers(const std::vector<Peer>& new_peers) {
    std::lock_guard<std::mutex> lock(mutex);

    for (const auto& peer : new_peers) {
        const auto key = MakeKey(peer);
        if (peer_states.find(key) == peer_states.end()) {
            peers.push_back(peer);
            peer_states[key] = PeerState::kDiscovered;
        }
    }
}

std::vector<Peer> PeerManager::TakeNewPeers() {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<Peer> result;
    for (auto& peer : peers) {
        const auto key = MakeKey(peer);
        auto it = peer_states.find(key);

        if (
            it != peer_states.end() &&
            (
                it->second == PeerState::kDiscovered ||
                it->second == PeerState::kFailed
            )
        ) {
            it->second = PeerState::kConnecting;
            result.push_back(peer);
        }
    }

    return result;
}

std::vector<Peer> PeerManager::GetPeersReadyForConnection() {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<Peer> result;
    for (const auto& peer : peers) {
        const auto key = MakeKey(peer);

        auto it = peer_states.find(key);

        if (
            it != peer_states.end() &&
            (
                it->second == PeerState::kDiscovered ||
                it->second == PeerState::kFailed
            )
        ) {
            result.push_back(peer);
        }
    }

    return result;
}

std::vector<Peer> PeerManager::GetAllPeers() const {
    std::lock_guard<std::mutex> lock(mutex);
    return peers;
}

void PeerManager::MarkConnected(const Peer& peer) {
    std::lock_guard<std::mutex> lock(mutex);
    const auto key = MakeKey(peer);

    if (peer_states.find(key) == peer_states.end()) {
        peers.push_back(peer);
    }

    peer_states[key] = PeerState::kConnected;
}

void PeerManager::MarkConnectionFailed(const Peer& peer) {
    std::lock_guard<std::mutex> lock(mutex);

    const auto key = MakeKey(peer);
    auto it = peer_states.find(key);
    if (it != peer_states.end()) {
        if (
            it->second == PeerState::kConnecting ||
            it->second == PeerState::kConnected
        ) {
            it->second = PeerState::kFailed;
        }
    }
}

size_t PeerManager::Count() const {
    std::lock_guard<std::mutex> lock(mutex);
    return peers.size();
}

size_t PeerManager::ConnectedCount() const {
    std::lock_guard<std::mutex> lock(mutex);

    size_t count = 0;
    for (const auto& [key, state] : peer_states) {
        if (state == PeerState::kConnected) {
            ++count;
        }
    }

    return count;
}

size_t PeerManager::AvailableCount() const {
    std::lock_guard<std::mutex> lock(mutex);

    size_t count = 0;
    for (const auto& [key, state] : peer_states) {
        if (state == PeerState::kDiscovered || state == PeerState::kFailed) {
            ++count;
        }
    }

    return count;
}

void PeerManager::Clear() {
    std::lock_guard<std::mutex> lock(mutex);
    peers.clear();
    peer_states.clear();
}

} // namespace tclient

