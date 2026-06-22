#include "net/HttpConnection.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace tclient {

std::string HttpConnection::Get(
    std::string_view url,
    const std::map<std::string, std::string>& params
) const {
    auto [host, path, port] = ParseUrl(url);

    std::string query = BuildQuery(params);
    if (!query.empty()) {
        path += "?" + query;
    }

    addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result;
    if (
        getaddrinfo(
            host.c_str(),
            std::to_string(port).c_str(),
            &hints,
            &result
        ) != 0
    ) {
        throw std::runtime_error("getaddrinfo failed");
    }

    int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(result);
        throw std::runtime_error("socket failed");
    }

    timeval tv;
    tv.tv_sec = kTimeout.count();
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(sock, result->ai_addr, result->ai_addrlen) < 0) {
        close(sock);
        freeaddrinfo(result);
        throw std::runtime_error("connect failed");
    }

    freeaddrinfo(result);

    std::ostringstream request;
    request
        << "GET "
        << path
        << " HTTP/1.1\r\n"
        << "Host: "
        << host
        << "\r\n"
        << "Connection: close\r\n"
        << "User-Agent: TorrentClient/1.0\r\n\r\n";

    std::string req = request.str();
    std::string response;

    send(sock, req.c_str(), req.size(), 0);

    char buffer[BUFSIZ];
    ssize_t bytes;
    while ((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, static_cast<size_t>(bytes));
    }

    close(sock);

    size_t header_end = response.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        throw std::runtime_error("Invalid HTTP response");
    }

    std::string headers = response.substr(0, header_end);
    if (headers.find("200 OK") == std::string::npos) {
        throw std::runtime_error("Tracker returned non-200");
    }

    return response.substr(header_end + 4);
}

ParsedUrl HttpConnection::ParseUrl(std::string_view url) const {
    ParsedUrl request;
    std::string working(url);

    if (working.starts_with("http://")) {
        request.port = 80;
        working = working.substr(7);
    } else {
        throw std::runtime_error("Unsupported URL scheme");
    }

    size_t slash = working.find('/');
    if (slash == std::string::npos) {
        request.host = working;
        request.path = "/";
    } else {
        request.host = working.substr(0, slash);
        request.path = working.substr(slash);
    }

    size_t colon = request.host.find(':');
    if (colon != std::string::npos) {
        request.port = std::stoi(request.host.substr(colon + 1));
        request.host = request.host.substr(0, colon);
    }

    return request;
}

std::string HttpConnection::UrlEncode(std::string_view value) const {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (size_t i = 0; i < value.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(value[i]);
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped
                << '%'
                << std::uppercase
                << std::setw(2)
                << static_cast<int>(c)
                << std::nouppercase;
        }
    }

    return escaped.str();
}

std::string HttpConnection::BuildQuery(
    const std::map<std::string, std::string>& params
) const {
    std::ostringstream oss;

    bool first = true;
    for (const auto& [key, value] : params) {
        if (!first) {
            oss << "&";
        }
        first = false;
        oss << key << "=" << UrlEncode(value);
    }

    return oss.str();
}

} // namespace tclient

