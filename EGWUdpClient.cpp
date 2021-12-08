
#include <arpa/inet.h>

// #include "EGWDefine.hpp"
#include "NDFServiceLog.hpp"
#include "EGWUdpClient.hpp"

EGWUdpClient::EGWUdpClient() {
    memset(&serverAddr_, 0, sizeof(serverAddr_));

}

EGWUdpClient::EGWUdpClient(const std::string & _ip, const int _port)
    : ip_(_ip),
      port_(_port) {

    memset(&serverAddr_, 0, sizeof(serverAddr_));
}

EGWUdpClient::~EGWUdpClient() {

}

bool EGWUdpClient::Init() {
    return Init(ip_, port_);
}

bool EGWUdpClient::Init(const std::string & _ip, const int _port) {

    if(ip_ != _ip)
        ip_ = _ip;

    if(port_ != _port)
        port_ = _port;

    clear();

    if(_ip.find(".") != std::string::npos)
        return makeSockAddrV4(_ip, _port);

    return makeSockAddrV6(_ip, _port);
}

void EGWUdpClient::clear() {
    if(fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

bool EGWUdpClient::makeSockAddrV4(const std::string & _ip, const int _port) {

    struct sockaddr_in  addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    if((fd_ = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        char buf[128];
        W_LOG("socket() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)_port);


    if(inet_pton(AF_INET, _ip.c_str(), &(addr.sin_addr)) <= 0)
    {
        char buf[128];
        W_LOG("inet_pton() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        clear();
        return false;
    }
    
    /*-
    struct in_addr  in;

    if((in.s_addr = inet_addr(_ip.c_str())) != (unsigned int)-1)
        addr.sin_addr.s_addr = in.s_addr;
    else
    {
        char buf[128];
        W_LOG("inet_addr() fail [%d:%s]",
             errno, strerror_r(errno, buf, sizeof(buf)));

        clear();
        return false;
    }
    -*/

    addrlen_ = sizeof(addr);
    memcpy(&serverAddr_, &addr, addrlen_);

    return true;
}

bool EGWUdpClient::makeSockAddrV6(const std::string & _ip, const int _port) {

    struct sockaddr_in6  addr;
    memset(&addr, 0, sizeof(struct sockaddr_in6));

    if((fd_ = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        char buf[128];
        W_LOG("socket() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        return false;
    }

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons((short)_port);

    if(inet_pton(AF_INET6, _ip.c_str(), &(addr.sin6_addr)) <= 0)
    {
        char buf[128];
        W_LOG("inet_pton() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        clear();
        return false;
    }

    addrlen_ = sizeof(addr);
    memcpy(&serverAddr_, &addr, addrlen_);

    return true;
}

void EGWUdpClient::reInit() {
    clear();
    Init();
}

int EGWUdpClient::Recv(char * _ptr, size_t _size) {

    if(fd_ < 0) {
        W_LOG("fd is invalid");
        return -1;
    }

    struct sockaddr_in6     serverAddr;
    socklen_t               addrlen;

    int readn = recvfrom(fd_,
                         _ptr,
                         _size,
                         MSG_WAITALL,
                         (struct sockaddr *)&serverAddr,
                         &addrlen);

    if(readn < 0) {
        char buf[128];
        W_LOG("recvfrom() fail [%d:%s]",
            errno, strerror_r(errno, buf, sizeof(buf)));

        reInit();
    }

    return readn;

}


bool EGWUdpClient::Send(const char * _ptr, size_t _size) {
    if(fd_ < 0) {
        W_LOG("fd is invalid");
        return false;
    }

    size_t sendt = sendto(fd_,
                          _ptr,
                          _size,
                          MSG_CONFIRM,
                          (const struct sockaddr *)&serverAddr_,
                          addrlen_);

    if(sendt != _size) {
        char buf[128];
        W_LOG("Send() fail [%zd] [%d:%s]",
            sendt, errno, strerror_r(errno, buf, sizeof(buf)));

        reInit();
        return false;
    }


    return true;
}
