#include "download/PiecePicker.hpp"

#include <stdexcept>

#include "storage/PieceStorage.hpp"

namespace tclient {

PiecePicker::PiecePicker(PieceStorage& piece_storage) :
    piece_storage(piece_storage)
{}

PiecePtr PiecePicker::SelectNextPiece(
    const std::vector<bool>& peer_availability
) {
    if (strategy == Strategy::kSequential) {
        return piece_storage.GetNextPieceToDownload();
    }

    throw std::runtime_error("Rarest-first strategy is not implemented");
}

bool PiecePicker::ShouldEnterEndgameMode() const {
    return piece_storage.GetMissingPieces().size() <= kEndgameThreshold;
}

std::vector<size_t> PiecePicker::GetMissingPieces() const {
    return piece_storage.GetMissingPieces();
}

void PiecePicker::SetStrategy(Strategy new_strategy) {
    strategy = new_strategy;
}

} // namespace tclient

