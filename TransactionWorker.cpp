
#include "PDBException.hpp"
#include "PDBTransactionWorker.hpp"
#include "PDBOdbcErr.hpp"

namespace PDB {

TransactionWorker::TransactionWorker() {

}

TransactionWorker::~TransactionWorker() {

}

void TransactionWorker::Start(Connection * _c, size_t _shardingNumber)
    throw(InvalidConnectionE) {

    c_ = _c;
    shardingNumber_ = _shardingNumber;

    //lock_ =  std::unique_lock<std::mutex>(c_->GetMutex(), std::adopt_lock);
    _c->Lock();

    if(c_->IsConnected() == false) {
        W_THD_LOG(gLogName, "TransactionWorker.start.IsConnected fail [%s:%d]",
            c_->GetIp(),
            c_->GetPort());

        throw InvalidConnectionE(_c->GetIp(), _c->GetPort());
    }

    uuidForConnection_ = c_->GetUuid();
}

void TransactionWorker::Execute(PreparedSQL & _ps)
    throw(InvalidConnectionE, AllocStatementE, ExecuteE) {


    if(c_->IsConnected() == false) {
        W_THD_LOG(gLogName, "Worker.execute.IsConnected [%s] fail [%s:%d]",
            _ps.GetClassName(),
            c_->GetIp(),
            c_->GetPort());

        throw   InvalidConnectionE(c_->GetIp(), c_->GetPort(), _ps.GetClassName());
    }

    if(c_->GetUuid() != uuidForConnection_) {
        W_THD_LOG(gLogName, "Worker.execute.GetUuid fail [%s:%d]",
            c_->GetIp(),
            c_->GetPort());

        throw   InvalidConnectionE(c_->GetIp(), c_->GetPort(), _ps.GetClassName());
    }

    SQLHSTMT hstmt = _ps.AllocStatement(c_, shardingNumber_);

    if(hstmt == SQL_NULL_HSTMT) {
        W_THD_LOG(gLogName, "TransactionWorker.Execute.AllocStatement [%s] fail [%s:%d]",
            _ps.GetClassName(),
            c_->GetIp(),
            c_->GetPort());

        c_->Disconnect();

        throw AllocStatementE(c_->GetIp(), c_->GetPort(), _ps.GetClassName());
    }

    if(_ps.Execute(hstmt) == true) {
        _ps.CloseStatement(hstmt);

        // SUCCESS
        return ;
    }

    state_ =  ODBCErr::GetStateSTMT(hstmt);

    W_THD_LOG(gLogName, "Worker.execute.Execute [%s] [%s:%d] fail [%s]",
        _ps.GetClassName(),
        c_->GetIp(),
        c_->GetPort(),
        ODBCErr::GetStringSTMT(hstmt).c_str());

    //_ps.DropStatement(hstmt);
    c_->ReleaseStmt(hstmt);

    throw ExecuteE(c_->GetIp(), c_->GetPort(), _ps.GetClassName(), state_);
}

bool TransactionWorker::Commit() {

    if(c_->Commit() == false) {
   
        //lock_.unlock();
        c_->UnLock();

        return false;
    }

    //lock_.unlock();
    c_->UnLock();

    return true;
}

void TransactionWorker::Rollback() {

    c_->Rollback();

    //lock_.unlock();
    c_->UnLock();
}


}
