
#include "PDBDefine.hpp"
#include "PDBConnectionMonitorTask.hpp"
#include "PDBTimer.hpp"


namespace PDB {

ConnectionMonitorTask::ConnectionMonitorTask(AlarmDeque & _alarm)
    : bRunF_(true),
      hangupValidTime_(0),
      lastActionTime_(time(nullptr)),
      pingCheckPeriod_(30),
      monitorPeriod_(5),
      lastMonitoringT_(0),
      lastSwitchActiveDBT_(0),
      uuidForConnection_(1),
      alarm_(_alarm) {

}

ConnectionMonitorTask::~ConnectionMonitorTask() {

    bRunF_.store(false);

    if(tid_.joinable())
        tid_.join();

    // 내 이름으로 된 모든 연결은 DELETE 하고 나가야 해요..

}

void ConnectionMonitorTask::Init(const std::string & _nodeId,
                                 const std::string & _procName,
                                 const int _pingCheckPeriod,
                                 const int _monitorPeriod,
                                 const stConnectionInfo & _stP,
                                 const stConnectionInfo & _stS) {
    pingCheckPeriod_ = (_pingCheckPeriod <= 0)?30:_pingCheckPeriod;
    monitorPeriod_ = (_monitorPeriod <= 0)?5:_monitorPeriod;

    I_THD_LOG(gLogName, "ConnectionMonitorTask.Init pingCheckPeriod [%d] monitorPeriod [%d]",
        pingCheckPeriod_, monitorPeriod_);

    nodeId_   = _nodeId;
    procName_ = _procName;

    reporter_.Init(_nodeId, _procName, _stP, _stS);


    // 이따가 검토하고 할꺼예요.
    // whoIsActive_.Init(_stP, _stS);
    // whoIsAcive_.Clear();

    // statusReporter_.Init(_stP, _stS);

}

void ConnectionMonitorTask::Add(std::shared_ptr<ConnectionCluster> _sp, std::memory_order) {
    D_THD_LOG(gLogName, "ConnectionMonitorTask.MonitoringList.Add DB[%zu:%zu]",
        _sp->GetDBType(), _sp->GetShardingNumber());

    std::lock_guard<std::mutex>     guard(mutex_);
    vecWaitingList_.push_back(_sp);
}


void ConnectionMonitorTask::Run() {
    tid_ = std::thread(&ConnectionMonitorTask::run, this, pCounter_);
}

void ConnectionMonitorTask::run(std::shared_ptr<bool> _pCounter_) {
    (void)_pCounter_;

#ifndef __T_DEBUG
    strcpy(gLogName, "PDBMonitor");
    NDF_INIT_THD_LOG(gLogName);
#endif

    D_THD_LOG(gLogName, "ConnectionMonitorTask Start");

    lastActionTime_ = time(nullptr);

    Timer       sleepTimer(monitorPeriod_);
    sleepTimer.RandomOn();

    while(bRunF_) {

        if(sleepTimer.IsTimeout() == false) {
            // millisecond
            sleepTimer.Sleep(90);

            if(hangupValidTime_ != 0)
                lastActionTime_ = time(nullptr);

            continue;
        }

        moveFromWaitingListToMonitoringList();
        reporter_.Reset();

        for(auto iter = vecMonitoringList_.begin(); iter != vecMonitoringList_.end(); ) {

            std::shared_ptr<ConnectionCluster> & sp = *iter;

            // 나만 참조한다는 것은 서비스에서 제외되었다는 거예요.. 지워야죠..이런건..
            // 설마 이 연결이 Active 는 아니겠죠?
            if(sp.use_count() <= 1) {
                W_THD_LOG(gLogName, "ConnectionMonitorTask.MonitoringList.Erase DB[%zu:%zu]",
                    sp->GetDBType(), sp->GetShardingNumber());
                iter = vecMonitoringList_.erase(iter);
                continue;
            }

            checkConnection(sp.get());
            ++iter;

        }

        reporter_.Report();
;
    }

    I_THD_LOG(gLogName, "ConnectionMonitorTask is terminate");

}

bool ConnectionMonitorTask::IsHangup() {
    if(hangupValidTime_ == 0 || bRunF_ == false || tid_.joinable() == false)
        return false;

    return (time(nullptr) > (lastActionTime_ + hangupValidTime_));
}

void ConnectionMonitorTask::checkConnection(ConnectionCluster * _cc) {

    if(_cc == nullptr)
        return ;

    size_t dbType = _cc->GetDBType();
    size_t shardingNumber = _cc->GetShardingNumber();

    // correctActiveDB(static_cast<eDefDBType>(dbType), shardingNumber, _cc);

    // Cluster 를 보고, 각각 Connection 을 쳐다 봐요..
    size_t clusterCnt = static_cast<size_t>(eDefClusterType::Cnt);
    for(size_t nLoop=0; nLoop < clusterCnt; ++nLoop) {

        if(hangupValidTime_ != 0)
            lastActionTime_ = time(nullptr);
        
        Connection * c = _cc->Get(static_cast<eDefClusterType>(nLoop));

        if(c == nullptr)
            continue;

        if(c->IsConnected() == false) {

            if(_cc->IsActiveDB(c))
                _cc->SetStandbyDB(c);

            if(c->Connect(GetUuidForConnection())) {
                alarm_.Push(nodeId_, PDB::CONNECT, c);
                reporter_.Update(dbType, shardingNumber, c, _cc->IsActiveDB(c));
            }
            continue;
        }

        /*-
        // Active 는 체크 안할꺼예요.. 바쁜 놈이예요..
        if(_cc->IsActiveDB(c)) {
            reporter_.Update(dbType, shardingNumber, c, _cc->IsActiveDB(c));
            continue;
        }
        -*/

        // 체크한지 쫌 된 놈들은 다시 체크해야 해요..
        if(Timer::IsTimeout(c->GetLastPingCheckTime(), pingCheckPeriod_)) {
            c->SetPingCheckTime();

            if(c->IsBusy() == false)
                pingCheck(c);
        }

        if(c->IsConnected()) {
            reporter_.Update(dbType, shardingNumber, c, _cc->IsActiveDB(c));
        }
    }

}

void ConnectionMonitorTask::correctActiveDB(eDefDBType _pdbT, size_t _shardingNumber, ConnectionCluster * _cc) {

    std::tuple<eDefDBType, size_t> key = std::make_tuple(_pdbT, _shardingNumber);
    
    auto iter = lastActiveDB_.find(key);

    if(iter != lastActiveDB_.end()) {

        Connection * c = _cc->Get(iter->second);

        if(c == nullptr)
            return ;

        if(c->IsConnected() && _cc->IsActiveDB(c) == false) {
            _cc->SetActiveDB(iter->second);
        }
    }
}

void ConnectionMonitorTask::pingCheck(Connection * _c) {
    _c->PingCheck();

    if(_c->IsConnected())
        return;

    alarm_.Push(nodeId_, PDB::DISCONNECT, _c);
    if(_c->Connect(GetUuidForConnection()))
        alarm_.Push(nodeId_, PDB::CONNECT, _c);
}

void ConnectionMonitorTask::saveActiveDB(eDefDBType _pdbT, eDefClusterType _clusterT, size_t _shardingNum) {

    std::tuple<eDefDBType, size_t> key = std::make_tuple(_pdbT, _shardingNum);
    
    auto iter = lastActiveDB_.find(key);
    if(iter == lastActiveDB_.end()) {
        lastActiveDB_.emplace(key, _clusterT);
    } else {
        iter->second = _clusterT;
    }
}


int ConnectionMonitorTask::SwitchActiveDB(eDefDBType _pdbT, eDefClusterType _clusterT, size_t _shardingNum) {

    // 혹시 Monitoring 을 요청한 List 가 있었을지도 몰라요.
    moveFromWaitingListToMonitoringList();

    std::lock_guard<std::mutex>     lock(mutex_);

    bool    bret = false; 
    int     cnt  = 0;

    for(const auto & cIter : vecMonitoringList_) {

        ConnectionCluster * cc = cIter.get();

        if(cc->GetDBType() == static_cast<size_t>(_pdbT) &&
           cc->GetShardingNumber() == _shardingNum) {
   
            if((bret = cc->SetActiveDB(_clusterT)) = true)
                cnt++;

            I_THD_LOG(gLogName, "Switch.ActiveDB [%s] [%s:%zu] [%s]",
                ToString::Get(_pdbT),
                (bret)?"ok":"nok",
                _shardingNum,
                ToString::Get(_clusterT));

            saveActiveDB(_pdbT, _clusterT, _shardingNum);
        }
    }

    lastSwitchActiveDBT_ = time(nullptr);

    return cnt;
}

void ConnectionMonitorTask::moveFromWaitingListToMonitoringList() {

    if(vecWaitingList_.size() > 0) {
        std::lock_guard<std::mutex>     guard(mutex_);

        for(auto & citer : vecWaitingList_)
            vecMonitoringList_.push_back(citer);

        vecWaitingList_.clear();

        D_THD_LOG(gLogName,
            "ConnectionMonitorTask.MonitoringList.Cnt [%zu]",
            vecMonitoringList_.size());

    }
}


} // PDB
