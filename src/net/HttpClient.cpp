#include "net/HttpClient.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sstream>
#include <stdexcept>
#include <iomanip>

std::string HttpClient::Get(
    const std::string& url,
    const std::map<std::string, std::string>& params,
    int timeout_sec
) const {
    auto [host, path, port, https] = ParseUrl(url);

    std::string query = BuildQuery(params);
    if (!query.empty()) {
        path += "?" + query;
    }

    addrinfo hints{};
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

    int sock = socket(
        result->ai_family,
        result->ai_socktype,
        result->ai_protocol
    );

    if (sock < 0) {
        freeaddrinfo(result);
        throw std::runtime_error("socket failed");
    }

    timeval tv{};
    tv.tv_sec = timeout_sec;

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
        << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Connection: close\r\n"
        << "User-Agent: TorrentClient/1.0\r\n\r\n";

    std::string req = request.str();
    std::string response;

    if (https) {
        SSL_library_init();

        SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());

        if (!ctx) {
            close(sock);
            throw std::runtime_error("SSL_CTX_new failed");
        }

        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sock);

        if (SSL_connect(ssl) <= 0) {
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);
            throw std::runtime_error("SSL_connect failed");
        }

        SSL_write(ssl, req.c_str(), req.size());

        char buffer[BUFSIZ];
        int bytes;

        while ((bytes = SSL_read(ssl, buffer, sizeof(buffer))) > 0) {
            response.append(buffer, bytes);
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);

    } else {
        send(sock, req.c_str(), req.size(), 0);

        char buffer[BUFSIZ];
        ssize_t bytes;

        while ((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            response.append(buffer, bytes);
        }
    }

    close(sock);

    size_t header_end = response.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        throw std::runtime_error("Invalid HTTP response");
    }

    std::string headers = response.substr(0, header_end);

    if (
        headers.find("200 OK") == std::string::npos
    ) {
        throw std::runtime_error("Tracker returned non-200");
    }

    return response.substr(header_end + 4);
}

ParsedUrl HttpClient::ParseUrl(
    const std::string& url
) const {
    ParsedUrl request;

    request.is_https = false;

    std::string working = url;

    if (working.starts_with("https://")) {
        request.is_https = true;
        request.port = 443;
        working = working.substr(8);
    } else if (working.starts_with("http://")) {
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

std::string HttpClient::UrlEncode(const std::string& value) const {
    std::ostringstream escaped;

    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : value) {
        if (
            std::isalnum(c) ||
            c == '-' ||
            c == '_' ||
            c == '.' ||
            c == '~'
        ) {
            escaped << c;
        } else {
            escaped << '%'
                    << std::uppercase
                    << std::setw(2)
                    << int(c)
                    << std::nouppercase;
        }
    }

    return escaped.str();
}

std::string HttpClient::BuildQuery(
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

