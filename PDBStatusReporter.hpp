#ifndef PDB_STATUS_REPORTER_HPP
#define PDB_STATUS_REPORTER_HPP

#include <unordered_map>

#include "PDBDefine.hpp"
#include "PDBConnectionCluster.hpp"
#include "EGWUdpClient.hpp"

namespace PDB {

class StatusReporter {
public:
    explicit StatusReporter();
    ~StatusReporter();

    bool Init(const std::string & _nodeId,
              const std::string & _procName,
              const stConnectionInfo & _stP,
              const stConnectionInfo & _stS);
    void Reset();
    bool Update(const size_t _dbType,
                const size_t _shardingNumber,
                const Connection * _c,
                const bool _bActiveDB);
    bool Report();

private:
    const char * getPdbTypeName(size_t _pdbType);
    size_t diff(std::unordered_map<std::string, stPdbStatusItem> & _m);
    void termReport();

private:

    std::string         nodeId_;
    std::string         procName_;
    stConnectionInfo    priConnectionInfo_;
    stConnectionInfo    sndConnectionInfo_;

    std::unordered_map<std::string, stPdbStatusItem>    mapBeforeItems_;
    std::unordered_map<std::string, stPdbStatusItem>    mapAfterItems_;
    // std::unordered_map<std::string, stPdbStatusItem>    mapReport_;

    EGWUdpClient        client_;
};



}


#endif // PDB_STATUS_REPORTER_HPP
