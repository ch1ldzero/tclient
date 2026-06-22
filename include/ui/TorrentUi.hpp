#pragma once

#include <atomic>

#include "core/TorrentSnapshot.hpp"
#include "ui/UiRenderer.hpp"

namespace tclient {

class TorrentClient;

class TorrentUi {
public:
    explicit TorrentUi(TorrentClient& client);
    void Run();

private:
    TorrentClient& client;
    UiRenderer renderer;
    std::atomic<bool> is_running = true;

private:
    TorrentSnapshot BuildSnapshot() const;
};

} // namespace tclient

