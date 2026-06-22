#include "tracker/TrackerFactory.hpp"

#include <stdexcept>

#include "tracker/HttpTracker.hpp"
#include "tracker/UdpTracker.hpp"

namespace tclient {

std::unique_ptr<ITracker> TrackerFactory::CreateTracker(std::string_view url) {
    if (IsUdpTracker(url)) {
        const auto [host, port] = ParseUdpUrl(std::string(url));
        return std::make_unique<UdpTracker>(host, port);
    }

    return std::make_unique<HttpTracker>(std::string(url));
}

bool TrackerFactory::IsUdpTracker(std::string_view url) {
    return url.substr(0, 6) == "udp://";
}

std::pair<std::string, int> TrackerFactory::ParseUdpUrl(std::string_view url) {
    if (url.substr(0, 6) != "udp://") {
        throw std::runtime_error("Invalid UDP URL: " + std::string(url));
    }

    std::string host_port(url.substr(6));

    size_t slash_pos = host_port.find('/');
    if (slash_pos != std::string::npos) {
        host_port = host_port.substr(0, slash_pos);
    }

    size_t colon_pos = host_port.find(':');
    if (colon_pos != std::string::npos) {
        std::string host = host_port.substr(0, colon_pos);
        int port = std::stoi(host_port.substr(colon_pos + 1));
        return {host, port};
    }

    return std::make_pair(host_port, 80);
}

} // namespace tclient

