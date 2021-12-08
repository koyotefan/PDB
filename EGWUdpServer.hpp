#ifndef EGW_UDP_SERVER_HPP
#define EGW_UDP_SERVER_HPP

#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class EGWUdpServer {
public:
    explicit EGWUdpServer();
    ~EGWUdpServer();

    void SetIpv6(bool _on=false) { bIpv6_ = _on; }
    void SetPort(int _port) { port_ = _port; }
    void SetWaitPeriod(int _period) { period_ = _period; }

    bool Init();
    bool Init(const int _port, const bool _bIpv6);

    void Cleanup();

    int Recv(char * _ptr, size_t _size);
    bool Send(const char * _ptr, size_t _size);

private:
    bool    createV4();
    bool    createV6();
    bool    ready(struct sockaddr * _addr, socklen_t _addrlen);
    void    reInit();
    void    clear();

private:
    bool    bIpv6_ = false;
    int     port_ = -1;
    int     period_ = 100;

    int     fd_ = -1;
    struct  pollfd  fds_;

    // For client
    struct sockaddr_in6     clientAddr_;
    socklen_t               addrlen_ = 0;

};


#endif // EGW_UDP_SERVER_HPP
