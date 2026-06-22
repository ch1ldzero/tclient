#pragma once

#include <chrono>
#include <map>
#include <string>
#include <string_view>

namespace tclient {

struct ParsedUrl {
    std::string host;
    std::string path;
    int port;
};

class HttpConnection {
public:
    std::string Get(
        std::string_view url,
        const std::map<std::string, std::string>& params
    ) const;

private:
    static constexpr std::chrono::seconds kTimeout = std::chrono::seconds(10);

private:
    ParsedUrl ParseUrl(std::string_view url) const;

    std::string UrlEncode(std::string_view value) const;

    std::string BuildQuery(
        const std::map<std::string, std::string>& params
    ) const;
};

} // namespace tclient

