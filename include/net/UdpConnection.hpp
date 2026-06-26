#pragma once

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <string>
#include <string_view>

namespace tclient {

class UdpConnection {
public:
    UdpConnection(std::string_view host, int port);
    ~UdpConnection();

    void Send(std::string_view data);
    std::string Receive();

private:
    static constexpr std::chrono::seconds kTimeout = std::chrono::seconds(1);

    int socket_fd = -1;
    struct sockaddr_in server_address;

    std::string host;
    int port;
};

} // namespace tclient

