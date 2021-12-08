
#include <vector>
#include <numeric>

#include "PDBStatusReporter.hpp"

namespace PDB {

StatusReporter::StatusReporter() {

}

StatusReporter::~StatusReporter() {
    termReport();
}




bool StatusReporter::Init(const std::string & _nodeId,
                          const std::string & _procName,
                          const stConnectionInfo & _stP,
                          const stConnectionInfo & _stS) {

    nodeId_ = _nodeId;
    procName_ = _procName;

    priConnectionInfo_ = _stP;
    sndConnectionInfo_ = _stS;

    if(client_.Init("127.0.0.1", 9030) == false) {
        D_THD_LOG(gLogName, "StatusReporter.UdpClient.fail");
        return false;
    }

    return true;
}

void StatusReporter::Reset() {

    mapBeforeItems_ = mapAfterItems_;

    for(auto iter=mapAfterItems_.begin();
        iter != mapAfterItems_.end();
        ++iter) {

        iter->second.actCnt = 0;
        iter->second.connCnt = 0;
    }
}

bool StatusReporter::Update(const size_t _dbType,
                            const size_t _shardingNumber,
                            const Connection * _c,
                            const bool _bActiveDB) {


    auto iter = mapAfterItems_.find(_c->GetDBName());

    if(iter == mapAfterItems_.end()) {
        mapAfterItems_.emplace(_c->GetDBName(), stPdbStatusItem());
        iter = mapAfterItems_.find(_c->GetDBName());

        stPdbStatusItem & item = iter->second;

        strncpy(item.nodeId, nodeId_.c_str(), COL_LEN_NODE_ID);
        strncpy(item.procName, procName_.c_str(), COL_LEN_PROC_ID);
        strncpy(item.pdbType, ToString::Get(static_cast<eDefDBType>(_dbType)), COL_LEN_PDB_TYPE);
        item.shardingId = _shardingNumber;
        strncpy(item.dbName, _c->GetDBName(), COL_LEN_DB_NAME);
        strncpy(item.dbIp, _c->GetIp(), COL_LEN_DB_IP);
        item.dbPort = _c->GetPort();
    }

    ++(iter->second.connCnt);
    if(_bActiveDB)
        ++(iter->second.actCnt);

    return true;
}

void StatusReporter::termReport() {

    char buf[256];
    snprintf(buf, sizeof(buf),
        "{ \"nodeId\": \"%s\","
        "\t\"procName\": \"%s\", "
        "\t\"reports\": [] }",
        nodeId_.c_str(),
        procName_.c_str());

    if(client_.Send(buf, strlen(buf)) == false) {
        W_THD_LOG(gLogName, "udpclient.Send.fail [%s]", buf);
    }
}

bool StatusReporter::Report() {

/*-
    항상 쏴야 할 것 같아요.. UDP 여서...
    mapReport_.clear();

    if(diff(mapReport_) == 0)
        return true;
-*/

    std::vector<std::string>    vec;

    char buf[256];
    snprintf(buf, sizeof(buf),
        "{\"nodeId\":\"%s\","
        "\t\"procName\":\"%s\",",
        nodeId_.c_str(),
        procName_.c_str());

    vec.push_back(buf);
    vec.push_back("\t\"reports\":[");

    for(auto & citer : mapAfterItems_) {
        stPdbStatusItem & item = citer.second;

        snprintf(buf, sizeof(buf),
            "\t{\"pdbType\":\"%s\","
            "\t\"shardingId\":%d,"
            "\t\"dbName\":\"%s\","
            "\t\"dbIp\":\"%s\","
            "\t\"dbPort\":%d,"
            "\t\"connCnt\":%d,"
            "\t\"actCnt\":%d}",
            item.pdbType,
            item.shardingId,
            item.dbName,
            item.dbIp,
            item.dbPort,
            item.connCnt,
            item.actCnt);
        vec.push_back(buf);
        vec.push_back(",");

        /*-
        D_THD_LOG(gLogName,
            "-- [%s:%s] [%s:%d] [%s:%s:%d] [%d] [%d]",
            item.nodeId,
            item.procName,
            item.pdbType,
            item.shardingId,
            item.dbName,
            item.dbIp,
            item.dbPort,
            item.connCnt,
            item.actCnt);
        -*/
    }

    if(mapAfterItems_.empty() == false)
        vec.pop_back();

    vec.push_back("]}");

    std::string jData = std::accumulate(vec.begin(), vec.end(), std::string(""));
    // D_THD_LOG(gLogName, "%s", jData.c_str());

    if(client_.Send(jData.c_str(), jData.length()) == false) {
        W_THD_LOG(gLogName, "udpclient.Send.fail [%s]", jData.c_str());
        return false;
    }

    return true;
}

size_t StatusReporter::diff(std::unordered_map<std::string, stPdbStatusItem> & _m) {

    for(auto & a : mapAfterItems_) {
        auto b = mapBeforeItems_.find(a.first);

        if(b == mapBeforeItems_.end()) {
            _m.emplace(a);
            continue;
        }

        if(a.second.Compare(b->second))
            continue;

        _m.emplace(a);
    }

    return _m.size();
}

}
