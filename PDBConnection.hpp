
#ifndef PDB_CONNECTION_HPP
#define PDB_CONNECTION_HPP

#include "PDBDefine.hpp"

#include <atomic>
#include <mutex>
#include <unordered_set>

#include <sql.h>
#include <sqlext.h>

namespace PDB {

class Connection {
public:
    explicit Connection();
    Connection(const Connection & _rhs) = delete;
    ~Connection();

    void Set(stConnectionInfo & _info);
    void SetAutoCommit(bool _b) { bAutoCommit_ = _b; }
    void Reset();

    bool Connect(unsigned long _uuid);
    void Disconnect();
    bool IsConnected() { return bConnected_; }

    unsigned long GetUuid() const { return uuid_; }
    const char * GetDBName() const { return dbName_.c_str(); }
    const char * GetIp() const { return ip_.c_str(); }
    int GetPort() const { return port_; }
    void Lock() { mutex_.lock(); ++usedCnt_; }
    void UnLock() { mutex_.unlock(); }
    bool TryLock() { return mutex_.try_lock(); }

    SQLRETURN GetSQLReturn() const { return sret_; }

    time_t & GetLastPingCheckTime() { return lastPingCheckT_; }
    void SetPingCheckTime() {  lastPingCheckT_ = time(nullptr);  }
    bool IsBusy();

    bool AllocStmt(SQLHSTMT & _hstmt);
    void ReleaseStmt(SQLHSTMT & _hstmt);
    void PingCheck();

    bool Commit();
    void Rollback();

private:
    bool sqlEndTran(SQLSMALLINT _completionType);
    void callSQLDriverConnect(int _cnt);
    void unregiStmt(SQLHSTMT & _hstmt);

private:
    unsigned long       uuid_;
    time_t              lastPingCheckT_;
    unsigned int        usedCnt_;
    std::atomic_bool    bConnected_;

    std::string         dbName_;
    std::string         connString_;
    std::string         ip_;
    int                 port_;

    bool                bAutoCommit_;
    int                 nConnectionTimeout_;

    SQLHENV             henv_;
    SQLHDBC             hdbc_;
    SQLRETURN           sret_;

    std::mutex          mutex_;
    std::unordered_set<SQLHSTMT>  setRegisteredStmt_;

};

}


#endif // PDB_CONNECTION_HPP
