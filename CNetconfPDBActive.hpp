#ifndef C_NETCONF_PDB_ACTIVE_HPP
#define C_NETCONF_PDB_ACTIVE_HPP

#include "CNetconfBase.hpp"

class CNetconfPDBActive : public CNetconfBase {
public:
    explicit CNetconfPDBActive()
        : CNetconfBase("PCF_PDB_ACTIVE", "/PdbActive/pdb-type-list") {

        }

    virtual ~CNetconfPDBActive() {}

    bool Init() { return CNetconfBase::Init(); }
    bool LoadConfig();
    const char * GetData() const { return jData_.c_str(); }
    bool IsChanged() { return CNetconfBase::IsChanged(); }
    bool SaveConfig(std::unordered_map<std::string, std::string> & _m);

private:
    bool saveConfig(std::unordered_map<std::string, std::string> & _m, int _cnt);
    bool makeJsonData(std::string & _jData,
                      std::unordered_map<std::string, std::string> & _m,
                      int _cnt=0);
    std::string getTimeString();

private:
    std::string     jData_;
};


#endif // C_NETCONF_PDB_ACTIVE_HPP
