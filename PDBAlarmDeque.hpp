#ifndef PDB_ALARM_DEQUE_HPP
#define PDB_ALARM_DEQUE_HPP

#include <string>
#include <deque>
#include <mutex>

#include "CNDFATOMAdaptor.hpp"
#include "PDBDefine.hpp"
#include "PDBConnection.hpp"

namespace PDB {

const std::string ALL_DOWN   = "DUAL_DOWN";
const std::string DISCONNECT = "DISCONNECT";
const std::string CONNECT    = "CONNECT";

const std::string METRIC_NAME= "db.connect.status";

class AlarmDeque {
public:
    explicit AlarmDeque() = default;
    AlarmDeque(const AlarmDeque * _rhs) = delete;
    ~AlarmDeque() = default;

    size_t GetCount() const { return deque_.size(); }
    bool Pop(std::string & _jData);
    bool Push(const std::string & _location,
              const std::string & _value,
              const Connection * _c);

private:
    std::mutex                  mutex_;
    std::deque<std::string>     deque_;

};



}


#endif // PDB_ALARM_HPP
