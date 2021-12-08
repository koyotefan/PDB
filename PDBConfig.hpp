#ifndef PDB_CONFIG_HPP
#define PDB_CONFIG_HPP

#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <sql.h>

#include "rapidjson/document.h"
#include "PDBDefine.hpp"
#include "PDBException.hpp"

namespace PDB {

class Config {
public:
    explicit Config();
    ~Config();

    bool Init(std::string _confName);
    stConnectionInfo & GetConnectionInfo(size_t _clusterNum,
                                         eDefClusterType _clusterT);
    bool IsFailOverReason(std::string & _state);

    size_t GetSessionDetailShardingCnt() { return nSessionDetailShardingCnt_; }
    int GetPingCheckPeriod() { return nPingCheckPeriod_; }
    int GetMonitorPeriod()   { return nConnectionMonitoringPeriod_; }

    static bool Parsing_PCF_PDB_ACTIVE(std::unordered_map<std::string, std::string> & _map,
                                       std::string & _json);
    static void loadForPDBActive(std::unordered_map<std::string, std::string> & _map,
                                 const rapidjson::Value & _value,
                                 const char * _name) throw(std::exception);

    static eDefDBType ExtractDBType(const std::string & _dbName);
    static size_t ExtractShardingNumber(const std::string & _dbName);
    static eDefClusterType ExtractClusterType(const std::string & _dbName);

    static std::string GetClusterName(const std::string & _dbName);
    size_t GetDBNames(std::vector<std::string> & _vec);

private:
    bool loadConfigFile();

    void fileToJson(rapidjson::Document & _doc,
                    std::string & _fname) throw(ConfigException, std::exception);
    void setConnectionInfoFromJson(rapidjson::Document & _doc) throw(std::exception);
    void setConnectionInfo(stConnectionInfo & _st,
                           const char * _instanceName,
                           rapidjson::Document & _doc) throw(std::exception);
    void parseConnectionInfo(eDefDBType     _dbT,
                            const char *    _dbTName,
                            int             _clusterNum,
                            rapidjson::Document & _doc,
                            stConnectionInfo & _stDef) throw(std::exception);
    void setFailOverInfoFromJson(rapidjson::Document & _doc) throw(std::exception);
    void setPingCheckPeriodFromJson(rapidjson::Document & _doc) throw(std::exception);
    void setConnectionMonitoringPeriodFromJson(rapidjson::Document & _doc) throw(std::exception);




private:
    std::string     confName_;

    std::array<std::array<stConnectionInfo, static_cast<size_t>(eDefClusterType::Cnt)>,
               static_cast<size_t>(eDefValue::NonShardingCnt) +
               static_cast<size_t>(eDefValue::MaxShardingCnt)
              >  arrConnectionInfo_;

    std::vector<std::string>   vecFailOverReason_;

    size_t nSessionDetailShardingCnt_;
    int   nPingCheckPeriod_;
    int   nConnectionMonitoringPeriod_;
};



}

#endif // PDB_CONFIG_HPP
