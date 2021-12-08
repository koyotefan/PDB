#ifndef     WORKER_TASL_HPP
#define     WORKER_TASL_HPP

#include <memory>
#include <thread>
#include <chrono>

#include "PDBConnectionManager.hpp"

class Base {
public:
    explicit Base() {
        D_THD_LOG(gLogName, "Base Construct");
    }

    virtual ~Base() {
        D_THD_LOG(gLogName, "Base Destroy");

        bRunF_.store(false);

        if(tid_.joinable())
            tid_.join();
    }

    void Init(std::shared_ptr<PDB::ConnectionManager>   _sp) {
        pdbConnectionManager_ = _sp;
    }

    virtual void Run() = 0;

protected:
    std::shared_ptr<bool>   pCounter_;
    std::atomic_bool        bRunF_;
    std::thread             tid_;

    std::shared_ptr<PDB::ConnectionManager>     pdbConnectionManager_;
};

class WorkerTask : public Base {
public:
    void Run() {
        tid_ = std::thread(&WorkerTask::run, this, pCounter_);
    }
private:
    void run(std::shared_ptr<bool>  _pCounter);
};

class SessionWorkerTask : public Base {
public:
    void Run() {
        tid_ = std::thread(&SessionWorkerTask::run, this, pCounter_);
    }
private:
    void run(std::shared_ptr<bool>  _pCounter);
};

class PdbStatusWorkerTask : public Base {
public:
    void Run() {
        tid_ = std::thread(&PdbStatusWorkerTask::run, this, pCounter_);
    }
private:
    void run(std::shared_ptr<bool>  _pCounter);
};

class PgNotiListWorkerTask : public Base {
public:
    void Run() {
        tid_ = std::thread(&PgNotiListWorkerTask::run, this, pCounter_);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
private:
    void run(std::shared_ptr<bool>  _pCounter);
};

class LTESessionWorkerTask : public Base {
public:
    void Run() {
        tid_ = std::thread(&LTESessionWorkerTask::run, this, pCounter_);
    }
private:
    void run(std::shared_ptr<bool>  _pCounter);
};

class PGLocalTask   : public Base {
public:
    void Run() {
        tid_ = std::thread(&PGLocalTask::run, this, pCounter_);
    }

    void Set(int _seed) {
        seed_ = _seed;
    }

private:
    void run(std::shared_ptr<bool>  _pCounter);

private:
    int     seed_;
};

class BusyTask   : public Base {
public:
    void Run() {
        tid_ = std::thread(&BusyTask::run, this, pCounter_);
    }

    void SetSleep(float _sleep) {
        sleep_ = _sleep;
    }

private:
    void run(std::shared_ptr<bool>  _pCounter);
    float sleep_ = 1;
};


#endif //   WORKER_TASL_HPP
