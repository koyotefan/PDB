#ifndef PDB_TRANSACTION_WORKER_HPP
#define PDB_TRANSACTION_WORKER_HPP

#include "PDBDefine.hpp"
#include "PDBPreparedSQL.hpp"
#include "PDBConnection.hpp"

namespace PDB {

class TransactionWorker {
public:
    explicit TransactionWorker();
    ~TransactionWorker();

    void Start(Connection * _c, size_t _shardingNumber) throw(InvalidConnectionE);
    void Execute(PreparedSQL & _ps) throw(InvalidConnectionE, AllocStatementE, ExecuteE);
    bool Commit();
    void Rollback();

private:

    Connection *    c_;
    unsigned long   uuidForConnection_;

    size_t  shardingNumber_;

    std::string     state_;

    std::unique_lock<std::mutex>    lock_;

};



}

#endif // PDB_TRANSACTION__WORKER_HPP
