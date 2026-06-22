#include "ui/UiFormatter.hpp"

#include <iomanip>
#include <sstream>

namespace tclient {

std::string UiFormatter::FormatDuration(std::chrono::seconds duration) {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;

    std::ostringstream oss;
    oss
        << std::setfill('0')
        << std::setw(2)
        << hours.count()
        << ':'
        << std::setw(2)
        << minutes.count()
        << ':'
        << std::setw(2) << duration.count();
    return oss.str();
}

std::string UiFormatter::FormatBytes(uint64_t bytes) {
    static constexpr double kKB = 1024.0;
    static constexpr double kMB = kKB * 1024.0;
    static constexpr double kGB = kMB * 1024.0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (bytes >= static_cast<uint64_t>(kGB)) {
        oss << static_cast<double>(bytes) / kGB << " GB";
    } else if (bytes >= static_cast<uint64_t>(kMB)) {
        oss << static_cast<double>(bytes) / kMB << " MB";
    } else if (bytes >= static_cast<uint64_t>(kKB)) {
        oss << static_cast<double>(bytes) / kKB << " KB";
    } else {
        oss << bytes << " B";
    }

    return oss.str();
}

std::string UiFormatter::BuildProgressBar(double progress, size_t width) {
    size_t filled =
        static_cast<size_t>(progress / 100.0 * static_cast<double>(width));
    if (filled > width) {
        filled = width;
    }

    return "[" +
        std::string(filled, '#') +
        std::string(width - filled, '.') +
        "]";
}

std::string UiFormatter::Trim(std::string_view text, size_t max_width) {
    if (text.size() <= max_width) {
        return std::string(text);
    }

    if (max_width <= 3) {
        return std::string(text.substr(0, max_width));
    }

    return std::string(text.substr(0, max_width - 3)) + "...";
}

std::string UiFormatter::Center(std::string_view text, size_t width) {
    if (text.size() >= width) {
        return std::string(text);
    }

    size_t padding = (width - text.size()) >> 1;
    return std::string(padding, ' ') + std::string(text);
}

} // namespace tclient

