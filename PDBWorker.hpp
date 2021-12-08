#ifndef PDB_WORKER_HPP
#define PDB_WORKER_HPP

#include <array>
#include <memory>
#include <sql.h>
#include <sqlext.h>

#include "PDBDefine.hpp"
#include "PDBConnectionManager.hpp"
#include "PDBConnectionCluster.hpp"
#include "PDBPreparedSQL.hpp"

#include "PDBException.hpp"
#include "PDBTransactionWorker.hpp"

namespace PDB {

class Worker {
public:
    explicit Worker();
    virtual ~Worker();

    void SetLogName(std::string _logName) {
      strncpy(gLogName, _logName.c_str(), sizeof(gLogName)-1);
    }
    void Assign(std::shared_ptr<ConnectionManager>     _mgr, std::memory_order=std::memory_order_seq_cst) {
        mgr_ = atomic_load(&_mgr);
    }
    bool TurnOn(eDefDBType      _pdbT, bool _bInterProcesses = false, bool _bAutoCommit = true);
    void TurnOff(eDefDBType     _pdbT);
    bool DirectExecute(eDefDBType   _pdbT,
                       const char * _sql,
                       size_t       _sqlLength,
                       const char * _shardingKey = nullptr,
                       size_t       _shardingKeyLength = 0);
    bool DirectQuery(eDefDBType   _pdbT,
                     const char * _sql,
                     size_t       _sqlLength,
                     std::function<bool(SQLHSTMT & _stmt)> _func,
                     const char * _shardingKey = nullptr,
                     size_t       _shardingKeyLength = 0);

    bool Execute(PreparedSQL & _ps,
                const char * _shardingKey = nullptr,
                size_t _shardingKeyLength = 0);

    // 20.08.06 LTE PCRF 의 세션 TBL 을 위함..
    bool Execute(PreparedSQL & _ps, int _sessTableNumber);

    // 21.07.19 Statement 를 close 하기 위함.
    bool ResetStmt(PreparedSQL & _ps,
                   const char * _shardingKey = nullptr,
                   size_t _shardingKeyLength = 0);

    bool ResetStmt(PreparedSQL & _ps, int _sessTableNumber);

    Worker & Transaction(PreparedSQL & _ps,
                                    const char * _shardingKey = nullptr,
                                    size_t _shardingKeyLength = 0) throw (NotFoundConnectionE, InvalidConnectionE, AllocStatementE, ExecuteE);
    Worker & Next(PreparedSQL & _ps) throw (InvalidConnectionE, AllocStatementE, ExecuteE);
    bool Commit();
    void Rollback();

    const char * GetSQLState() { return state_.c_str(); }

    // 20.08.06 LTE PCRF 의 세션 TBL 을 위함.
    int GetNumberOfSessionTBL(const char * _min, size_t _minLength);

    // 20.08.06 LTE PCRF 의 세션 TBL 개수 반환을 위함.
    int GetTotalLTESessionTBLCnt() { return maxLTESessionTBLCnt_; } 


private:
    bool execute(PreparedSQL & _ps,
                ConnectionCluster * _cc,
                int _tryCnt);
    ConnectionCluster * getConnectionCluster(eDefDBType & _pdbT,
                                            const char * _shardingKey,
                                            size_t _shardingKeyLength);

    // 20.08.06 LTE PCRF 의 세션 TBL 을 위함.
    ConnectionCluster * getConnectionCluster(int _sessTableNumber);

    // 21.07.19 - Statement 를 close 하기 위함.
    bool resetStmt(PreparedSQL & _ps, ConnectionCluster * _cc);

private:

    std::string   state_;

    std::shared_ptr<ConnectionManager>     mgr_;

    ConnectionClusterArray  ccArr_;

    TransactionWorker   trans_;
    int     maxExecuteTryCnt_;

    // 20.08.06 LTE PCRF 의 세션 TBL 을 위함.
    int     maxLTESessionTBLCnt_ = 24;

};


}


#endif // PDB_WORKER_HPP
