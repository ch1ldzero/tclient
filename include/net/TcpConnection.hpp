#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <string>
#include <string_view>

namespace tclient {

class TcpConnection {
public:
    explicit TcpConnection(std::string_view ip, int port);

    ~TcpConnection();

    void EstablishConnection();
    void SendData(std::string_view data) const;
    std::string ReceiveData(size_t expected_size = 0) const;
    void CloseConnection();
    void ForceClose();
    const std::string& GetIp() const;
    int GetPort() const;
    bool IsTerminated() const;

private:
    static constexpr size_t kMaxMessageSize = 100'000;
    static constexpr int kBufferSize = 1024 * 1024; // 1 MB

    static constexpr std::chrono::milliseconds kConnectTimeout =
        std::chrono::milliseconds(2'000);
    static constexpr std::chrono::milliseconds kReadTimeout =
        std::chrono::milliseconds(5'000);

    const std::string ip;
    const int port;

    mutable std::atomic<bool> is_force_closed = false;
    mutable int socket_fd = -1;

private:
    void SetNonBlocking(bool enable);
    void WaitForConnection(int fd, fd_set* fdset, struct timeval& tv);
    std::string ReceiveExactBytes(size_t num_bytes) const;
};

} // namespace tclient

