#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

namespace tclient {

class UiFormatter {
public:
    static std::string FormatDuration(std::chrono::seconds duration);
    static std::string FormatBytes(uint64_t bytes);

    static std::string BuildProgressBar(double progress, size_t width = 50);

    static std::string Trim(std::string_view text, size_t max_width);
    static std::string Center(std::string_view text, size_t width);
};

} // namespace tclient

