#include <poll.h>

#include "PDBConnection.hpp"
#include "PDBOdbcErr.hpp"

namespace PDB {

Connection::Connection()
    : uuid_(0),
      lastPingCheckT_(0),
      usedCnt_(0),
      bConnected_(false),
      port_(0),
      bAutoCommit_(true),
      nConnectionTimeout_(5),
      henv_(SQL_NULL_HENV),
      hdbc_(SQL_NULL_HDBC),
      sret_(SQL_SUCCESS) {

}

Connection::~Connection() {
    Lock();
    Disconnect();

    if(henv_ != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv_);
        henv_ = SQL_NULL_HENV;
    }

    UnLock();
}

void Connection::Disconnect() {

    bConnected_.store(false);

    setRegisteredStmt_.clear();

    if(hdbc_ != SQL_NULL_HDBC) {
        SQLDisconnect(hdbc_);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc_);
        hdbc_ = SQL_NULL_HDBC;
        W_THD_LOG(gLogName, "Connection.Disconnect [%s:%d] uuid[%lu]",
            ip_.c_str(),
            port_,
            uuid_);
    }

    uuid_ = 0;
    usedCnt_ = 0;
}

void Connection::Set(stConnectionInfo & _info) {

    dbName_ = _info.DBName;

    char buf[512];

    // ALTIBASE 용

    snprintf(buf,
        sizeof(buf),
        "DSN=%s;"
        "SERVER=%s;"
        "UID=%s;"
        "PWD=%s;"
        "CONNTYPE=%d;"
        "NLS_NCHAR_LITERAL_REPLACE=%d;"
        "PORT_NO=%d;"
        "TIMEOUT=%d;"
        "CONNECTION_TIMEOUT=%d",
        _info.DSN.c_str(),
        _info.Host.c_str(),
        _info.Id.c_str(),
        _info.Pwd.c_str(),
        (_info.nConnType==0)?1:_info.nConnType,
        (_info.bNcharCheck)?1:0,
        _info.nPort,
        (_info.nTimeout == 0)?1:_info.nTimeout,
        (_info.nConnectionTimeout==0)?1:_info.nConnectionTimeout);


    if(strcasestr(_info.DSN.c_str(), "GOLDILOCKS") != nullptr) {

        snprintf(buf,
            sizeof(buf),
            "DSN=%s;"
            "HOST=%s;"
            "UID=%s;"
            "PWD=%s;"
            "PORT=%d",
            _info.DSN.c_str(),
            _info.Host.c_str(),
            _info.Id.c_str(),
            _info.Pwd.c_str(),
            _info.nPort);
    }


    connString_ = buf;

    ip_ = _info.Host;
    port_ = _info.nPort;

    if(_info.nConnectionTimeout > 0)
        nConnectionTimeout_ = _info.nConnectionTimeout;

    // d_log("connection string : [%s]", connString_.c_str());
}

