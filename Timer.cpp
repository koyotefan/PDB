
#include <random>

#include "PDBTimer.hpp"

namespace PDB {

Timer::Timer(int _period)
    : period_(_period),
      lastT_(0),
      randomF_(false),
      marginPeriod_(_period) {

}

bool Timer::IsTimeout(const time_t _t) {

    if(_t < lastT_ + marginPeriod_)
        return false;

    lastT_ = _t;

    if(randomF_)
        marginPeriod_ = getRandomNumber(period_);

    return true;
}

bool Timer::IsTimeout(const time_t _beforeT, const int _period) {

    time_t now = time(nullptr);

    if(_beforeT + _period >= now)
        return false;

    return true;
}

// 입력 받은 _max 에서 1/2 값은 최소 Timeout 확인 값.
// 1/2 값 범위에서 random 하게 결정합니다.
// 예를 들어, max 가 3 이면,
// -----------------------------
// max   | 3       | 4
// -----------------------------
// rnd() | 0 or 1  | 0 or 1 or 2
// max/2 | 1       | 2
// max%2 | 1       | 0
// return| 2 or 3  | 2 or 3 or 4

// rand(rnd) 값은 0 or 1
// _max/2 값은 1
// _max%2 값은 1 입니다.
int Timer::getRandomNumber(int _max) {

    if(_max == 0)
        return 0;

    // 엄청 오래걸리는 연산이지만, 그래도 몇 초에 한번씩 하는거니까 괜찮지 않을까요?
    // 50 마이크로 초 정도..
    std::random_device rn;
    std::mt19937_64 rnd( rn() );

    std::uniform_int_distribution<int> range(_max/2, _max);

    int ret = range(rnd) + _max%2;

    return (ret > _max)?_max:ret;
}

}