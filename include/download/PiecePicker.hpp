#pragma once

#include <vector>

#include "storage/Piece.hpp"

namespace tclient {

class PieceStorage;

class PiecePicker {
public:
    enum class Strategy {
        kSequential,
        kRarestFirst
    };

    explicit PiecePicker(PieceStorage& piece_storage);

    PiecePtr SelectNextPiece(const std::vector<bool>& peer_availability);
    bool ShouldEnterEndgameMode() const;
    std::vector<size_t> GetMissingPieces() const;

    void SetStrategy(Strategy strategy);

private:
    static constexpr size_t kEndgameThreshold = 100;

    PieceStorage& piece_storage;
    Strategy strategy = Strategy::kSequential;
};

} // namespace tclient

