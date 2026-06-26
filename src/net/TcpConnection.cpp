#include "net/TcpConnection.hpp"

#include <netinet/tcp.h>

#include <cstdio>
#include <cstring>

#include "utils/byte_tools.hpp"

namespace tclient {

TcpConnection::TcpConnection(
    std::string_view ip,
    int port
) :
    ip(ip),
    port(port),
    socket_fd(-1)
{}

TcpConnection::~TcpConnection() {
    CloseConnection();
}

void TcpConnection::SetNonBlocking(bool enable) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (enable) {
        fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    } else {
        fcntl(socket_fd, F_SETFL, flags & ~O_NONBLOCK);
    }
}

void TcpConnection::WaitForConnection(int fd, fd_set* fdset, struct timeval& tv) {
    int code = select(fd + 1, nullptr, fdset, nullptr, &tv);

    if (code == 0) {
        close(socket_fd);
        socket_fd = -1;
        throw std::runtime_error("Connection timeout");
    }

    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

    if (so_error != 0) {
        close(socket_fd);
        socket_fd = -1;
        throw std::runtime_error("Socket connection error");
    }
}

void TcpConnection::CloseConnection() {
    is_force_closed.store(true);
    if (socket_fd != -1) {
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
        socket_fd = -1;
    }
}

bool TcpConnection::IsTerminated() const {
    return is_force_closed.load();
}

void TcpConnection::ForceClose() {
    is_force_closed.store(true);
    if (socket_fd != -1) {
        close(socket_fd);
        socket_fd = -1;
    }
}

void TcpConnection::EstablishConnection() {
    is_force_closed.store(false);

    if (socket_fd != -1) {
        close(socket_fd);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        throw std::runtime_error(
            "Failed to create socket: " + std::string(strerror(errno))
        );
    }

    int flag = 1;
    setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));

    int keepidle = 30;
    int keepintvl = 10;
    int keepcnt = 3;
    setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));

    setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &kBufferSize, sizeof(kBufferSize));
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &kBufferSize, sizeof(kBufferSize));

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(static_cast<uint16_t>(port));

    SetNonBlocking(true);

    int code = connect(
        socket_fd,
        reinterpret_cast<struct sockaddr*>(&server),
        sizeof(struct sockaddr_in)
    );

    if (code == 0) {
        SetNonBlocking(false);
        return;
    }

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(socket_fd, &fdset);

    struct timeval time_val;
    time_val.tv_sec =
        std::chrono::duration_cast<std::chrono::seconds>(kConnectTimeout).count();
    time_val.tv_usec = 0;

    WaitForConnection(socket_fd, &fdset, time_val);
    SetNonBlocking(false);
}

void TcpConnection::SendData(std::string_view data) const {
    if (is_force_closed.load() || socket_fd == -1) {
        throw std::runtime_error("Connection closed");
    }

    size_t total_sent = 0;
    while (total_sent < data.size()) {
        if (is_force_closed.load()) {
            throw std::runtime_error("Connection terminated during send");
        }

        ssize_t sent = send(
            socket_fd,
            data.data() + total_sent,
            data.size() - total_sent,
            MSG_NOSIGNAL
        );

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                struct pollfd pfd;
                pfd.fd = socket_fd;
                pfd.events = POLLOUT;
                int ret = poll(&pfd, 1, static_cast<int>(kReadTimeout.count()));
                if (ret <= 0 || is_force_closed.load()) {
                    throw std::runtime_error(
                        "Send timeout or connection terminated"
                    );
                }
                continue;
            }
            throw std::runtime_error("Send error: " + std::string(strerror(errno)));
        }

        total_sent += static_cast<size_t>(sent);
    }
}

std::string TcpConnection::ReceiveExactBytes(size_t num_bytes) const {
    std::string result;
    result.resize(num_bytes);
    size_t total_received = 0;

    while (total_received < num_bytes) {
        if (is_force_closed.load()) {
            throw std::runtime_error("Connection terminated during receive");
        }

        struct pollfd pfd;
        pfd.fd = socket_fd;
        pfd.events = POLLIN;
        int ret = poll(&pfd, 1, static_cast<int>(kReadTimeout.count()));

        if (is_force_closed.load()) {
            throw std::runtime_error("Connection terminated");
        }

        if (ret == -1) {
            throw std::runtime_error("Poll error: " + std::string(strerror(errno)));
        }

        if (ret == 0) {
            throw std::runtime_error("Receive timeout");
        }

        ssize_t received = recv(
            socket_fd,
            result.data() + total_received,
            num_bytes - total_received,
            0
        );

        if (received <= 0) {
            if (received == 0) {
                throw std::runtime_error("Connection closed by peer");
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            throw std::runtime_error("Read error: " + std::string(strerror(errno)));
        }

        total_received += static_cast<size_t>(received);
    }

    return result;
}

std::string TcpConnection::ReceiveData(size_t expected_size) const {
    if (is_force_closed.load() || socket_fd == -1) {
        throw std::runtime_error("Connection closed");
    }

    size_t message_size = expected_size;
    std::string message;

    if (message_size == 0) {
        struct pollfd pfd;
        pfd.fd = socket_fd;
        pfd.events = POLLIN;
        int ret = poll(&pfd, 1, static_cast<int>(kReadTimeout.count()));

        if (is_force_closed.load()) {
            throw std::runtime_error("Connection terminated");
        }

        if (ret == -1) {
            throw std::runtime_error("Poll error: " + std::string(strerror(errno)));
        }

        if (ret == 0) {
            return "";
        }

        std::string header = ReceiveExactBytes(4);
        message_size = static_cast<size_t>(bytes_to_int32(header));

        if (message_size == 0) {
            return header;
        }

        message = std::move(header);
    }

    if (message_size > kMaxMessageSize) {
        throw std::runtime_error(
            "Message too large: " + std::to_string(message_size) + " bytes"
        );
    }

    std::string body = ReceiveExactBytes(message_size);
    message += std::move(body);

    return message;
}

const std::string& TcpConnection::GetIp() const {
    return ip;
}

int TcpConnection::GetPort() const {
    return port;
}

} // namespace tclient

