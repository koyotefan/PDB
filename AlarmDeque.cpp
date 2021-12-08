
#include "PDBAlarmDeque.hpp"

namespace PDB {

bool AlarmDeque::Pop(std::string & _jData) {

    if(deque_.empty())
        return false;

    std::lock_guard<std::mutex>     guard(mutex_);
    _jData = deque_.front();
    deque_.pop_front();

    return true;
}

bool AlarmDeque::Push(const std::string & _location,
                      const std::string & _value,
                      const Connection * _c) {

    std::lock_guard<std::mutex>     guard(mutex_);

    char jsonData[256];
    snprintf(jsonData, sizeof(jsonData),
        "{ \"action\" : \"%s\",\n"
        " \"metric\" : \"%s\",\n"
        " \"location\" : \"%s\",\n"
        " \"value\" : \"%s\",\n"
        " \"dbname\" : \"%s\",\n"
        " \"ip\" : \"%s\",\n"
        " \"port\" : \"%d\"\n}",
        ATOM_TRAP_ACTION_NOTIFY,
        METRIC_NAME.c_str(),
        _location.c_str(),
        _value.c_str(),
        _c->GetDBName(),
        _c->GetIp(),
        _c->GetPort());


    deque_.emplace_back(jsonData);
    return true;
}

}