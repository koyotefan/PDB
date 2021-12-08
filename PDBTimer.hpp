#ifndef PDB_TIMER_HPP
#define PDB_TIMER_HPP

#include <poll.h>

namespace PDB {

class Timer {
public:
    explicit Timer(int _period=5);
    ~Timer() = default;

    void SetPeriod(int _period) {
        period_ = (_period <= 0)?1:_period;
        marginPeriod_ = (randomF_)?getRandomNumber(period_):period_;
    }
    void RandomOn() {
        randomF_ = true;
    }
    void RandomOff() {
        randomF_ = false;
    }
    bool IsTimeout(const time_t _t = time(nullptr));
    void Sleep(int _period) {
        poll(nullptr, 0, _period);
    }

    static bool IsTimeout(const time_t _beforeT, const int _period) ;

private:
    int getRandomNumber(int _max);

private:
    int     period_;
    time_t  lastT_;

    bool    randomF_;
    int     marginPeriod_;
};



}


#endif // PDB_RANDOM_TIMER_HPP
