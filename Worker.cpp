
#include <mutex>

#include "PDBWorker.hpp"
#include "PDBOdbcErr.hpp"
#include "PDBAlarmDeque.hpp"

namespace PDB {

Worker::Worker() {
    maxExecuteTryCnt_ = static_cast<int>(eDefClusterType::Cnt) + 1;
}

Worker::~Worker() {


}

bool Worker::TurnOn(eDefDBType     _pdbT, bool _bInterProcesses, bool  _bAutoCommit) {

    if(!mgr_) {
        W_THD_LOG(gLogName, "Worker.TurnOn not execute. Assign() is not called yet [%s]",
            ToString::Get(_pdbT));
       return false; 
    }

    size_t cnt = mgr_->GetShardingCnt(_pdbT);
    size_t clusterNum = 0;

    for(size_t shardingNum=0; shardingNum < cnt; ++shardingNum) {
        clusterNum = ccArr_.GetIndex(_pdbT, shardingNum);

        if(ccArr_.IsUsed(clusterNum)) {
            // 이미 연결된 타입의 연결을 다시 연결하지 않아요.
            D_THD_LOG(gLogName, "Worker.TurnOn already used [%s:%zu]",
                ToString::Get(_pdbT), shardingNum);
            continue;
        }

        if(_bInterProcesses) { 

            auto cc = mgr_->GetConnectionCluster(clusterNum, _pdbT, shardingNum);
            if(!cc) {
                W_THD_LOG(gLogName, "Worker.TurnOn unexpected value [%s:%zu]",
                    ToString::Get(_pdbT), shardingNum);
                continue;
            } 

            ccArr_.Assign(clusterNum, cc);

        } else {

            if(ccArr_.Assign(clusterNum, _pdbT, shardingNum) == false) {
                W_THD_LOG(gLogName, "Worker.TurnOn unexpected value [%s:%zu]",
                    ToString::Get(_pdbT), shardingNum);
                continue;
            }
        }

        I_THD_LOG(gLogName, "Worker.TurnOn [%s:%zu]", ToString::Get(_pdbT), shardingNum);

        if(mgr_->SetConfigAndTryConnect(ccArr_.Get(clusterNum), clusterNum, _bAutoCommit) > 0)
            mgr_->AddMonitorList(ccArr_.GetSharedPtr(clusterNum));
    }


    maxLTESessionTBLCnt_ = 
        PDB::sessionTBLCntByInstance * mgr_->GetShardingCnt(eDefDBType::SessionDetail);

    return true;

/*- 
    size_t typeIndex = static_cast<size_t>(_pdbT);
    size_t cnt = mgr_->GetShardingCnt(_pdbT);

    for(size_t shardingNum=0; shardingNum < cnt; ++shardingNum) {
        size_t clusterNum = typeIndex + shardingNum;
        if(arr_[clusterNum].use_count() > 0) {
            // 이미 연결된 타입의 연결을 다시 연결하지 않아요.
            D_THD_LOG(gLogName, "Worker.TurnOn already used [%zu:%zu]",
                typeIndex, shardingNum);
            continue;
        }

        I_THD_LOG(gLogName, "Worker.TurnOn [%zu:%zu]", typeIndex, shardingNum);

        arr_[clusterNum] =
            std::make_shared<ConnectionCluster>(_pdbT, shardingNum);

        mgr_->SetConfigAndTryConnect(arr_[clusterNum].get(), clusterNum, _bAutoCommit);
        mgr_->AddMonitorList(arr_[clusterNum]);
    }


    maxLTESessionTBLCnt_ = 
        PDB::sessionTBLCntByInstance * mgr_->GetShardingCnt(eDefDBType::SessionDetail);

    return true;
-*/

}

void Worker::TurnOff(eDefDBType     _pdbT) {

    size_t cnt = mgr_->GetShardingCnt(_pdbT);
    size_t clusterNum = 0;

    for(size_t shardingNum=0; shardingNum < cnt; ++shardingNum) {
        clusterNum = ccArr_.GetIndex(_pdbT, shardingNum);

        if(ccArr_.IsUsed(clusterNum) == false) {
            // MonitorTask 조차 관리하지 않아요.
            D_THD_LOG(gLogName, "Worker.TurnOff already not used [%s:%zu]",
                ToString::Get(_pdbT), shardingNum);
            continue;
        }

        // MonitorTask 가 알아서 잘 끊고 정리 합니다. 걱정 마세요.
        ccArr_.Release(clusterNum);

        // 만약, SharedPtr 의 use_count 가 2 면, {} 를 할당해야 합니다.
        // 그래야, 정리 가능해요.
        mgr_->ReleaseConnectionCluster(clusterNum);

        I_THD_LOG(gLogName, "Worker.TurnOff [%s:%zu]", ToString::Get(_pdbT), shardingNum);
    }

    /*-
    size_t typeIndex = static_cast<size_t>(_pdbT);
    size_t cnt = mgr_->GetShardingCnt(_pdbT);

    for(size_t shardingNum=0; shardingNum < cnt; ++shardingNum) {
        size_t clusterNum = typeIndex + shardingNum;

        if(arr_[clusterNum].use_count() == 0) {
            D_THD_LOG(gLogName, "Worker.TurnOff already not used [%zu:%zu]",
                typeIndex, shardingNum);
            continue;
        }

        I_THD_LOG(gLogName, "Worker.TurnOff [%zu:%zu]", typeIndex, shardingNum);

        // MonitorTask 가 알아서 잘 끊고 정리 합니다. 걱정 마세요.

        arr_[clusterNum] = {};
    }
    -*/
}

bool Worker::DirectExecute(eDefDBType _pdbT,
                           const char * _sql,
                           size_t  _sqlLength,
                           const char * _shardingKey,
                           size_t   _shardingKeyLength) {

    auto _func
        = [](SQLHSTMT & _stmt) -> bool {

        SQLRETURN sret = SQLExecute(_stmt);

        if(sret == SQL_SUCCESS || sret == SQL_NO_DATA || sret == SQL_SUCCESS_WITH_INFO)
            return true;

        W_THD_LOG(gLogName, "Worker.DirectExecute.SQLExecute fail [%d:%s]",
            sret,
            ODBCErr::GetStringSTMT(_stmt).c_str());

        return false;
    };

    return DirectQuery(_pdbT, _sql, _sqlLength, _func, _shardingKey, _shardingKeyLength);

}

bool Worker::DirectQuery(eDefDBType     _pdbT,
                           const char * _sql,
                           size_t       _sqlLength,
                           std::function<bool(SQLHSTMT & _stmt)> _func,
                           const char * _shardingKey,
                           size_t       _shardingKeyLength) {

    PreparedSQL  ps;

    if(ps.SetSQL(_pdbT, _sql, _sqlLength) == false) {
        E_THD_LOG(gLogName, "Worker.DirectExecute.SetSQL is fail [%p:%zu] [%s:%zu]",
            _sql, _sqlLength, _shardingKey, _shardingKeyLength);
        return false;
    }

    ps.SetFunc(_func);

    return Execute(ps, _shardingKey, _shardingKeyLength);

}


bool Worker::Execute(PreparedSQL & _ps, const char * _shardingKey, size_t _shardingKeyLength) {

    // 선택 해야죠..
    eDefDBType  pdbT = _ps.GetDBType();

    //int maxTryCnt = static_cast<int>(eDefClusterType::Cnt) + 1;

    if(_shardingKey != nullptr && _shardingKeyLength == 0)
        _shardingKeyLength = strlen(_shardingKey);

    ConnectionCluster * cc = getConnectionCluster(pdbT,
                                                  _shardingKey,
                                                  _shardingKeyLength);

    if(cc == nullptr)
        return false;

    return execute(_ps, cc, maxExecuteTryCnt_);
}

bool Worker::ResetStmt(PreparedSQL & _ps, const char * _shardingKey, size_t _shardingKeyLength) {

    // 선택 해야죠..
    eDefDBType  pdbT = _ps.GetDBType();

    //int maxTryCnt = static_cast<int>(eDefClusterType::Cnt) + 1;

    if(_shardingKey != nullptr && _shardingKeyLength == 0)
        _shardingKeyLength = strlen(_shardingKey);

    ConnectionCluster * cc = getConnectionCluster(pdbT,
                                                  _shardingKey,
                                                  _shardingKeyLength);

    if(cc == nullptr)
        return false;

    return resetStmt(_ps, cc);
}

ConnectionCluster * Worker::getConnectionCluster(eDefDBType & _pdbT, const char * _shardingKey, size_t _shardingKeyLength) {

    size_t  shardingIndex = 0;

    if(_pdbT == eDefDBType::SessionDetail) {
        if(_shardingKey == nullptr || _shardingKeyLength == 0) {
            W_THD_LOG(gLogName, "Worker.getConnectionCluster [%s] [%s:%zu]",
                ToString::Get(_pdbT),
                _shardingKey,
                _shardingKeyLength);
            return nullptr;
        }

        shardingIndex = mgr_->FindShardingIndex(_shardingKey, _shardingKeyLength);
    }

    // ConnectionCluster * cc = arr_[static_cast<size_t>(_pdbT) + shardingIndex].get();
    ConnectionCluster * cc = ccArr_.Get(_pdbT, shardingIndex);

    if(cc == nullptr) {
        W_THD_LOG(gLogName, "Worker.getConnectionCluster fail [%s:%zu] [%s:%zu]",
            ToString::Get(_pdbT),
            shardingIndex,
            _shardingKey,
            _shardingKeyLength);
        return nullptr;
    }

    return cc;
}

bool Worker::Execute(PreparedSQL & _ps, int _sessTableNumber) {

    if(_sessTableNumber < 0 || _sessTableNumber >= maxLTESessionTBLCnt_) {
        E_THD_LOG(gLogName, "Worker.Execute.sessTableNumber is fail [%d:%d]",
            _sessTableNumber, maxLTESessionTBLCnt_);
        return false;
    } 

    ConnectionCluster * cc = getConnectionCluster(_sessTableNumber);

    if(cc == nullptr)
        return false;

    return execute(_ps, cc, maxExecuteTryCnt_);
}

bool Worker::ResetStmt(PreparedSQL & _ps, int _sessTableNumber) {

    if(_sessTableNumber < 0 || _sessTableNumber >= maxLTESessionTBLCnt_) {
        E_THD_LOG(gLogName, "Worker.ResetStmt.sessTableNumber is fail [%d:%d]",
            _sessTableNumber, maxLTESessionTBLCnt_);
        return false;
    } 

    ConnectionCluster * cc = getConnectionCluster(_sessTableNumber);

    if(cc == nullptr)
        return false;

    return resetStmt(_ps, cc);
}

ConnectionCluster * Worker::getConnectionCluster(int _sessTableNumber) {

    size_t  shardingIndex = 0;
   
    if(PDB::sessionTBLCntByInstance != 0)
        shardingIndex = _sessTableNumber / PDB::sessionTBLCntByInstance;

    // ConnectionCluster * cc = arr_[static_cast<size_t>(eDefDBType::SessionDetail) + shardingIndex].get();
    ConnectionCluster * cc = ccArr_.Get(eDefDBType::SessionDetail, shardingIndex);

    if(cc == nullptr) {
        W_THD_LOG(gLogName, "Worker.getConnectionCluster fail [%d]",
            _sessTableNumber);
        return nullptr;
    }

    return cc;
}

bool Worker::resetStmt(PreparedSQL & _ps, ConnectionCluster * _cc) {

    Connection * c = _cc->Get();
    size_t shardingNumber = _cc->GetShardingNumber();

    if(c == nullptr) {
        E_THD_LOG(gLogName, "Worker.resetStmt.Get [%s] fail [%zu:%zu]",
            _ps.GetClassName(),
            _cc->GetDBType(),
            shardingNumber);

        return false;
    }

    I_THD_LOG(gLogName, "[%zu] [%s] Worker.resetStmt.call [%s:%d] conn [%zu]",
            syscall(SYS_gettid),
            _ps.GetClassName(),
            c->GetIp(),
            c->GetPort(),
            c->GetUuid());

    c->Lock();

    SQLHSTMT & hstmt = _ps.AllocStatement(c, shardingNumber);

    if(hstmt == SQL_NULL_HSTMT) {
        W_THD_LOG(gLogName, "[%zu] [%s] Worker.resetStmt.AllocStatement fail [%s:%d] conn [%zu]",
            syscall(SYS_gettid),
            _ps.GetClassName(),
            c->GetIp(),
            c->GetPort(),
            c->GetUuid());

        c->Disconnect();
        c->UnLock();

        mgr_->PushAlarm(PDB::DISCONNECT, c);

        return false;
    }

    c->ReleaseStmt(hstmt);
    c->UnLock();

    return true;

}


bool Worker::execute(PreparedSQL & _ps, ConnectionCluster * _cc, int _tryCnt) {

    if(_tryCnt <= 0) {
        E_THD_LOG(gLogName, "Worker.execute.TryCount [%s] over [%d]",
            _ps.GetClassName(),
            _tryCnt);
        return false;
    }

    Connection * c = _cc->Get();
    size_t shardingNumber = _cc->GetShardingNumber();

    if(c == nullptr) {
        E_THD_LOG(gLogName, "Worker.execute.Get [%s] fail [%zu:%zu]",
            _ps.GetClassName(),
            _cc->GetDBType(),
            shardingNumber);

        return false;
    }

    c->Lock();

    // log
    /*- 
    I_THD_LOG(gLogName, "---- [%zu] Worker.execute.Execute.BeforeCall.Alloc [%s:%p] [%s:%d] [%zu]",
            syscall(SYS_gettid),
            _ps.GetClassName(),
            (unsigned char *)&_ps,
            c->GetIp(),
            c->GetPort(),
            c->GetUuid());
     -*/

    SQLHSTMT & hstmt = _ps.AllocStatement(c, shardingNumber);

    if(hstmt == SQL_NULL_HSTMT) {
        W_THD_LOG(gLogName, "Worker.execute.AllocStatement [%s] fail [%s:%d]",
            _ps.GetClassName(),
            c->GetIp(),
            c->GetPort());

        c->Disconnect();
        c->UnLock();

        mgr_->PushAlarm(PDB::DISCONNECT, c);

        return execute(_ps, _cc, --_tryCnt);
    }

    if(_ps.Execute(hstmt) == true) {
        _ps.CloseStatement(hstmt);
        c->UnLock();

        return true;
    }

    state_ =  ODBCErr::GetStateSTMT(hstmt);


    if(mgr_->IsFailOverReason(state_)) {
        /*-
        원래는 절체 대상 Error 를 확인하고자 찍었으나, 라이브러리에서 빼더라도 Appl. 에서의 로그로
        파악이 될 듯하여 삭제 합니다.
        -*/

        W_THD_LOG(gLogName, "3. Worker.execute.Execute [%s] [%s:%d] fail [%s]",
            _ps.GetClassName(),
            c->GetIp(),
            c->GetPort(),
            ODBCErr::GetStringSTMT(hstmt).c_str());

        // 에러가 발생한 Connection 이 여전히 Active 인 경우 예요.
        _cc->SetStandbyDB(c);

        //_ps.DropStatement(hstmt);
        c->ReleaseStmt(hstmt);
        c->Disconnect();
        c->UnLock();

        mgr_->PushAlarm(PDB::DISCONNECT, c);
        return execute(_ps, _cc, --_tryCnt);
    }

    SQLReturn sret = _ps.GetSQLReturn();

    if(sret != SQL_ERROR &&
       sret != SQL_SUCCESS &&
       sret != SQL_SUCCESS_WITH_INFO &&
       sret != SQL_NO_DATA) {

        _ps.CloseStatement(hstmt);
        c->ReleaseStmt(hstmt);

    } else {

        _ps.CloseStatement(hstmt);
    }

    /*-
    // log 
    W_THD_LOG(gLogName, "---- 4. [%zu] [%p] [%s] Worker.execute.Execute [%d] [%s:%d] [%zu] fail [%d] [%s]",
            syscall(SYS_gettid),
            hstmt,
            _ps.GetClassName(),
            _tryCnt,
            c->GetIp(),
            c->GetPort(),
            c->GetUuid(),
            _ps.GetSQLReturn(),
            ODBCErr::GetStringSTMT(hstmt).c_str());
    -*/
    
    c->UnLock();

    return false;
}

Worker & Worker::Transaction(PreparedSQL & _ps,
                             const char *  _shardingKey,
                             size_t        _shardingKeyLength)
    throw (NotFoundConnectionE, InvalidConnectionE, AllocStatementE, ExecuteE) {

    eDefDBType  pdbT = _ps.GetDBType();

    if(_shardingKey != nullptr && _shardingKeyLength == 0)
        _shardingKeyLength = strlen(_shardingKey);

    ConnectionCluster * cc = getConnectionCluster(pdbT,
                                                  _shardingKey,
                                                  _shardingKeyLength);

    if(cc == nullptr) {
        size_t      s = static_cast<size_t>(pdbT);
        W_THD_LOG(gLogName, "Worker.Transaction.getConnectionCluster [%s] [%d:%s] fail",
            _ps.GetClassName(),
            s,
            _shardingKey);
        throw NotFoundConnectionE(s, _shardingKey, _ps.GetClassName());
    }

    Connection * c = cc->Get();
    size_t shardingNumber = cc->GetShardingNumber();

    if(c == nullptr) {
        size_t      s = static_cast<size_t>(pdbT);

        W_THD_LOG(gLogName, "Worker.Transaction.Get [%s] [%d:%d] fail",
            _ps.GetClassName(),
            s,
            shardingNumber);
        throw NotFoundConnectionE(s, shardingNumber, _ps.GetClassName());
    }

    trans_.Start(c, shardingNumber);
    trans_.Execute(_ps);
    return *this;
}

Worker & Worker::Next(PreparedSQL & _ps)
    throw (InvalidConnectionE, AllocStatementE, ExecuteE) {

    trans_.Execute(_ps);
    return *this;
}

bool Worker::Commit() {
    return trans_.Commit();
}

void Worker::Rollback() {
    trans_.Rollback();
}

int Worker::GetNumberOfSessionTBL(const char * _min, size_t _minLength) {

    if(_min == nullptr || _minLength < 10) {
        W_THD_LOG(gLogName, "Worker.GetNumberOfSessionTBL.MIN invalid [%s:%u]",
            _min,
            _minLength);
        return -1;
    }

    return static_cast<int>(std::atol(_min) % maxLTESessionTBLCnt_);

}

}
