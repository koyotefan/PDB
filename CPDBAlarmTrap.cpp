
#include "CPDBAlarmTrap.hpp"

size_t CPDBAlarmTrap::Send(PDB::ConnectionManager * _cmgr) {

    std::string     alarmString;

    size_t          cnt = 0;

    while(true) {
        if(_cmgr->PopAlarm(alarmString) == false)
            return cnt;

        if(trap(alarmString))
            ++cnt;
    }

    return cnt;
}

bool CPDBAlarmTrap::trap(std::string & _str) {

    std::string     action;
    std::string     metric;
    std::string     location;
    std::string     value;
    std::map<std::string, std::string>  mD;
    std::map<std::string, std::string>  mV;

    rapidjson::Document     doc;

    try {
        if(doc.Parse(_str.c_str()).HasParseError()) {
            W_THD_LOG(gLogName,
                "CPDBAlarmTrap document fail [%s]",
                _str.c_str());
            return false;
        }
        parse(action, metric, location, value, mD, mV, doc);
    } catch( std::exception & _e) {
        W_THD_LOG(gLogName,
            "CPDBAlarmTrap.parse fail [%s:%s]",
            _str.c_str(), _e.what());
        return false;
    }

    ATOM_TRAP_EXT(const_cast<char *>(action.c_str()),
                  const_cast<char *>(metric.c_str()),
                  const_cast<char *>(location.c_str()),
                  const_cast<char *>(value.c_str()),
                  mD,
                  mV);

    return true;
}

void CPDBAlarmTrap::parse(std::string & _action,
    std::string & _metric,
    std::string & _location,
    std::string & _value,
    std::map<std::string, std::string> & _mD,
    std::map<std::string, std::string> & _mV,
    rapidjson::Document & _doc) throw(std::exception) {

    setValue(_action, "action", _doc);
    setValue(_metric, "metric", _doc);
    setValue(_location, "location", _doc);
    setValue(_value, "value", _doc);

    std::string     dbname;
    std::string     ip;
    std::string     port;

    setValue(dbname, "dbname", _doc);
    setValue(ip, "ip", _doc);
    setValue(port, "port", _doc);

    _mV.emplace("dbname", dbname);
    _mV.emplace("ip"    , ip);
    _mV.emplace("port"  , port);
}

void CPDBAlarmTrap::setValue(std::string & _out,
                             const char * _name,
                             rapidjson::Document & _doc) throw(std::exception) {

    if(_doc.HasMember(_name) == false) {
        W_THD_LOG(gLogName, "CPDBAlarmTrap.setValue fail [%s]", _name);
        return ;
    }

    rapidjson::Value & _value = _doc[_name];

    if(_value.IsString() == false) {
        W_THD_LOG(gLogName, "CPDBAlarmTrap.setValue fail [%s]", _name);
        return ;
    }

    _out = _value.GetString();
    return ;
}