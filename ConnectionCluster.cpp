
#include "PDBConnectionCluster.hpp"

namespace PDB {

ConnectionCluster::ConnectionCluster(eDefDBType _pdbT, size_t _shardingNum)
    : myT_(_pdbT),
      myShardingNum_(_shardingNum),
      activeClusterT_(eDefClusterType::P) {

    size_t clusterCnt = static_cast<size_t>(eDefClusterType::Cnt);

    for(size_t clusterT=0; clusterT < clusterCnt; ++clusterT) {
        arr_[clusterT] = std::make_shared<Connection>();
    }
}

ConnectionCluster::~ConnectionCluster() {

}

Connection * ConnectionCluster::Get(eDefClusterType _clusterT) {
    return arr_[static_cast<size_t>(_clusterT)].get();
}

Connection * ConnectionCluster::Get() {

    Connection * c = arr_[static_cast<size_t>(activeClusterT_)].get();

    if(c != nullptr && c->IsConnected())
        return c;

    for(const auto & citer : arr_) {
        c = citer.get();

        if(c != nullptr && c->IsConnected())
            return c;
    }

    // 연결된 Connection 이 없어요.
    return nullptr;
}

std::shared_ptr<Connection> ConnectionCluster::GetSharedPtr(eDefClusterType _clusterT) {
    return arr_[static_cast<size_t>(_clusterT)];
}

eDefClusterType ConnectionCluster::GetStandbyClusterT() {
    return (activeClusterT_ == eDefClusterType::P)?eDefClusterType::S:eDefClusterType::P;
}

/*-
void ConnectionCluster::WhenIsNotActiveThenChangeTo(Connection * _c) {

    if(IsActiveDB(_c))
        return ;

    // 우리 Active 가 변경되었어요..
    for(size_t nLoop=0; nLoop < arr_.size(); ++nLoop) {
        if(_c == arr_[nLoop].get()) {
            Connection * _oldC = arr_[static_cast<size_t>(activeClusterT_)].get();
            activeClusterT_ = static_cast<eDefClusterType>(nLoop);

            W_THD_LOG(
                gLogName,
                "ConnectionCluster.ChangeTo [%s:%d] -> [%s:%d]",
                _oldC->GetIp(),
                _oldC->GetPort(),
                _c->GetIp(),
                _c->GetPort());

            return ;
        }
    }
}
-*/

bool ConnectionCluster::IsActiveDB(Connection * _c) {

    Connection * nowActiveC = arr_[static_cast<size_t>(activeClusterT_)].get();
    return (_c == nowActiveC)?true:false;
}

bool ConnectionCluster::SetActiveDB(eDefClusterType _clusterT) {
    
    //if(activeClusterT_ == _clusterT)
    //    return false;

    Connection * c = arr_[static_cast<size_t>(_clusterT)].get();

    if(c->IsConnected() == false)
        return false;

    I_THD_LOG(gLogName, "ConnectionCluster.SetActiveDB DBT [%s] ShardingNum [%zu] [%s]->[%s]",
        ToString::Get(myT_),
        myShardingNum_,
        ToString::Get(activeClusterT_),
        ToString::Get(_clusterT));

    activeClusterT_ = _clusterT;
    return true;
}

bool ConnectionCluster::SetStandbyDB(Connection * _c) {

    for(size_t nLoop=0; nLoop < arr_.size(); ++nLoop) {
        Connection * newC = arr_[nLoop].get();

        if(_c == newC)
            continue;

        if(newC->IsConnected() == false)
            return false;

        activeClusterT_ = static_cast<eDefClusterType>(nLoop);

        I_THD_LOG(
            gLogName,
            "ConnectionCluster.ChangeTo [%s:%d] -> [%s:%d]",
            _c->GetIp(),
            _c->GetPort(),
            newC->GetIp(),
            newC->GetPort());

            return true;
    }

    W_THD_LOG(
        gLogName,
        "ConnectionCluster.ChangeTo.fail");

    return false;
}

}
