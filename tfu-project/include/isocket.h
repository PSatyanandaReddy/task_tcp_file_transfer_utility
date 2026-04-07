#ifndef TFU_ISOCKET_H
#define TFU_ISOCKET_H

#include <string>
#include <cstddef>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  using SocketFd = SOCKET;
  static constexpr SocketFd INVALID_SOCK = INVALID_SOCKET;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  using SocketFd = int;
  static constexpr SocketFd INVALID_SOCK = -1;
#endif

class ISocket {
public:
    ISocket();
    ~ISocket();

    bool Connect(const std::string& ip, int port);
    bool Listen(int port);
    ISocket* Accept();
    int  Send(const void* data, size_t len);
    int  Receive(void* data, size_t len);
    void Close();

private:
    SocketFd fd_;
    explicit ISocket(SocketFd fd);
};

#endif
