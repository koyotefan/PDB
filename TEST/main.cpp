#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>

#include "PDBConnectionManager.hpp"
#include "WorkerTask.hpp"

int main(int argc, char const *argv[])
{
#ifndef __T_DEBUG
    NDF_OPEN_SERVICE_LOG("./", "pdbt", 1, 15, 0, 0);
    NDF_INIT_THD_LOG(gLogName);
#endif
    if(argc != 4) {

        E_THD_LOG(gLogName, "invalid argument");
        I_THD_LOG(gLogName, "ex) pdbt [nodeName] [procName] [config filename]");
        return 0;
    }


    auto pcm = std::make_shared<PDB::ConnectionManager>();

    if(pcm->Init(argv[1], argv[2], argv[3]) == false) {
        E_THD_LOG(gLogName, "ConnectionManager Init() fail");
        return 0;
    }
    pcm->SetLogName("MAIN");

    // For Instance#3

    //PgNotiListWorkerTask    task;
    //task.Init(pcm);
    //task.Run();

    // Subscriber Instance#3
    // WorkerTask 	task;
    // task.Init(pcm);
    // task.Run();

    // 5G SESSION DETAIL
    //SessionWorkerTask  swtask;
    //swtask.Init(pcm);
    //swtask.Run();

    //PdbStatusWorkerTask pstask;
    //pstask.Init(pcm);
    //pstask.Run();

    //LTESessionWorkerTask    lteTask;
    //lteTask.Init(pcm);
    //lteTask.Run();

    PGLocalTask     pgTask;
    pgTask.Init(pcm);
    pgTask.Run();

    //PGLocalTask     pgTask1;
    //pgTask1.Init(pcm);
    //pgTask1.Run();

    //PGLocalTask     pgTask2;
    //pgTask2.Init(pcm);
    //pgTask2.Run();

    //SliceDnnCntTask     sdtask;
    //sdtask.Init(pcm);
    //sdtask.Run();

    /*--
    if(strstr(argv[1], "BUSY") != nullptr) {
        BusyTask        busy;
        busy.Init(pcm);
        busy.SetSleep(0.1);
        busy.Run();
    }
    if(strstr(argv[1], "LAZY") != nullptr) {
        BusyTask        lazy;
        lazy.Init(pcm);
        lazy.SetSleep(36000);
        lazy.Run();
    }
    --*/

    //LTESessionWorkerTask    lteTask;
    //lteTask.Init(pcm);
    //lteTask.Run();

    return 0;
}
