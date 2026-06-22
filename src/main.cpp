#include <filesystem>

#include "core/TorrentClient.hpp"
#include "ui/TorrentUi.hpp"
#include "utils/Logger.hpp"

int main(int argc, char* argv[]) {
    using namespace tclient;

    if (argc != 3) {
        Logger::LogError(
            "Usage: " + std::string(argv[0]) +
            " <torrent-file> <output-directory>"
        );
        return EXIT_FAILURE;
    }

    std::filesystem::path torrent_file_path = argv[1];
    std::filesystem::path output_directory = argv[2];

    if (!std::filesystem::exists(torrent_file_path)) {
        Logger::LogError(
            "Error: Torrent file not found: " + torrent_file_path.string()
        );
        return EXIT_FAILURE;
    }

    if (!std::filesystem::exists(output_directory)) {
        std::filesystem::create_directories(output_directory);
    }

    try {
        TorrentClient client;

        client.DownloadTorrent(torrent_file_path, output_directory);

        TorrentUi torrent_ui(client);
        torrent_ui.Run();

    } catch (const std::exception& error) {
        Logger::LogError("Error: " + std::string(error.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

