#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace iv {

struct HostPort {
    std::string host;
    uint16_t port{0};
};

class TcpClient {
   public:
    TcpClient() = default;
    TcpClient(const HostPort& hostport, unsigned timeout = 5);

    TcpClient(TcpClient&& rhs) noexcept;
    TcpClient& operator=(TcpClient&& rhs) noexcept;

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;
    operator bool() const noexcept;

    void connect();
    void send(std::span<const uint8_t> buf);
    void recv(std::span<uint8_t> buf);

   private:
    void set_timeout();
    void close();
    void throw_runtime_err(const std::string& msg, int sys_errno = 0);

    int _fd{-1};
    HostPort _hostport;
    unsigned _timeout{5};
};

} // namespace iv
