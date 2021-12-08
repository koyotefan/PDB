
#include <unordered_map>

#include "PDBConnectionManager.hpp"
#include "PDBOdbcErr.hpp"

thread_local     char gLogName[32] = {"MAIN"};

namespace PDB {

ConnectionManager::ConnectionManager()
    : task_(alarm_) {

}

ConnectionManager::~ConnectionManager() {

}

bool ConnectionManager::Init(std::string _nodeId, std::string _procName, std::string _confName) {

    nodeId_   = _nodeId;
    procName_ = _procName;
    confName_ = _confName;

    if(config_.Init(_confName) == false) {
        W_THD_LOG(gLogName, "ConnectionManager.Init fail");
        return false;
    }

    task_.Init(nodeId_,
               procName_,
               config_.GetPingCheckPeriod(),
               config_.GetMonitorPeriod(),
               config_.GetConnectionInfo(static_cast<size_t>(eDefDBType::PCF), eDefClusterType::P),
               config_.GetConnectionInfo(static_cast<size_t>(eDefDBType::PCF), eDefClusterType::S)
              );
    task_.Run();

    return true;
}

size_t ConnectionManager::GetShardingCnt(eDefDBType _pdbT) {

    if(_pdbT < eDefDBType::SessionDetail)
        return 1;

    return config_.GetSessionDetailShardingCnt();
}

int ConnectionManager::SetConfigAndTryConnect(ConnectionCluster * _cc, size_t _clusterNum, bool _bAutoCommit) {

    int nRet = 0;

    // Config 에서 연결 정보를 얻은 후, 넣어야 해요.
    size_t clusterCnt = static_cast<size_t>(eDefClusterType::Cnt);

    for(size_t clusterT =0; clusterT < clusterCnt; ++clusterT) {
        Connection * c = _cc->Get(static_cast<eDefClusterType>(clusterT));

        stConnectionInfo & info = config_.GetConnectionInfo(_clusterNum,
                                                            static_cast<eDefClusterType>(clusterT));


        // 2021.09.14 by jhchoi. IP 설정이 없으면, 연결 관리를 하지 않아요. - SESS.PUSH 에러 로그 때문.
        // SBI 만 있는 경우, PG Local DB 설정이 안되면, 1 sec 마다 로그를 찍어서요.
        if(info.DBName.empty() || 
           info.DSN.empty() ||
           info.Host.empty() || 
           info.Id.empty() || 
           info.Pwd.empty()) {

            W_THD_LOG(gLogName, "ConnectionManager.SetConfigAndTryConnect.clusterT Fail [%s:%s:%s:%s:%s]",
                info.DBName.c_str(),
                info.DSN.c_str(),
                info.Host.c_str(),
                info.Id.c_str(),
                info.Pwd.c_str());
    
            continue;
        }

        c->Set(info);
        c->SetAutoCommit(_bAutoCommit);
        //D_THD_LOG(gLogName, "## SetConfigAndTryConnect clusterT [%zu] [%s:%d] [%s:%d]",
        //    _clusterNum, info.Host.c_str(), info.nPort, c->GetIp(), c->GetPort());
    

        if(c->Connect(task_.GetUuidForConnection()) == false) {
            E_THD_LOG(gLogName,
                "ConnectionManager.SetConfigAndTryConnect Connect [%zu] [%s:%d]",
                _clusterNum, c->GetIp(), c->GetPort());
        }

        ++nRet;
    }

    return nRet;
}

void ConnectionManager::AddMonitorList(std::shared_ptr<ConnectionCluster> _spCC, std::memory_order) {
    task_.Add(_spCC);
}

bool ConnectionManager::IsFailOverReason(std::string & _state) {
    return config_.IsFailOverReason(_state);
}


size_t ConnectionManager::FindShardingIndex(const char * _shardingKey, size_t _shardingKeyLength) {

    if(_shardingKey == nullptr)
        return 0;

    if(_shardingKeyLength < 2)
        return 0;

    size_t  shardingCnt = config_.GetSessionDetailShardingCnt();

    size_t  val =
        (_shardingKey[_shardingKeyLength-2] - '0')*10 +
        (_shardingKey[_shardingKeyLength-1] - '0');

    return val % shardingCnt;
}


/*-
size_t ConnectionManager::FindShardingIndexForLTE_PCRF(const char * _shardingKey, size_t _shardingKeyLength) {

    if(_shardingKey == nullptr)
        return 0;

    if(_shardingKeyLength < 10)
        return 0;

    size_t  shardingCnt = config_.GetSessionDetailShardingCnt();

    size_t  val =
        (_shardingKey[_shardingKeyLength-2] - '0')*10 +
        (_shardingKey[_shardingKeyLength-1] - '0');

    return val % shardingCnt;
}
-*/

int ConnectionManager::SwitchActiveDB(eDefDBType _pdbT, eDefClusterType _clusterT, size_t _shardingNumber) {
    return task_.SwitchActiveDB(_pdbT, _clusterT, _shardingNumber);
}

int ConnectionManager::SwitchActiveDB(std::string _json) {

    // 1. Parsing
    // 2. _pdbT 과 _clusterT 값을 얻습니다.
    // 3. SwitchActiveDB() 를 호출합니다.
    // 변경 사항이 있다면 Count 하여 int 형으로 반환 합니다.
    // 바뀌는 과정에서 Alarm 을 써야 합니다.

    // DBClusterName : DBInstanceName
    // DBClusterName - ex) Subscriber, SessionDetail_0,
    // DBInstanceName- ex) Subscriber_P, SessionDetail_0_S,
    std::unordered_map<std::string, std::string>     mapActiveStatus;
    if(Config::Parsing_PCF_PDB_ACTIVE(mapActiveStatus, _json) == false) {
        return -1;
    }

    int changedCnt = 0;

    for(const auto & citer : mapActiveStatus) {
        eDefDBType      pdbT            = Config::ExtractDBType(citer.first);
        size_t          shardingNumber  = Config::ExtractShardingNumber(citer.first);
        eDefClusterType clusterType     = Config::ExtractClusterType(citer.second);

        // JUST TEST
        int ret = SwitchActiveDB(pdbT, clusterType, shardingNumber);

        if(ret < 0) {
            W_THD_LOG(gLogName, "SwitchActiveDB return [%d]", ret);
        }

        changedCnt += ret;
        // Alarm
    }

    return changedCnt;
}


void ConnectionManager::PushAlarm(const std::string & _value, const Connection * _c) {

    alarm_.Push(nodeId_, _value, _c);
}

bool ConnectionManager::PopAlarm(std::string & _jData) {

    if(alarm_.GetCount() == 0)
        return false;

    return alarm_.Pop(_jData);
}

void ConnectionManager::SetHangupValidTime(int _settime) {

    int temp = (minHangupValidTime > _settime)?minHangupValidTime:_settime;
    task_.SetHangupValidTime(temp);

    if(_settime < minHangupValidTime) {
        I_THD_LOG(gLogName, 
            "[%d] is so small. hangup valid time is set [%d]", _settime, minHangupValidTime);
    }
}

std::shared_ptr<ConnectionCluster>  
ConnectionManager::GetConnectionCluster(size_t _clusterNum, eDefDBType _pdbT, size_t _shardingNum) {

    std::lock_guard<std::mutex>     lock(mutex_);

    // 등록 여부를 살펴서 등록되어 있다면, return 합니다.
    // 만약, 등록되지 않으면 등록 후, return 합니다.
    auto cc = ccArr_.GetSharedPtr(_clusterNum);

    if(!cc) {
        ccArr_.Assign(_pdbT, _shardingNum);
        cc = ccArr_.GetSharedPtr(_clusterNum);
    }

    return cc;
}

void ConnectionManager::ReleaseConnectionCluster(size_t _clusterNum) {

    std::lock_guard<std::mutex>     lock(mutex_);

    // ccArr_ 과 ConnectionMonitoringTask 경우
    if(ccArr_.GetSharedPtrUsedCount(_clusterNum) <= 2) {
        ccArr_.Release(_clusterNum); 
    }
}

}

