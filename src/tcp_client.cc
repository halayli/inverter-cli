#include "tcp_client.h"

#include <fmt/format.h>

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <exception>

namespace iv {

TcpClient::TcpClient(const HostPort& hostport, unsigned timeout)
    : _fd(-1), _hostport(hostport), _timeout(timeout)
{
}

TcpClient::TcpClient(TcpClient&& rhs) noexcept : TcpClient()
{
    *this = std::move(rhs);
}

TcpClient&
TcpClient::operator=(TcpClient&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    this->close();

    return *this;
}

TcpClient::operator bool() const noexcept
{
    return _fd != -1;
}

void
TcpClient::send(std::span<const uint8_t> buf)
{
    if (::send(_fd, buf.data(), buf.size(), 0) < 0)
        throw_runtime_err("send failed", errno);
}

void
TcpClient::recv(std::span<uint8_t> buf)
{
    if (::recv(_fd, buf.data(), buf.size(), 0) != buf.size())
        throw_runtime_err("recv failed", errno);
}

void
TcpClient::connect()
{
    close();

    struct hostent* he;
    if ((he = gethostbyname(_hostport.host.c_str())) == NULL)
        throw_runtime_err("failed to resolve host", errno);

    if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw_runtime_err("failed to create socket", errno);

    set_timeout();

    struct sockaddr_in dst;
    dst.sin_family = AF_INET;
    dst.sin_port = htons(_hostport.port);
    dst.sin_addr = *((struct in_addr*)he->h_addr);
    bzero(&(dst.sin_zero), 8);

    if (::connect(_fd, (struct sockaddr*)&dst, sizeof(struct sockaddr)) == -1)
        throw_runtime_err("connect failed", errno);
}

void
TcpClient::close()
{
    if (_fd == -1)
        return;

    ::close(_fd);
    _fd = -1;
}

void
TcpClient::set_timeout()
{
    if (!*this)
        return;

    struct timeval timeout {
        .tv_sec = _timeout, .tv_usec = 0
    };

    if (setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
        throw_runtime_err(fmt::format("failed to set timeout={}", _timeout), errno);

    if (setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0)
        throw_runtime_err(fmt::format("failed to set timeout={}", _timeout), errno);
}

void
TcpClient::throw_runtime_err(const std::string& msg, int sys_errno)
{
    throw std::runtime_error(fmt::format("TcpClient: {} - timeout={}secs host={}:{} fd={} err={}",
                                         msg,
                                         _timeout,
                                         _hostport.host,
                                         _hostport.port,
                                         _fd,
                                         strerror(sys_errno)));
}

} // namespace iv
