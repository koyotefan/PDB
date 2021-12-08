
#include "PDBPreparedSQL.hpp"
#include "PDBOdbcErr.hpp"

namespace PDB {

PreparedSQL::PreparedSQL()
    : myT_(eDefDBType::PCF) {

        sret_ = SQL_SUCCESS;
        className_ = typeid(*this).name();
}

PreparedSQL::~PreparedSQL() {
    clear();
}

void PreparedSQL::clear() {
    for(auto & citer : vecStmtInfo_) {
        if(citer.hstmt != SQL_NULL_HSTMT) {
            SQLFreeHandle(SQL_HANDLE_STMT, citer.hstmt);
            citer.hstmt = SQL_NULL_HSTMT;
        }
    }

    vecStmtInfo_.clear();
}

bool PreparedSQL::SetSQL(eDefDBType _pdbT, const char * _sql, size_t _sqlLength) {

    if(_sql == nullptr || _sqlLength == 0)
        return false;

    clear();

    myT_ = _pdbT;
    sql_ = std::string(_sql, _sqlLength);

    size_t maxCnt = 1;
    if(myT_ == eDefDBType::SessionDetail) {
        maxCnt = static_cast<size_t>(eDefValue::MaxShardingCnt);
    }

    for(size_t nLoop=0; nLoop < maxCnt; ++nLoop) {
        stStmtInfo  info;
        vecStmtInfo_.push_back(info);
    }

    return true;
}

void PreparedSQL::AllPrint() {
    int n =0;
    for(const auto & rInfo : vecStmtInfo_) {
        if(rInfo.hstmt != SQL_NULL_HSTMT) 
            I_THD_LOG(gLogName, 
                "---- [%d] [%zu] [%p] [%s] PreparedSQL.AllPrint conn [%zu]",
                    n,
                    syscall(SYS_gettid),
                    rInfo.hstmt,
                    GetClassName(),
                    rInfo.uuidForConnection);
        ++n;
    } 
}

void PreparedSQL::SetBindParameter(STBindParameter * _arrP, size_t _cnt) {

    paramInfo_.ptr = _arrP;
    paramInfo_.cnt = _cnt;

    for(size_t nLoop=0; nLoop < vecStmtInfo_.size(); ++nLoop) {
        stStmtInfo & info = vecStmtInfo_.at(nLoop);

        if(info.hstmt != SQL_NULL_HSTMT)
            bindParameter(info.hstmt);
    }
}

void PreparedSQL::SetBindColumn(STBindColumn * _arrP, size_t _cnt) {

    colInfo_.ptr = _arrP;
    colInfo_.cnt = _cnt;

    for(size_t nLoop=0; nLoop < vecStmtInfo_.size(); ++nLoop) {
        stStmtInfo & info = vecStmtInfo_.at(nLoop);

        if(info.hstmt != SQL_NULL_HSTMT)
            bindColumn(info.hstmt);
    }
}

const char * PreparedSQL::GetSQL() {
    return sql_.c_str();
}

size_t PreparedSQL::GetSQLSize() {
    return sql_.size();
}

void PreparedSQL::SetFunc(std::function<bool(SQLHSTMT & _stmt)> _func) {
    func_ = _func;
}

SQLHSTMT & PreparedSQL::AllocStatement(Connection * _c, size_t _shardingNum) {

    /*-
    if(_shardingNum >= vecStmtInfo_.size()) {
        return SQL_NULL_HSTMT;
    }
    -*/

    stStmtInfo & _info = vecStmtInfo_.at(_shardingNum);
    unsigned long uuidForConnection = _c->GetUuid();

    if(_info.hstmt == SQL_NULL_HSTMT || uuidForConnection != _info.uuidForConnection) {

        I_THD_LOG(gLogName, 
                "---- 1. [%zu] [%p] [%s] PreparedSQL.AllocStatement.AllocStmt.before [%s:%d] [%zu] start uuid[%zu:%zu] [%s]",
                syscall(SYS_gettid),
                _info.hstmt,
                GetClassName(),
                _c->GetIp(),
                _c->GetPort(),
                _shardingNum,
                _info.uuidForConnection,
                uuidForConnection,
                (_info.hstmt == SQL_NULL_HSTMT)?"null":"not null");
                

        // 2021.10.07 - 
        /*--
        // Stmt 를 다시 만들어야 합니다.
        if(_info.hstmt != SQL_NULL_HSTMT) {
            I_THD_LOG(gLogName, 
                    "---- 1-1. [%zu] [%p] [%s] PreparedSQL.AllocStatement.SQLFreeHandle [%s:%d] [%zu] conn [%zu]",
                    syscall(SYS_gettid),
                    _info.hstmt,
                    GetClassName(),
                    _c->GetIp(),
                    _c->GetPort(),
                    _shardingNum,
                    _info.uuidForConnection);
            
            SQLFreeHandle(SQL_HANDLE_STMT, _info.hstmt);
            _info.hstmt = SQL_NULL_HSTMT;
        }
        --*/

        if(_c->AllocStmt(_info.hstmt) == false) {
            sret_ = _c->GetSQLReturn();

            W_THD_LOG(gLogName, "PreparedSQL.AllocStatement.AllocStmt [%s:%d] fail [%s]",
                _c->GetIp(),
                _c->GetPort(),
                ODBCErr::GetStringSTMT(_info.hstmt).c_str());

            _info.hstmt = SQL_NULL_HSTMT;
            return _info.hstmt;
        }

        if(bPreparedUsed_) {
            sret_ = SQLPrepare(_info.hstmt, (SQLCHAR *)sql_.c_str(), sql_.size());

            if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {

                W_THD_LOG(gLogName, "PreparedSQL.AllocStatement.SQLPrepare [%s:%d] fail [%d] [%s]",
                    _c->GetIp(),
                    _c->GetPort(),
                    sret_,
                    ODBCErr::GetStringSTMT(_info.hstmt).c_str());

                _info.hstmt = SQL_NULL_HSTMT;
                return _info.hstmt;
            }
        }

        // For Debugging
        unsigned long olduuid = _info.uuidForConnection;
        bool bparam = false;
        bool bcol = false; 

        _info.uuidForConnection = uuidForConnection;

        if(paramInfo_.cnt > 0)
            bparam = bindParameter(_info.hstmt);

        if(colInfo_.cnt > 0)
            bcol = bindColumn(_info.hstmt);

        I_THD_LOG(gLogName, "2. [%zu] [%p] [%s] PreparedSQL.AllocStatement.SQLPrepare [%s:%d] conn id[%zu->%zu] [%d:%d] prepared[%d]",
            syscall(SYS_gettid),
            _info.hstmt,
            GetClassName(),
            _c->GetIp(),
            _c->GetPort(),
            olduuid,
            _info.uuidForConnection,
            bparam,
            bcol,
            bPreparedUsed_);

        // 2021.10.06, TEST
        AllPrint();
    }

    return _info.hstmt;
}

void PreparedSQL::CloseStatement(SQLHSTMT & _stmt) {
    if(_stmt != SQL_NULL_HSTMT) {
        SQLFreeStmt(_stmt, SQL_CLOSE);
    }
}

/*-
void PreparedSQL::DropStatement(SQLHSTMT & _stmt) {

    if(_stmt != SQL_NULL_HSTMT) {

        I_THD_LOG(gLogName, 
            "---- 5. [%zu] [%p] [%s] PreparedSQL.AllocStatement.SQLFreeHandle",
                    syscall(SYS_gettid),
                    _stmt,
                    GetClassName());
        
        SQLFreeHandle(SQL_HANDLE_STMT, _stmt);
        _stmt = SQL_NULL_HSTMT;
    }
}
-*/

bool PreparedSQL::Execute(SQLHSTMT & _hstmt) {
    return func_(_hstmt);
}

bool PreparedSQL::BindParameter(SQLHSTMT & _hstmt,
                                STBindParameter * _arrP,
                                size_t _cnt) {

    paramInfo_.ptr = _arrP;
    paramInfo_.cnt = _cnt;

    return bindParameter(_hstmt);
}

bool PreparedSQL::BindColumn(SQLHSTMT & _hstmt,
                            STBindColumn * _arrP,
                            size_t _cnt) {

    colInfo_.ptr = _arrP;
    colInfo_.cnt = _cnt;

    return bindColumn(_hstmt);
}

bool PreparedSQL::bindParameter(SQLHSTMT & _stmt) {

    STBindParameter * arr = paramInfo_.ptr;
    size_t  arrCnt = paramInfo_.cnt;

    if(arr == nullptr || arrCnt == 0) {
        W_THD_LOG(gLogName, "PreparedSQL.bindParameter invalid arguemnt [%s] [%p:%zu]",
            GetSQL(), arr, arrCnt);
        return false;
    }

    SQLFreeStmt(_stmt, SQL_RESET_PARAMS);

    for(size_t i=0; i < arrCnt; ++i) {
        sret_ = SQLBindParameter(_stmt,
                    arr[i].ipar,
                    SQL_PARAM_INPUT,
                    arr[i].fCType,
                    arr[i].fSQLType,
                    arr[i].pLen,
                    arr[i].pScale,
                    arr[i].pData,
                    arr[i].cbValueMax,
                    arr[i].cbValueLen);

        if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
            W_THD_LOG(gLogName, "PreparedSQL.bindParameter [%s] [%d] [%s]",
                GetSQL(),
                sret_,
                ODBCErr::GetStringSTMT(_stmt).c_str());

            SQLFreeStmt(_stmt, SQL_RESET_PARAMS);
            return false;
        }
    }

