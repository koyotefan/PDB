#ifndef C_NETCONF_BASE
#define C_NETCONF_BASE

#include <iostream>
#include <string>
#include <mutex>

#include "CNetconfAPI.hpp"
#include "rapidjson/document.h"


class CNetconfBase {

public:
    explicit CNetconfBase(std::string _modelName, std::string _path);
    virtual ~CNetconfBase();

    std::string GetModelName() { return modelName_; }
    std::string GetPath() { return path_; }

protected:
    bool Init();
    bool Load(rapidjson::Document & _doc);
    bool Load(std::string & _jData);
    bool IsChanged() { return configVersion_ != lastLoadingVersion_; }

private :
    std::string     waitData(int _id) throw (atom::CException);
    std::string     loadConfig();

protected:
    std::string     modelName_;
    std::string     path_;

    CNetconfAPI * api_;
    std::mutex     mutex_;

private:
    int     configVersion_;
    int     lastLoadingVersion_;
};


#endif // C_NETCONF_BASE
