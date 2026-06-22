#include "net/UdpConnection.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace tclient {

UdpConnection::UdpConnection(
    std::string_view host,
    int port
) :
    host(host),
    port(port)
{
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        throw std::runtime_error(
            "[UdpConnection] Failed to create socket: " +
            std::string(strerror(errno))
        );
    }

    struct timeval tv;
    tv.tv_sec = kTimeout.count();
    tv.tv_usec = 0;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        close(socket_fd);
        throw std::runtime_error(
            "[UdpConnection] Failed to set SO_RCVTIMEO: " +
            std::string(strerror(errno))
        );
    }

    struct hostent* server = gethostbyname(std::string(host).c_str());
    if (!server) {
        close(socket_fd);
        throw std::runtime_error(
            "[UdpConnection] Failed to resolve host " +
            std::string(host) +
            ": " +
            std::string(hstrerror(h_errno))
        );
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(static_cast<uint16_t>(port));
    memcpy(
        &server_address.sin_addr.s_addr,
        server->h_addr,
        static_cast<size_t>(server->h_length)
    );
}

UdpConnection::~UdpConnection() {
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

void UdpConnection::Send(std::string_view data) {
    ssize_t sent = sendto(
        socket_fd,
        data.data(),
        data.size(),
        0,
        reinterpret_cast<sockaddr*>(&server_address),
        sizeof(server_address)
    );

    if (sent < 0) {
        throw std::runtime_error(
            "[UdpConnection] Send error to " +
            host +
            ":" +
            std::to_string(port) +
            ": " +
            strerror(errno)
        );
    }

    if (static_cast<size_t>(sent) != data.size()) {
        throw std::runtime_error(
            "[UdpConnection] Partial send to " +
            host +
            ":" +
            std::to_string(port) +
            ": " +
            std::to_string(sent) +
            " of " +
            std::to_string(data.size()) +
            " bytes"
        );
    }
}

std::string UdpConnection::Receive() {
    constexpr size_t kBufferSize = 65'536;
    std::vector<char> buffer(kBufferSize);
    socklen_t address_length = sizeof(server_address);

    ssize_t received = recvfrom(
        socket_fd,
        buffer.data(),
        buffer.size(),
        0,
        reinterpret_cast<sockaddr*>(&server_address),
        &address_length
    );

    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            throw std::runtime_error(
                "[UdpConnection] Timeout waiting for response (" +
                std::to_string(kTimeout.count()) +
                "s) from " +
                host +
                ":" +
                std::to_string(port)
            );
        }
        throw std::runtime_error(
            "[UdpConnection] Receive error from " +
            host +
            ":" +
            std::to_string(port) +
            ": " +
            strerror(errno)
        );
    }

    return std::string(buffer.data(), static_cast<size_t>(received));
}

} // namespace tclient