bool Connection::Connect(unsigned long _uuid) {

    std::lock_guard<std::mutex>     guard(mutex_);

    uuid_ = _uuid;

    if(henv_ == SQL_NULL_HENV) {
        sret_ = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv_);

        if(sret_ == SQL_ERROR || sret_ == SQL_INVALID_HANDLE) {
            E_THD_LOG(gLogName, "Connection.Connect.SQLAllocHandle.forENV fail [%s:%d] [%i]",
                ip_.c_str(),
                port_,
                sret_);
            Disconnect();
            return false;
        }

        sret_ = SQLSetEnvAttr(henv_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

        if(sret_ == SQL_ERROR || sret_ == SQL_INVALID_HANDLE) {
            E_THD_LOG(gLogName, "Connection.Connect.SQLSetEnvAttr fail [%s:%d] [%i]",
                ip_.c_str(),
                port_,
                sret_);
            Disconnect();
            return false;
        }
    }

    // 이미 연결이 있다면 종료한다는 뜻 이예요.
    if(hdbc_ != SQL_NULL_HDBC) {
        E_THD_LOG(gLogName, "Connection.Connect.DisconnectOld");
        SQLDisconnect(hdbc_);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc_);
        hdbc_ = SQL_NULL_HDBC;
    }

    sret_ = SQLAllocHandle(SQL_HANDLE_DBC, henv_, &hdbc_);
    if(sret_ == SQL_ERROR || sret_ == SQL_INVALID_HANDLE) {
        E_THD_LOG(gLogName, "Connection.Connect.SQLAllocHandle.forDBC fail [%s:%d] [%i]",
            ip_.c_str(),
            port_,
            sret_);
        Disconnect();
        return false;
    }

    int nTryCnt = 2;
    callSQLDriverConnect(nTryCnt);

    /*-
    sret_ = SQLDriverConnect(hdbc_,
                            nullptr,
                            (SQLCHAR *)connString_.c_str(),
                            SQL_NTS,
                            nullptr,
                            0,
                            nullptr,
                            SQL_DRIVER_NOPROMPT);
    -*/

    if(sret_ != SQL_SUCCESS) {

        E_THD_LOG(gLogName, "Connection.Connect.SQLDriverConnect  fail [%s:%d] [%i] [%s]",
                ip_.c_str(),
                port_,
                sret_,
                ODBCErr::GetStringDBC(hdbc_).c_str());

        Disconnect();
        return false;
    }

    SQLSetConnectAttr(hdbc_, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)&nConnectionTimeout_, SQL_IS_INTEGER);

    if(bAutoCommit_ == false) {
        sret_ = SQLSetConnectAttr(hdbc_, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);

        if(sret_ != SQL_SUCCESS) {
            W_THD_LOG(gLogName, "Connection.Connect.SQLSetConnectAttr SQL_AUTOCOMMIT_OFF fail [%s:%d] [%i]",
                ip_.c_str(),
                port_,
                sret_);
        }
    }

    bConnected_.store(true);

    I_THD_LOG(gLogName, "Connection.Connect success uuid [%s:%d] [%lu]",
        ip_.c_str(),
        port_,
        uuid_);

    return true;
}

void Connection::callSQLDriverConnect(int _cnt) {
    if(_cnt <= 0)
        return ;

    sret_ = SQLDriverConnect(hdbc_,
                            nullptr,
                            (SQLCHAR *)connString_.c_str(),
                            SQL_NTS,
                            nullptr,
                            0,
                            nullptr,
                            SQL_DRIVER_NOPROMPT);

    if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "Connection.Connect.callSQLDriverConnect [%d] fail [%s] [%i] [%s]",
            _cnt,
            connString_.c_str(),
            sret_,
            ODBCErr::GetStringDBC(hdbc_).c_str());

        poll(nullptr, 0, 5);
        callSQLDriverConnect(--_cnt);
    }

    return ;
}

// Lock 이 많이 호출된다는 것은 사용을 많이 한다는 거예요.
bool Connection::IsBusy() {
    if(usedCnt_ > 0) {
        usedCnt_ = 0;
        return true;
    }

    return false;
}

bool Connection::AllocStmt(SQLHSTMT & _hstmt) {

    // 2021.10.07
    if(_hstmt != SQL_NULL_HSTMT) {
        if(setRegisteredStmt_.find(_hstmt) == setRegisteredStmt_.end()) {
            ReleaseStmt(_hstmt);
        }
    }

    sret_ = SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &_hstmt);
    if(sret_ == SQL_ERROR || sret_ == SQL_INVALID_HANDLE) {
        W_THD_LOG(gLogName, "Connection.AllocStmt.SQLAllocHandle.forSTMT fail [%s:%d] [%i] [%s]",
            ip_.c_str(),
            port_,
            sret_,
            ODBCErr::GetStringDBC(hdbc_).c_str());
        return false;
    }

    setRegisteredStmt_.emplace(_hstmt);

    return true;
}

