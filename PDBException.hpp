#ifndef PDB_EXCEPTION_HPP
#define PDB_EXCEPTION_HPP

#include <string>
#include <exception>
#include <cstdio>

namespace PDB {

class PDBException : public std::exception
{
public:
    explicit PDBException() {}
    virtual ~PDBException() {}

    virtual const char * what() = 0;

};

class ConfigException : public PDBException
{
public:
    explicit ConfigException(std::string _reason)
        : reason_(_reason) {
    }

    virtual ~ConfigException() {}

    const char * what() { return reason_.c_str(); }
private:
    std::string     reason_;
};

class InvalidConnectionE : public PDBException
{
public:
    explicit InvalidConnectionE(std::string _ip, int _port) {
        ip_     = _ip;
        port_   = _port;
    }

    explicit InvalidConnectionE(std::string _ip, int _port, std::string _className) {
        ip_         = _ip;
        port_       = _port;
        className_  = _className;
    }

    const char * what() {
        char    buf[256];

        snprintf(buf,
            sizeof(buf),
            "DB [%s:%d] ClassName [%s]",
            ip_.c_str(), port_, className_.c_str());

        ret_ = buf;
        return ret_.c_str();
    }

private:
    std::string     ip_;
    int             port_;
    std::string     className_;

    std::string     ret_;

};

class NotFoundConnectionE : public PDBException {
public:
    explicit NotFoundConnectionE(size_t         _pdbT,
                                std::string     _shardingKey,
                                std::string     _className) {
        pdbT_           = _pdbT;
        shardingKey_    = _shardingKey;
        shardingNumber_ = 0;
        className_      = _className;
    }

    explicit NotFoundConnectionE(size_t     _pdbT,
                                size_t      _shardingNumber,
                                std::string _className) {
        pdbT_           = _pdbT;
        shardingNumber_ = _shardingNumber;
        className_      = _className;
    }

    const char * what() {
        char    buf[256];

        snprintf(buf,
            sizeof(buf),
            "DB [%zu:%s:%zu] ClassName [%s]",
            pdbT_, shardingKey_.c_str(), shardingNumber_, className_.c_str());

        ret_ = buf;
        return ret_.c_str();
    }

private:
    size_t          pdbT_;
    std::string     shardingKey_;
    size_t          shardingNumber_;
    std::string     className_;

    std::string     ret_;

};

class AllocStatementE : public PDBException {
public:
    explicit AllocStatementE(std::string _ip,
                            int         _port,
                            std::string _className) {
        ip_         = _ip;
        port_       = _port;
        className_  = _className;
    }

    const char * what() {
        char    buf[256];

        snprintf(buf,
            sizeof(buf),
            "DB [%s:%d] ClassName [%s]",
            ip_.c_str(), port_, className_.c_str());

        ret_ = buf;
        return ret_.c_str();
    }

private:
    std::string     ip_;
    int             port_;
    std::string     className_;

    std::string     ret_;

};

class ExecuteE : public PDBException {
public:
    explicit ExecuteE(std::string   _ip,
                      int           _port,
                      std::string   _className,
                      std::string   _state) {

        ip_         = _ip;
        port_       = _port;
        className_  = _className;
        state_      = _state;
    }

    const char * what() {
        char    buf[256];

        snprintf(buf,
            sizeof(buf),
            "DB [%s:%d] ClassName [%s] reason [%s]",
            ip_.c_str(), port_, className_.c_str(), state_.c_str());

        ret_ = buf;
        return ret_.c_str();
    }

private:
    std::string     ip_;
    int             port_;
    std::string     className_;
    std::string     state_;

    std::string     ret_;

};



} // PDB

#endif // PDB_EXCEPTION_HPP
