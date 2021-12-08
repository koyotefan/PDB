
#include <numeric>
#include <sstream>
#include <iomanip>

#include "NDFServiceLog.hpp"
#include "CNDFATOMAdaptor.hpp"

#include "CNetconfPDBActive.hpp"
#include "rapidjson/document.h"

bool CNetconfPDBActive::LoadConfig() {

    std::string     data;
    if(Load(data) == false) {
        E_LOG("LoadConfig() fail [%s:%s]", modelName_.c_str(), path_.c_str());
        return false;
    }

    jData_ = data;

    return true;
}

bool CNetconfPDBActive::SaveConfig(std::unordered_map<std::string, std::string> & _m) {

    saveConfig(_m, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    saveConfig(_m, 1);

    return true;
}

bool CNetconfPDBActive::saveConfig(std::unordered_map<std::string, std::string> & _m, int _cnt) {

    std::string jData;

    if(makeJsonData(jData, _m, _cnt) == false) {
        return false;
    }

    std::lock_guard<std::mutex>     guard(mutex_);

    try {
        api_->EditConfigJsonReq("PCF_PDB_ACTIVE", jData, CNetconfAPI::SRC_RUNNING);
        api_->EditConfigJsonReq("PCF_PDB_ACTIVE", jData, CNetconfAPI::SRC_STARTUP);
    } catch (atom::CException & _e) {
        W_LOG("SaveConfig.Exception [%s] [%s]", _e.what(), jData.c_str());
        return false;
    }

    return true;
}

bool CNetconfPDBActive::makeJsonData(std::string & _jData,
                                     std::unordered_map<std::string, std::string> & _m,
                                     int _cnt) {

    if(_m.empty() == true)
        return false;

    std::string time = getTimeString();

    std::vector<std::string>    vec;

    vec.push_back("{ \"PdbActive\": { ");

    char buf[256];

    for(auto & iter : _m) {
        snprintf(buf, sizeof(buf),
                "\t\"pdb-type-list\": "
                "{ \"name\": \"%s\", "
                " \"active\": \"%s\", "
                " \"update-time\": \"%s%d\" }",
                iter.first.c_str(),
                iter.second.c_str(),
                time.c_str(),
                _cnt);
        vec.push_back(buf);
        vec.push_back(",");
    }

    vec.pop_back();
    vec.push_back(" } }");

    _jData = std::accumulate(vec.begin(), vec.end(), std::string(""));

    return true;
}

std::string CNetconfPDBActive::getTimeString() {
    struct tm   stT;
    time_t t = time(nullptr);

    std::stringstream ss;
    ss << std::put_time(localtime_r(&t, &stT), "%Y%m%d%H%M%S");

    return ss.str();
}
