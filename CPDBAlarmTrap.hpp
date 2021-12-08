#ifndef CPDB_ALARM_TRAP_HPP
#define CPDB_ALARM_TRAP_HPP

#include <string>
#include <map>
#include <exception>

#include "rapidjson/document.h"
#include "PDBConnectionManager.hpp"

class CPDBAlarmTrap {
public:
    explicit CPDBAlarmTrap() = delete;
    CPDBAlarmTrap(const CPDBAlarmTrap & _rhs) = delete;
    ~CPDBAlarmTrap() = delete;

    static size_t Send(PDB::ConnectionManager * _cmgr);
    static bool trap(std::string & _str);
    static void parse(std::string & _action,
                    std::string & _metric,
                    std::string & _location,
                    std::string & _value,
                    std::map<std::string, std::string> & _mD,
                    std::map<std::string, std::string> & _mV,
                    rapidjson::Document & _doc) throw(std::exception);
    static void setValue(std::string & _out,
                        const char * _name,
                        rapidjson::Document & _doc) throw(std::exception);

private:




};


#endif // CPDB_ALARM_TRAP_HPP
