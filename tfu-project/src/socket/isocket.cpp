#include "isocket.h"
#include <cstring>
#include <cstdio>

#ifdef _WIN32
static struct WsaInit {
    WsaInit()  { WSADATA d; WSAStartup(MAKEWORD(2,2), &d); }
    ~WsaInit() { WSACleanup(); }
} wsa_init_;
#endif

static void close_fd(SocketFd fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    ::close(fd);
#endif
}

ISocket::ISocket() : fd_(INVALID_SOCK) {}
ISocket::ISocket(SocketFd fd) : fd_(fd) {}

ISocket::~ISocket() { Close(); }

void ISocket::Close() {
    if (fd_ != INVALID_SOCK) { close_fd(fd_); fd_ = INVALID_SOCK; }
}

bool ISocket::Connect(const std::string& ip, int port) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ == INVALID_SOCK) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(port));
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    return ::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
}

bool ISocket::Listen(int port) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ == INVALID_SOCK) return false;

    int opt = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port));

    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) return false;
    return ::listen(fd_, 1) == 0;
}

ISocket* ISocket::Accept() {
    SocketFd client = ::accept(fd_, nullptr, nullptr);
    if (client == INVALID_SOCK) return nullptr;
    return new ISocket(client);
}

int ISocket::Send(const void* data, size_t len) {
    size_t sent = 0;
    auto* p = static_cast<const char*>(data);
    while (sent < len) {
        auto n = ::send(fd_, p + sent, len - sent, 0);
        if (n <= 0) return -1;
        sent += static_cast<size_t>(n);
    }
    return static_cast<int>(sent);
}

int ISocket::Receive(void* data, size_t len) {
    size_t got = 0;
    auto* p = static_cast<char*>(data);
    while (got < len) {
        auto n = ::recv(fd_, p + got, len - got, 0);
        if (n <= 0) return (got > 0) ? static_cast<int>(got) : -1;
        got += static_cast<size_t>(n);
    }
    return static_cast<int>(got);
}
