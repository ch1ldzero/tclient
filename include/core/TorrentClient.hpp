#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <vector>

#include "core/TorrentTask.hpp"

namespace tclient {

class TorrentSession;

class TorrentClient {
public:
    explicit TorrentClient(const std::string& peer_id = "TESTAPPDONTWORRY");
    ~TorrentClient();

    void DownloadTorrent(
        const std::filesystem::path& torrent_file_path,
        const std::filesystem::path& output_directory
    );

    const std::string& GetPeerId() const;
    TorrentTask GetCurrentTask() const;
    std::vector<std::string> GetLogMessages(size_t max_count = 50) const;

    bool IsFinished() const;
    std::chrono::seconds ElapsedTime() const;

private:
    static constexpr int kListenPort = 12345;

    std::mt19937 random_engine;
    std::uniform_int_distribution<int> char_dist =
        std::uniform_int_distribution<int>('a', 'z');

    std::string peer_id;

    mutable std::mutex session_mutex;

    std::unique_ptr<TorrentSession> active_session;

    std::atomic<bool> is_terminated = false;

private:
    std::string GenerateRandomSuffix(size_t length = 4);
};

} // namespace tclient

