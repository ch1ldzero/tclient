#pragma once

#include <string>
#include <map>

struct ParsedUrl {
    std::string host;
    std::string path;
    int port;
    bool is_https;
};

class HttpClient {
public:
    std::string Get(
        const std::string& url,
        const std::map<std::string, std::string>& params,
        int timeout_sec = 10
    ) const;

private:
    ParsedUrl ParseUrl(const std::string& url) const;

    std::string UrlEncode(const std::string& value) const;

    std::string BuildQuery(
        const std::map<std::string, std::string>& params
    ) const;
};

