#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "tracker/ITracker.hpp"

namespace tclient {

class TrackerFactory {
public:
    static std::unique_ptr<ITracker> CreateTracker(std::string_view url);

private:
    static bool IsUdpTracker(std::string_view url);
    static std::pair<std::string, int> ParseUdpUrl(std::string_view url);
};

} // namespace tclient

