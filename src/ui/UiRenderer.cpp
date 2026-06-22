#include "ui/UiRenderer.hpp"

#include <iostream>

#include "ui/UiFormatter.hpp"

namespace tclient {

void UiRenderer::ClearScreen() {
    std::cout << "\033[2J\033[H";
}

void UiRenderer::Render(
    const TorrentSnapshot& snapshot,
    const std::vector<std::string>& logs
) const {
    ClearScreen();

    std::cout
        << UiFormatter::Center("== TCLIENT ==", kScreenWidth)
        << "\n\n";

    std::cout
        << UiFormatter::Center(
            UiFormatter::Trim(snapshot.filename, 60), kScreenWidth
        )
        << "\n\n";

    std::cout
        << UiFormatter::Center(
            UiFormatter::BuildProgressBar(snapshot.progress), kScreenWidth
        )
        << "\n\n";

    std::cout
        << UiFormatter::Center(
            std::to_string(
                static_cast<int>(snapshot.progress)
            ) + "%",
            kScreenWidth
        )
        << '\n';

    std::cout
        << UiFormatter::Center(
            UiFormatter::FormatBytes(snapshot.downloaded) +
                " / " +
                UiFormatter::FormatBytes(snapshot.total_size),
            kScreenWidth
        )
        << '\n';

    std::cout << UiFormatter::Center(
        std::to_string(snapshot.downloaded_pieces) +
            " / " +
            std::to_string(snapshot.total_pieces) +
            " pieces",
        kScreenWidth
    ) << "\n\n";

    std::cout
        << UiFormatter::Center("[STATUS] " + snapshot.state_string, kScreenWidth)
        << '\n';

    std::cout
        << UiFormatter::Center(
            "[ELAPSED] " +
                UiFormatter::FormatDuration(snapshot.elapsed_time), kScreenWidth
        )
        << '\n';

    std::cout
        << UiFormatter::Center(
            "[PEERS] " +
                std::to_string(snapshot.connected_peers) +
                " connected / " +
                std::to_string(snapshot.total_peers) + " known",
            kScreenWidth
        )
        << "\n\n";

    std::cout << UiFormatter::Center("[LOG]", kScreenWidth) << "\n";

    size_t begin;
    if (logs.size() > kLogLines) {
        begin = logs.size() - kLogLines;
    } else {
        begin = 0;
    }

    for (size_t i = begin; i < logs.size(); ++i) {
        std::cout << UiFormatter::Trim(logs[i], kScreenWidth) << '\n';
    }

    std::cout.flush();
}

} // namespace tclient

