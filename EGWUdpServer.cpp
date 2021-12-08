
#include "NDFServiceLog.hpp"
#include "EGWUdpServer.hpp"

EGWUdpServer::EGWUdpServer() {
    memset(&fds_, 0, sizeof(fds_));
    memset(&clientAddr_, 0, sizeof(clientAddr_));
    addrlen_ = sizeof(clientAddr_);

}

EGWUdpServer::~EGWUdpServer() {
    Cleanup();
}

bool EGWUdpServer::Init() {
    return Init(port_, bIpv6_);
}

bool EGWUdpServer::Init(const int _port, const bool _bIpv6) {

    if(port_ != _port)
        port_ = _port;

    if(bIpv6_ != _bIpv6)
        bIpv6_ = _bIpv6;

    if(port_ < 0) {
        E_LOG("server port is invalid [%d]. you should call SetPort()", port_);
        return false;
    }

    clear();

    bool bret = (bIpv6_)?createV6():createV4();

    if(bret == false)
        clear();

    return bret;
}

void EGWUdpServer::Cleanup() {
    clear();
}

void EGWUdpServer::clear() {
    if(fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }

    memset(&fds_, 0, sizeof(fds_));
}

bool EGWUdpServer::createV6() {

    if((fd_ = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        char buf[128];
        E_LOG("socket() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        return false;
    }

    int nval = 1;
    if(setsockopt(fd_,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  &nval,
                  sizeof(nval)) < 0)
    {
        char buf[128];
        E_LOG("setsockopt() SO_REUSEADDR fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        return false;
    }

    struct sockaddr_in6  addr;
    memset(&addr, 0, sizeof(struct sockaddr_in6));

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons((short)port_);
    addr.sin6_addr = in6addr_any;

    return ready((struct sockaddr *)&addr, sizeof(addr));
}

bool EGWUdpServer::createV4() {

    if((fd_ = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {

        char buf[128];
        E_LOG("socket() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        return false;
    }

    int nval = 1;
    if(setsockopt(fd_,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  &nval,
                  sizeof(nval)) < 0)
    {
        char buf[128];
        E_LOG("setsockopt() SO_REUSEADDR fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        return false;
    }

    struct sockaddr_in  addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    return ready((struct sockaddr *)&addr, sizeof(addr));

}

bool EGWUdpServer::ready(struct sockaddr * _addr, socklen_t _addrlen) {

    if(bind(fd_, _addr, _addrlen) < 0)
    {
        char buf[128];
        E_LOG("bind() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        return false;
    }

    // option
    struct timeval  to;
    to.tv_sec = 3;
    to.tv_usec = 0;

    if(setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) < 0) {
        char buf[128];
        W_LOG("setsockopt() - SO_RCVTIMEO fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));
    }

    if(setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) < 0) {
        char buf[128];
        W_LOG("setsockopt() - SO_SNDTIMEO fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));
    }

    memset(&fds_, 0, sizeof(fds_));
    fds_.fd      = fd_;
    fds_.events  = POLLIN;

    return true;
}

void EGWUdpServer::reInit() {
    clear();
    Init();
}

int EGWUdpServer::Recv(char * _ptr, size_t _size) {

    if(fd_ < 0) {
        W_LOG("fd is invalid");
        return -1;
    }

    int ret = poll(&fds_, 1, period_);

    switch(ret) {
    case 0:
        return 0;
    case -1:
        char buf[128];
        W_LOG("poll() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        reInit();
        return 0;
    default:
        break;
    }

    if(fds_.revents != POLLIN)
    {
        W_LOG("poll() unexpected event [%d]", fds_.revents);

        reInit();
        return 0;
    }

    int readn = recvfrom(fd_,
                         _ptr,
                         _size,
                         MSG_WAITALL,
                         (struct sockaddr *)&clientAddr_,
                         &addrlen_);

    if(readn < 0) {
        char buf[128];
        W_LOG("recvfrom() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        reInit();
        return 0;
    }

    return readn;

}

bool EGWUdpServer::Send(const char * _ptr, size_t _size) {

    if(fd_ < 0) {
        W_LOG("fd is invalid");
        return false;
    }

    ssize_t sendt = sendto(fd_,
                          _ptr,
                          _size,
                          MSG_CONFIRM,
                          (const struct sockaddr *)&clientAddr_,
                          addrlen_);

    if(sendt != ssize_t(_size)) {
        char buf[128];
        W_LOG("Send() fail [%zd] [%d:%s]",
            sendt, errno, strerror_r(errno, buf, sizeof(buf)));

        reInit();
        return false;
    }

    return true;
}
