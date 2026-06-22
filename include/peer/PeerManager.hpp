#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "peer/Peer.hpp"

namespace tclient {

enum class PeerState {
    kDiscovered,
    kConnecting,
    kConnected,
    kFailed
};

class PeerManager {
public:
    void AddPeers(const std::vector<Peer>& peers);
    std::vector<Peer> TakeNewPeers();

    std::vector<Peer> GetPeersReadyForConnection();
    std::vector<Peer> GetAllPeers() const;

    void MarkConnected(const Peer& peer);
    void MarkConnectionFailed(const Peer& peer);

    size_t Count() const;
    size_t ConnectedCount() const;
    size_t AvailableCount() const;
    void Clear();

private:
    mutable std::mutex mutex;
    std::vector<Peer> peers;
    std::unordered_map<std::string, PeerState> peer_states;

private:
    std::string MakeKey(const Peer& peer);

    PeerState GetPeerState(const std::string& key) const;
    void SetPeerState(const std::string& key, PeerState state);
};

} // namespace tclient

