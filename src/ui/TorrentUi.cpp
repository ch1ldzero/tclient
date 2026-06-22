#include "ui/TorrentUi.hpp"

#include <chrono>
#include <thread>

#include "core/TorrentClient.hpp"

namespace tclient {

TorrentUi::TorrentUi(TorrentClient& client) :
    client(client)
{}

TorrentSnapshot TorrentUi::BuildSnapshot() const {
    auto task = client.GetCurrentTask();
    auto elapsed = client.ElapsedTime();
    return TorrentSnapshot::CreateFromTask(task, elapsed);
}

void TorrentUi::Run() {
    using namespace std::chrono_literals;

    while (is_running) {
        auto snapshot = BuildSnapshot();
        auto logs = client.GetLogMessages(30);
        renderer.Render(snapshot, logs);

        if (client.IsFinished()) {
            std::this_thread::sleep_for(1s);

            auto final_snapshot = BuildSnapshot();
            auto final_logs = client.GetLogMessages(30);
            renderer.Render(final_snapshot, final_logs);

            break;
        }

        std::this_thread::sleep_for(250ms);
    }
}

} // namespace tclient

