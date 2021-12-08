#ifndef PDB_CONNECTION_MONITOR_TASK_HPP
#define PDB_CONNECTION_MONITOR_TASK_HPP

#include <memory>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <map>
#include <tuple>
#include <mutex>
#include <vector>

#include "PDBConnectionCluster.hpp"
#include "PDBStatusReporter.hpp"
#include "PDBAlarmDeque.hpp"

namespace PDB {

class ConnectionMonitorTask {
public:
    explicit ConnectionMonitorTask(AlarmDeque & _alarm);
    ~ConnectionMonitorTask();

    void Init(const std::string & _nodeId,
              const std::string & _procName,
              const int pingCheckPeriod,
              const int monitorPeriod,
              const stConnectionInfo & _stP,
              const stConnectionInfo & _stS);
    void Add(std::shared_ptr<ConnectionCluster> _sp, std::memory_order=std::memory_order_seq_cst);
    void Run();

    unsigned long   GetUuidForConnection() {
        // return uuidForConnection_.fetch_add(1);
        std::lock_guard<std::mutex>     lock(mutex_);
        return ++uuidForConnection_;
    }

    int SwitchActiveDB(eDefDBType _pdbT, eDefClusterType _clusterT, size_t _shardingNumber);

    void SetHangupValidTime(int _settime) {
        hangupValidTime_ = _settime;
    }

    bool IsHangup();

private:
    void run(std::shared_ptr<bool>  _pCounter);

    void checkConnection(ConnectionCluster * _cc);
    void pingCheck(Connection * _c);
    void moveFromWaitingListToMonitoringList();

    void saveActiveDB(eDefDBType _pdbT, eDefClusterType _clusterT, size_t _shardingNumber);
    void correctActiveDB(eDefDBType _pdbT, size_t _shardingNumber, ConnectionCluster * _cc);

private:

    std::shared_ptr<bool>   pCounter_;
    std::atomic_bool        bRunF_;
    std::thread             tid_;

    std::string             nodeId_;
    std::string             procName_;

    int                     hangupValidTime_; 
    time_t                  lastActionTime_;   

    int                     pingCheckPeriod_;
    int                     monitorPeriod_;

    time_t                  lastMonitoringT_;
    time_t                  lastSwitchActiveDBT_;

    unsigned long           uuidForConnection_;
    AlarmDeque &            alarm_;

    StatusReporter          reporter_;

    std::mutex  mutex_;
    std::vector<std::shared_ptr<ConnectionCluster>>    vecWaitingList_;
    std::vector<std::shared_ptr<ConnectionCluster>>    vecMonitoringList_;

    std::map<std::tuple<eDefDBType, size_t>, eDefClusterType>   lastActiveDB_;    
    
};

}


#endif // PDB_CONNECTION_MONITOR_TASK_HPP