    // I_THD_LOG(gLogName, "-- bindParameter Success [%p] [%s]", (void *)_stmt, GetSQL());
    return true;

}

bool PreparedSQL::bindColumn(SQLHSTMT & _stmt) {
    STBindColumn * arr = colInfo_.ptr;
    size_t arrCnt = colInfo_.cnt;

    if(arr == nullptr || arrCnt == 0) {
        W_THD_LOG(gLogName, "PreparedSQL.bindColumn invalid arguemnt [%s] [%p:%zu]",
            GetSQL(), arr, arrCnt);
        return false;
    }

    SQLFreeStmt(_stmt, SQL_UNBIND);

    for(size_t i=0; i < arrCnt; ++i) {
        sret_ = SQLBindCol(_stmt,
                    arr[i].ipar,
                    arr[i].fCType,
                    arr[i].pData,
                    arr[i].cbValueMax,
                    arr[i].pInd);

        if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
            W_THD_LOG(gLogName, "PreparedSQL.bindColumn [%s] [%d] [%s]",
                GetSQL(),
                sret_,
                ODBCErr::GetStringSTMT(_stmt).c_str());
            SQLFreeStmt(_stmt, SQL_UNBIND);
            return false;
        }
    }

    // I_THD_LOG(gLogName, "-- bindColumn Success [%p] [%s]", (void *)_stmt, GetSQL());

    return true;
}

}
