#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>

#include "core/TorrentFile.hpp"
#include "net/TcpConnection.hpp"
#include "peer/Peer.hpp"
#include "storage/Block.hpp"
#include "storage/Piece.hpp"

namespace tclient {

class PieceStorage;

class PeerSession {
public:
    PeerSession(
        const Peer& peer,
        const TorrentFile& torrent_file,
        std::string self_peer_id,
        PieceStorage& piece_storage
    );

    ~PeerSession();

    PeerSession(const PeerSession& other) = delete;
    PeerSession& operator=(const PeerSession& other) = delete;
    PeerSession(PeerSession&& other) = delete;
    PeerSession& operator=(PeerSession&& other) = delete;

    void Start();
    void Stop();
    bool IsTerminated() const;
    bool IsConnected() const;
    std::string GetPeerId() const;
    std::string GetPeerAddress() const;
    Peer GetPeer() const;

private:
    static constexpr size_t kMaxFailures = 5;
    static constexpr size_t kMaxInflightBlocks = 1024;

    class PeerPiecesAvailability {
    public:
        PeerPiecesAvailability() = default;
        PeerPiecesAvailability(std::string bitfield, size_t size);
        bool IsPieceAvailable(size_t index) const;
        void SetPieceAvailable(size_t index);

    private:
        std::string bitfield;
        size_t size = 0;
    };

    TorrentFile torrent_file;
    TcpConnection tcp_connection;
    std::string self_peer_id;
    std::string peer_id;
    PieceStorage& piece_storage;

    PeerPiecesAvailability pieces_availability;

    PiecePtr piece_in_progress;
    std::unordered_set<size_t> inflight_offsets;

    bool is_choked = true;
    std::atomic<bool> is_terminated = false;
    std::atomic<bool> is_connected = false;

    std::unique_ptr<std::thread> worker_thread;

private:
    void RunLoop();
    bool EstablishConnection();
    void MainLoop();
    PiecePtr GetNextAvailablePiece();
    void ProcessMessage(std::string_view data);
    void RequestBlock(const Block* block);
    void HandleError();
    void ReturnCurrentPiece();

    void PerformHandshake();
    void ReceiveBitfield();
    void SendInterested();
};

} // namespace tclient

