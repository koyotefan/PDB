#ifndef PDB_CONNECTION_CLUSTER_HPP
#define PDB_CONNECTION_CLUSTER_HPP

#include <array>
#include <memory>

#include "PDBDefine.hpp"
#include "PDBConnection.hpp"

namespace PDB {

class ConnectionCluster {
public:
    explicit ConnectionCluster(eDefDBType _pdbT, size_t _shardingNum);
    ConnectionCluster(const ConnectionCluster & _rhs) = delete;
    ~ConnectionCluster();
    Connection * Get(eDefClusterType _clusterT);
    Connection * Get();
    std::shared_ptr<Connection>     GetSharedPtr(eDefClusterType _clusterT);

    size_t      GetDBType() { return static_cast<size_t>(myT_); }
    size_t      GetShardingNumber() { return myShardingNum_; }
    eDefClusterType GetActiveClusterT() const { return activeClusterT_; }
    eDefClusterType GetStandbyClusterT();

    //void        WhenIsNotActiveThenChangeTo(Connection * _c);

    bool    IsActiveDB(Connection * _c);
    bool    SetActiveDB(eDefClusterType _clusterT);
    bool    SetStandbyDB(Connection * _c);


private:

    eDefDBType      myT_;
    size_t          myShardingNum_;

    std::array<std::shared_ptr<Connection>,
                static_cast<size_t>(eDefClusterType::Cnt)>   arr_;

    eDefClusterType     activeClusterT_;
};

class ConnectionClusterArray {
public:

    explicit ConnectionClusterArray() {};
    ~ConnectionClusterArray() {};

    bool    IsUsed(int _clusterNum) { return IsUsed(static_cast<size_t>(_clusterNum)); } 
    bool    IsUsed(size_t _clusterNum) { return arr_[_clusterNum].use_count() > 0; }
    bool    Assign(const eDefDBType _pdbT, const size_t _shardingNum) { 
        size_t clusterNum = GetIndex(_pdbT, _shardingNum);
        return Assign(clusterNum, _pdbT, _shardingNum);
    }
    bool    Assign(const size_t _clusterNum, const eDefDBType _pdbT, size_t _shardingNum) {
        if(_clusterNum > arr_.size())
            return false;

        arr_[_clusterNum] = 
            std::make_shared<ConnectionCluster>(_pdbT, _shardingNum);

        return true;
    }
    void Assign(const size_t _clusterNum, std::shared_ptr<ConnectionCluster>     sp) {
        arr_[_clusterNum] = sp;
    }

    void    Release(size_t _clusterNum) {
        if(_clusterNum > arr_.size())
            return ;

        if(arr_[_clusterNum])
            arr_[_clusterNum] = {};
    }

    ConnectionCluster * Get(const eDefDBType _pdbT, const size_t _shardingNum) {
        size_t clusterNum = GetIndex(_pdbT, _shardingNum);
        return Get(clusterNum);
    }

    ConnectionCluster * Get(const size_t _clusterNum) {
        if(_clusterNum > arr_.size())
            return nullptr;

        return arr_[_clusterNum].get();
    } 

    std::shared_ptr<ConnectionCluster>  GetSharedPtr(const size_t _clusterNum) {
        return arr_[_clusterNum];
    }

    size_t GetIndex(const eDefDBType _pdbT, const size_t _shardingNum) {
        return static_cast<size_t>(_pdbT) + _shardingNum;
    } 

    size_t GetSharedPtrUsedCount(const size_t _clusterNum) {
        return arr_[_clusterNum].use_count();
    }

private:
       std::array<std::shared_ptr<ConnectionCluster>,
               static_cast<int>(eDefValue::NonShardingCnt) +
               static_cast<int>(eDefValue::MaxShardingCnt)>   arr_;

};

}

#endif // PDB_CONNECTION_CLUSTER_HPP
