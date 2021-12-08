
#include <iostream>
#include <sstream>
#include <fstream>

#include "PDBConfig.hpp"

namespace PDB {

Config::Config()
    : nSessionDetailShardingCnt_(0),
      nPingCheckPeriod_(30),
      nConnectionMonitoringPeriod_(5) {

}

Config::~Config() {

}

bool Config::Init(std::string _confName) {
    confName_ = _confName;

    if(loadConfigFile() == false) {
        W_THD_LOG(gLogName,
            "Config.Init.loadConfigFile [%s]", confName_.c_str());
        return false;
    }

    return true;
}

// Static 함수, ConnectionManager 에서 호출 합니다.
// Netconf 에 있는 PDB Active 정보를 Parsing 합니다.
bool Config::Parsing_PCF_PDB_ACTIVE(std::unordered_map<std::string, std::string> & _map,
                                    std::string & _json) {

    rapidjson::Document     doc;

    try {

        if(doc.Parse(_json.c_str()).HasParseError()) {
            E_LOG("json Parse() Error [%s]", _json.c_str());
            return false;
        }

        loadForPDBActive(_map, doc, "pdb-type-list");
    } catch ( std::exception & _e ) {
        W_THD_LOG(gLogName,
            "Config.Parsing_PCF_PDB_ACTIVE.SetActivePDBInstanceName fail [%s:%s]",
            _json.c_str(), _e.what());
        return false;
    }

    return true;
}

void Config::loadForPDBActive(std::unordered_map<std::string, std::string> & _map,
                              const rapidjson::Value & _value,
                              const char * _name) throw(std::exception) {

    for(auto iter=_value.MemberBegin(); iter != _value.MemberEnd(); ++iter) {
        if(strcmp(iter->name.GetString(), _name) != 0) {
            if(iter->value.IsObject()) {
                loadForPDBActive(_map, iter->value, _name);
            }
            continue;
        }

        std::string name = iter->value["name"].GetString();
        std::string active = iter->value["active"].GetString();

        if(name.size() == 0 || active.size() == 0)
            continue;

        _map.emplace(name, active);
    }
}

eDefDBType Config::ExtractDBType(const std::string & _dbName) {
    // ex) SessionDetail_1_P -> SessionDetail

    std::size_t pos = _dbName.find("_");

    if(pos == std::string::npos) {
        return ToEnumClass::GetDBType(_dbName);
    }

    return ToEnumClass::GetDBType(_dbName.substr(0, pos));
}

size_t Config::ExtractShardingNumber(const std::string & _dbName) {
    // ex) SessionDetail_1_P -> 1

    std::size_t pos = _dbName.find("_");

    if(pos == std::string::npos) {
        return 0;
    }

    try {
        return std::stoull(_dbName.substr(pos+1));
    } catch (std::invalid_argument & _e) {
        W_THD_LOG(gLogName, "invalid DBName [%s]", _dbName.c_str());
        return 0;
    }
}

eDefClusterType Config::ExtractClusterType(const std::string & _dbName) {
    // ex) SessionDetail_1_P -> P
    std::size_t pos = _dbName.rfind("_");

    if(pos == std::string::npos) {
        return ToEnumClass::GetClusterType("P");
    }

    // 1 자리만 return 합니다.
    return ToEnumClass::GetClusterType(_dbName.substr(pos+1, 1));
}

std::string Config::GetClusterName(const std::string & _dbName) {
    // ex) SessionDetail_1_P ->  SessionDetail_1
    //     PCF_P -> PCF
    std::size_t pos = _dbName.rfind("_");

    if(pos == std::string::npos) {
        return _dbName;
    }

    return _dbName.substr(0, pos);
}

size_t Config::GetDBNames(std::vector<std::string> & _vec) {
    for(auto & arr : arrConnectionInfo_) {
        for(auto & info : arr) {
            if(info.DBName.length() == 0)
                continue;
            _vec.push_back(info.DBName);
        }
    }

    return _vec.size();
}


bool Config::loadConfigFile() {

    rapidjson::Document     doc;

    try {
        fileToJson(doc, confName_);
    } catch ( ConfigException & _ce ) {
        W_THD_LOG(gLogName,
            "Config.loadConfigFile.fileToJson fail [%s:%s]",
            confName_.c_str(), _ce.what());
        return false;
    } catch ( std::exception & _e ) {
        W_THD_LOG(gLogName,
            "Config.loadConfigFile.fileToJson fail [%s:%s]",
            confName_.c_str(), _e.what());
        return false;
    }

    try {
        setConnectionInfoFromJson(doc);
    } catch ( std::exception & _e ) {
        W_THD_LOG(gLogName,
            "Config.loadConfigFile.setConnectionInfoFromJson fail [%s:%s]",
            confName_.c_str(), _e.what());
        return false;
    }

    try {
        setFailOverInfoFromJson(doc);
    } catch ( std::exception & _e ) {
        W_THD_LOG(gLogName,
            "Config.loadConfigFile.setFailOverInfoFromJson fail [%s:%s]",
            confName_.c_str(), _e.what());
        return false;
    }

    try {
        setPingCheckPeriodFromJson(doc);
    } catch ( std::exception & _e ) {
        W_THD_LOG(gLogName,
            "Config.loadConfigFile.setPingCheckPeriodFromJson fail [%s:%s]",
            confName_.c_str(), _e.what());
        return false;
    }

    try {
        setConnectionMonitoringPeriodFromJson(doc);
    } catch ( std::exception & _e ) {
        W_THD_LOG(gLogName,
            "Config.loadConfigFile.setConnectionMonitoringPeriodFromJson  fail [%s:%s]",
            confName_.c_str(), _e.what());
        return false;
    }

    return true;
}


void Config::fileToJson(rapidjson::Document & _doc, std::string & _fname)
    throw (ConfigException, std::exception) {
    std::ifstream f(_fname);

    if(f.is_open() == false)
        throw ConfigException("file open fail");

    std::stringstream   buffer;
    buffer << f.rdbuf();
    f.close();

    std::string str = buffer.str();
    if(_doc.Parse(str.c_str()).HasParseError())
        throw ConfigException("invalid json file");
}


void Config::setConnectionInfoFromJson(rapidjson::Document & _doc)
    throw (std::exception) {

    // Default Read
    stConnectionInfo    stDefault;

    setConnectionInfo(stDefault, "Default", _doc);

    parseConnectionInfo(eDefDBType::PCF,            "PCF",          0, _doc, stDefault);
    parseConnectionInfo(eDefDBType::SessionMaster,  "SessionMaster",0, _doc, stDefault);
    parseConnectionInfo(eDefDBType::Subscriber,     "Subscriber",   0, _doc, stDefault);
    parseConnectionInfo(eDefDBType::PG,             "PG",           0, _doc, stDefault);
    parseConnectionInfo(eDefDBType::PGLocal1,       "PGLocal1",     0, _doc, stDefault);
    parseConnectionInfo(eDefDBType::PGLocal2,       "PGLocal2",     0, _doc, stDefault);

    nSessionDetailShardingCnt_ = 0;
    char    tagName[32];
    while(true) {
        snprintf(tagName, sizeof(tagName), "SessionDetail_%zu", nSessionDetailShardingCnt_);
        if(_doc.HasMember(tagName) == false)
            break;

        parseConnectionInfo(eDefDBType::SessionDetail, tagName, nSessionDetailShardingCnt_, _doc, stDefault);
        nSessionDetailShardingCnt_++;
    }
}

void Config::setConnectionInfo(stConnectionInfo & _st, const char * _instanceName, rapidjson::Document & _doc)
    throw (std::exception) {

    if(_doc.HasMember(_instanceName) == false) {
        W_THD_LOG(gLogName, "Config.setConnectionInfo [%s] is nothing", _instanceName);
        return ;
    }

    _st.DBName = _instanceName;

    rapidjson::Value & _value = _doc[_instanceName];

    if(_value.IsObject() == false) {
        W_THD_LOG(gLogName, "Config.setConnectionInfo [%s] is not Object", _instanceName);
        return ;
    }

    for(auto iter = _value.MemberBegin(); iter != _value.MemberEnd(); ++iter) {

        if(iter->value.IsString() && strcmp(iter->name.GetString(), "Dsn") == 0) {
            _st.DSN = iter->value.GetString();
            continue;
        }

        if(iter->value.IsString() && strcmp(iter->name.GetString(), "Host") == 0) {
            _st.Host = iter->value.GetString();
            continue;
        }

        if(iter->value.IsString() && strcmp(iter->name.GetString(), "Id") == 0) {
            _st.Id = iter->value.GetString();
            continue;
        }

        if(iter->value.IsString() && strcmp(iter->name.GetString(), "Pwd") == 0) {
            _st.Pwd = iter->value.GetString();
            continue;
        }

        if(iter->value.IsInt() && strcmp(iter->name.GetString(), "nConnType") == 0) {
            _st.nConnType = iter->value.GetInt();
            continue;
        }

        if(iter->value.IsBool() && strcmp(iter->name.GetString(), "bNcharCheck") == 0) {
            _st.bNcharCheck = iter->value.GetBool();
            // 찍어봐야 겠어요.
            continue;
        }

        if(iter->value.IsInt() && strcmp(iter->name.GetString(), "nPort") == 0) {
            _st.nPort = iter->value.GetInt();
            continue;
        }

        if(iter->value.IsInt() && strcmp(iter->name.GetString(), "nTimeout") == 0) {
            _st.nTimeout = iter->value.GetInt();
            continue;
        }

        if(iter->value.IsInt() && strcmp(iter->name.GetString(), "nConnectionTimeout") == 0) {
            _st.nConnectionTimeout = iter->value.GetInt();
            continue;
        }
    }

    D_THD_LOG(gLogName,
        "Config.setConnectionInfo [%s] [%s:%s:%s:%d:%s:%d:%d:%d]",
        _st.DBName.c_str(),
        _st.DSN.c_str(),
        _st.Host.c_str(),
        _st.Id.c_str(),
        _st.nConnType,
        (_st.bNcharCheck)?"true":"false",
        _st.nPort,
        _st.nTimeout,
        _st.nConnectionTimeout);
}

void Config::parseConnectionInfo(eDefDBType     _dbT,
                                const char *    _dbTName,
                                int             _clusterNum,
                                rapidjson::Document & _doc,
                                stConnectionInfo & _stDef)
    throw(std::exception) {

    if(_doc.HasMember(_dbTName) == false) {
        W_THD_LOG(gLogName, "Config.parseConnectionInfo fail [%s] is nothing", _dbTName);
        return ;
    }

    rapidjson::Value & _value = _doc[_dbTName];

    if(_value.IsArray() == false) {
        W_THD_LOG(gLogName, "Config.parseConnectionInfo fail [%s] is not Object", _dbTName);
        return ;
    }

    size_t clusterIndex = static_cast<size_t>(_dbT) + _clusterNum;
    for(size_t i=0; i < _value.Size(); ++i) {
        stConnectionInfo    st = _stDef;
        setConnectionInfo(st, _value[i].GetString(), _doc);
        arrConnectionInfo_[clusterIndex][i] = st;
    }
}


void Config::setFailOverInfoFromJson(rapidjson::Document & _doc)
    throw (std::exception) {

    if(_doc.HasMember("FailOvers") == false) {
        W_THD_LOG(gLogName, "Config.setFailOverInfoFromJson fail [FailOvers] is nothing");
        return ;
    }

    rapidjson::Value & _value = _doc["FailOvers"];

    if(_value.IsArray() == false) {
        W_THD_LOG(gLogName, "Config.setFailOverInfoFromJson fail [FailOvers] is not Array");
        return ;
    }

    for(size_t i=0; i < _value.Size(); ++i)
        vecFailOverReason_.push_back(_value[i].GetString());

    for(const auto & citer : vecFailOverReason_)
        I_THD_LOG(gLogName, "Config.setFailOverInfoFromJson [%s]", citer.c_str());

}

void Config::setPingCheckPeriodFromJson(rapidjson::Document & _doc)
    throw (std::exception) {

    if(_doc.HasMember("nPingCheckPeriod") == false) {
        W_THD_LOG(gLogName,
            "Config.setPingCheckPeriodFromJson is nothing [%d]",
            nPingCheckPeriod_);
        return ;
    }

    rapidjson::Value & _value = _doc["nPingCheckPeriod"];

    if(_value.IsInt() == false) {
        W_THD_LOG(gLogName,
            "Config.setPingCheckPeriodFromJson is not String [%d]",
            nPingCheckPeriod_);
        return ;
    }

    nPingCheckPeriod_ = (_value.GetInt() > 0)?_value.GetInt():nPingCheckPeriod_;

    I_THD_LOG(gLogName, "Config.setPingCheckPeriodFromJson [%d]", nPingCheckPeriod_);
}


void Config::setConnectionMonitoringPeriodFromJson(rapidjson::Document & _doc)
    throw (std::exception) {

    if(_doc.HasMember("nConnectionMonitoringPeriod") == false) {
        W_THD_LOG(gLogName,
            "Config.setConnectionMonitoringPeriodFromJson is nothing [%d]",
            nConnectionMonitoringPeriod_);
        return ;
    }

    rapidjson::Value & _value = _doc["nConnectionMonitoringPeriod"];

    if(_value.IsInt() == false) {
        W_THD_LOG(gLogName,
            "Config.setConnectionMonitoringPeriodFromJson is not String [%d]",
            nConnectionMonitoringPeriod_);
        return ;
    }

    nConnectionMonitoringPeriod_ = (_value.GetInt() > 0)?_value.GetInt():nConnectionMonitoringPeriod_;

    I_THD_LOG(gLogName, "Config.setConnectionMonitoringPeriodFromJson  [%d]", nConnectionMonitoringPeriod_);
}


stConnectionInfo & Config::GetConnectionInfo(size_t _clusterNum, eDefClusterType _clusterT) {

    return arrConnectionInfo_[_clusterNum][static_cast<size_t>(_clusterT)];
}

bool Config::IsFailOverReason(std::string & _state) {

    for(const auto & citer : vecFailOverReason_) {
        if(citer.compare(_state) == 0)
            return true;
    }

    return false;
}

}
