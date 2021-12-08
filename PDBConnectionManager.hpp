#ifndef PDB_CONNECTION_MANAGER_HPP
#define PDB_CONNECTION_MANAGER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>

#include "PDBDefine.hpp"
#include "PDBConnectionCluster.hpp"
#include "PDBConfig.hpp"
#include "PDBAlarmDeque.hpp"
#include "PDBConnectionMonitorTask.hpp"

namespace PDB {

class ConnectionManager {
public:
    explicit ConnectionManager();
    ConnectionManager(const ConnectionManager & _rhs) = delete;
    ~ConnectionManager();

    void SetLogName(std::string _logName) { strncpy(gLogName, _logName.c_str(), sizeof(gLogName)-1); }
    bool Init(std::string _nodeId, std::string _procName, std::string _confName);

    int SwitchActiveDB(eDefDBType _pdbT, eDefClusterType _clusterT, size_t _shardingNum=0);
    int SwitchActiveDB(std::string _json);
    void PushAlarm(const std::string & _value, const Connection * _c);
    bool PopAlarm(std::string & _jData);


    size_t GetShardingCnt(eDefDBType _pdbT);
    int SetConfigAndTryConnect(ConnectionCluster * _cc, size_t _clusterNum, bool _bAutoCommit);
    void AddMonitorList(std::shared_ptr<ConnectionCluster> _spCC, std::memory_order=std::memory_order_seq_cst);
    bool IsFailOverReason(std::string & _state);

    size_t FindShardingIndex(const char * _shardingKey, size_t _shardingKeyLength);

    void SetHangupValidTime(int _settime); 
    bool IsHangup() { return task_.IsHangup(); }

    std::shared_ptr<ConnectionCluster>  GetConnectionCluster(size_t _clusterNum, eDefDBType _pdbT, size_t _shardingNum);
    void ReleaseConnectionCluster(size_t _clusterNum);

private:
    std::string         nodeId_;
    std::string         procName_;
    std::string         confName_;

    Config              config_;
    AlarmDeque          alarm_;

    std::mutex          mutex_;
    ConnectionClusterArray  ccArr_;

    ConnectionMonitorTask   task_;
};

}
using SHAREDPTR_PDB_CONNMGR = std::shared_ptr<PDB::ConnectionManager>;

#endif // PDB_CONNECTION_MANAGER_HPP