void Connection::PingCheck() {

    if(TryLock() == false)
        return ;

    if(hdbc_ == SQL_NULL_HDBC) {

        Disconnect();
        UnLock();

        W_THD_LOG(gLogName, "Connection.PingCheck fail [%s:%d] HDBC Handler is null",
            ip_.c_str(),
            port_);

        return ;
    }

    SQLHSTMT    hstmt = SQL_NULL_HSTMT;

    if(AllocStmt(hstmt) == false) {
        Disconnect();
        UnLock();

        W_THD_LOG(gLogName, "Connection.PingCheck.SQLAllocHandle.forSTMT fail [%s:%d] [%i]",
            ip_.c_str(),
            port_,
            sret_);

        return;
    }

    /*-
    sret_ = SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt);

    if(sret_ == SQL_ERROR || sret_ == SQL_INVALID_HANDLE) {
        Disconnect();
        UnLock();

        W_THD_LOG(gLogName, "Connection.PingCheck.SQLAllocHandle.forSTMT fail [%s:%d] [%i]",
            ip_.c_str(),
            port_,
            sret_);

        return;
    }
    -*/

    char buf[64];
    snprintf(buf, sizeof(buf), "SELECT * FROM DUAL");
    sret_ = SQLExecDirect(hstmt, (SQLCHAR *)buf, strlen(buf));

    if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
        W_THD_LOG(gLogName, "Connection.PingCheck.SQLExecDirect fail [%s:%d] [%d] [%s]",
            ip_.c_str(),
            port_,
            sret_,
            ODBCErr::GetStringDBC(hdbc_).c_str());

        /*-
        if(hstmt != SQL_NULL_HSTMT) {
            SQLFreeStmt(hstmt, SQL_CLOSE);
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
        -*/

        Disconnect();
        UnLock();

        return ;
    }

    ReleaseStmt(hstmt);

    /*-  
    if(hstmt != SQL_NULL_HSTMT) {
        SQLFreeStmt(hstmt, SQL_CLOSE);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
    -*/

    UnLock();

    //D_THD_LOG(gLogName, "Connection.PingCheck Success [%s:%d] uuid[%lu]",
    //    ip_.c_str(),
    //    port_,
    //    uuid_);
    return ;
}

void Connection::ReleaseStmt(SQLHSTMT & _hstmt) {

    if(_hstmt != SQL_NULL_HSTMT) {
        D_THD_LOG(gLogName,
            "[%zu] [%p] Connection.ReleaseStmt.SQLFreeHandle",
            syscall(SYS_gettid),
            _hstmt);

        SQLFreeStmt(_hstmt, SQL_CLOSE);
        SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);

        _hstmt = SQL_NULL_HSTMT;
    }

    unregiStmt(_hstmt);
}

void Connection::unregiStmt(SQLHSTMT & _hstmt) {

    if(_hstmt != SQL_NULL_HSTMT) {
        setRegisteredStmt_.erase(_hstmt);
    }
}

bool Connection::sqlEndTran(SQLSMALLINT _completionType) {
    if(IsConnected() == false) {
        W_THD_LOG(gLogName, "Connection.sqlEndTran [%s] fail [%s:%d]",
            (SQL_COMMIT == _completionType)?"COMMIT":"ROLLBACK",
            ip_.c_str(),
            port_);

        return false;
    }

    SQLRETURN sret = SQLEndTran(SQL_HANDLE_DBC, hdbc_, _completionType);

    if(sret != SQL_SUCCESS && sret != SQL_SUCCESS_WITH_INFO) {
        W_THD_LOG(gLogName, "Connection.Commit.SQLEndTran [%s] fail [%s:%d] [%d] [%s]",
            (SQL_COMMIT == _completionType)?"COMMIT":"ROLLBACK",
            ip_.c_str(),
            port_,
            sret_,
            ODBCErr::GetStringDBC(hdbc_).c_str());

        return false;
    }

    return true;
}

bool Connection::Commit() {
    return sqlEndTran(SQL_COMMIT);
}

void Connection::Rollback() {
    sqlEndTran(SQL_ROLLBACK);
}


}
