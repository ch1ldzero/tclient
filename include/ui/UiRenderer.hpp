#pragma once

#include <string>
#include <vector>

#include "core/TorrentSnapshot.hpp"

namespace tclient {

class UiRenderer {
public:
    void Render(
        const TorrentSnapshot& snapshot,
        const std::vector<std::string>& logs
    ) const;

private:
    static constexpr size_t kScreenWidth = 70;
    static constexpr size_t kLogLines = 10;

private:
    static void ClearScreen();
};

} // namespace tclient

