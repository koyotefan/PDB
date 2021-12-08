#ifndef EGW_UDP_CLIENT_HPP
#define EGW_UDP_CLIENT_HPP

#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class EGWUdpClient {

public:
    explicit EGWUdpClient();
    explicit EGWUdpClient(const std::string & _ip, const int _port);
    ~EGWUdpClient();

    void SetIp(std::string _ip) { ip_ = _ip; }
    void SetPort(int _port) { port_ = _port; }

    bool Init();
    bool Init(const std::string & _ip, const int _port);

    bool Send(const char * _ptr, size_t _size);
    int Recv(char * _ptr, size_t _size);

private:
    void clear();
    void reInit();
    bool makeSockAddrV4(const std::string & _ip, const int _port);
    bool makeSockAddrV6(const std::string & _ip, const int _port);

private:
    std::string     ip_;
    int             port_ = -1;

    int             fd_ = -1;
    struct sockaddr_in6     serverAddr_;
    socklen_t               addrlen_ = 0;

};

#endif // EGW_UDP_CLIENT_HPP
