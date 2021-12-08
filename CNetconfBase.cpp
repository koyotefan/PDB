
#include "NDFServiceLog.hpp"
#include "CNDFATOMAdaptor.hpp"

#include "CNetconfBase.hpp"

CNetconfBase::CNetconfBase(std::string _modelName, std::string _path)
    : modelName_(_modelName),
      path_(_path),
      api_(nullptr),
      configVersion_(0),
      lastLoadingVersion_(0) {

}

CNetconfBase::~CNetconfBase() {
    api_ = nullptr;
}

bool CNetconfBase::Init() {

    if((api_ = G_ATOM_ADAPTOR.GetNetconfAPI()) == nullptr) {
        E_LOG("GetNetconfAPI() fail");
        return false;
    }

    try {

        api_->AddConfigCallbackForce(
            modelName_.c_str(),
            path_.c_str(),
            [this](atom::EN_CONFIG_CB_TYPE _op, string _path, string _old, string _new) {

                D_LOG("Callback : path [%s] old [%s] new [%s]",
                    _path.c_str(), _old.c_str(), _new.c_str());

                (void)_op;
                (void)_path;
                (void)_old;
                (void)_new;

                ++(this->configVersion_);
            });

    } catch ( atom::CException & e) {

        E_LOG("AddConfigCallbackForce() fail [%s]", e.what());
        return false;
    } catch( ... ) {

        E_LOG("AddConfigCallbackForce() fail");
        return false;
    }

    return true;
}

bool CNetconfBase::Load(rapidjson::Document & _doc) {
    I_LOG("Load");

    std::lock_guard<std::mutex>     guard(mutex_);

    std::string     jData = loadConfig();

    if(jData.length() == 0) {
        E_LOG("loadConfig() fail");
        return false;
    }

    D_LOG("[%s]", jData.c_str());

    if(_doc.Parse(jData.c_str()).HasParseError()) {
        E_LOG("json Parse() Error [%s]", jData.c_str());
        return false;
    }

    lastLoadingVersion_ = configVersion_;

    return true;
}

bool CNetconfBase::Load(std::string & _jData) {
    I_LOG("Load");

    std::lock_guard<std::mutex>     guard(mutex_);

    std::string     jData = loadConfig();

    if(jData.length() == 0) {
        E_LOG("loadConfig() fail");
        return false;
    }

    // D_LOG("[%s]", jData.c_str());

    _jData = jData;
    lastLoadingVersion_ = configVersion_;

    return true;
}

std::string CNetconfBase::loadConfig() {
    try
    {
        int id = api_->GetConfigReq(modelName_,
                                    path_,
                                    CNetconfAPI::SRC_RUNNING);

        return waitData(id);
    } catch (atom::CException & e) {

        E_LOG("in loadConfig() exception [%s]", e.what());
        return "";
    } catch ( ... ) {

        E_LOG("in loadConfig() exception");
        return "";
    }
}

std::string CNetconfBase::waitData(int _id) throw (atom::CException) {

    int     nCmd = 0;
    char    buf[1024];
    bool    bRecv = false;
    CResult ret;
    time_t  startT = time(nullptr);

    do {
        if(G_ATOM_ADAPTOR.GetCommand(&nCmd, buf) == false) {
            throw atom::CException(atom::ATOM_ERR_ETC, "GetCommand() fail");
        }

        if(api_->GetMessage(_id, ret) == EXIT_SUCCESS) {
            bRecv = true;
            break;
        }

    } while(time(nullptr) - startT < 30);

    if(bRecv == false) {
        throw atom::CException(atom::ATOM_ERR_ETC, "GetMessage() timeout");
    }

    std::string     data = ret.GetData();

    return data;
}
