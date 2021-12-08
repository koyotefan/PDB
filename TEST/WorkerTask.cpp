
#include "WorkerTask.hpp"
#include "PDBWorker.hpp"
#include "SubscriberSQL.hpp"
#include "SessionSMFSQL.hpp"
#include "PdbStatusSQL.hpp"
#include "PgNotiListSQL.hpp"
#include "LTESessionTEST.hpp"
#include "PGLocalSQL.hpp"
#include "BusySQL.hpp"
//#include "SliceDnnCntSQL.hpp"

#include <thread>
#include <poll.h>

void PgNotiListWorkerTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "PgNotiListWorkerTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "PgNotiListWorker::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::Subscriber) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

    InsertPgNotiListSQL iPg;
    if(iPg.Bind() == false) {
        E_THD_LOG(gLogName, "InsertPgNotiListSQL Bind() fail");
        return ;
    }

    stPgNotiList    item;
    sprintf(item.mdn, "01028071121");
    sprintf(item.tid, "00000000000");
    sprintf(item.sys, "PCF.01");
   
    iPg.SetValue(item);

    if(pdbW.Execute(iPg) == false) {
        E_THD_LOG(gLogName, "InsertPgNotiListSQL Execute() fail");
    } else {
        E_THD_LOG(gLogName, "InsertPgNotiListSQL Execute() success");
    }

}

void PdbStatusWorkerTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "PdbStatusWorkerTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "PdbStatusWorkerTask::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::PCF) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

    InsertPdbStatusSQL  iPdb;
    if(iPdb.Bind() == false) {
        E_THD_LOG(gLogName, "Insert Bind() fail");
        return ;
    }

    UpdatePdbStatusSQL  uPdb;
    if(uPdb.Bind() == false) {
        E_THD_LOG(gLogName, "Update Bind() fail");
        return ;
    }

    SelectPdbStatusSQL  sPdb;
    if(sPdb.Bind() == false) {
        E_THD_LOG(gLogName, "Select Bind() fail");
        return ;
    }

    DeletePdbStatusSQL  dPdb;
    if(dPdb.Bind() == false) {
        E_THD_LOG(gLogName, "Select Bind() fail");
        return ;
    }

    // INSERT TEST
    stPdbStatusItem     item;
    sprintf(item.nodeId, "NodeId");
    sprintf(item.procId, "ProcId");
    sprintf(item.pdbType, "PDBType");
    item.shardingId = 0;
    sprintf(item.dbName, "DBName");
    sprintf(item.dbIp, "127.0.0.1");
    item.dbPort = 3333;
    item.actCnt = 10;
    item.connCnt = 10;

    iPdb.SetValue(item);

    if(pdbW.Execute(iPdb) == false) {
        E_THD_LOG(gLogName, "Insert Execute() fail");
        return;
    }

    sPdb.SetValue(item);
    if(pdbW.Execute(sPdb) == false) {
        E_THD_LOG(gLogName, "Select Execute() fail");
        return;
    }

    stPdbStatusItem     st;
    sPdb.GetItem(st);
    I_THD_LOG(gLogName, "SELECT VALUE [%s:%s:%s:%d:%s:%s:%d:%d:%d]",
        st.nodeId,
        st.procId,
        st.pdbType,
        st.shardingId,
        st.dbName,
        st.dbIp,
        st.dbPort,
        st.actCnt,
        st.connCnt);

    item.actCnt = 0;
    item.connCnt = 0;

    uPdb.SetValue(item);

    if(pdbW.Execute(uPdb) == false) {
        E_THD_LOG(gLogName, "Update Execute() fail");
        return;
    }

    if(pdbW.Execute(sPdb) == false) {
        E_THD_LOG(gLogName, "Select Execute() fail");
        return;
    }

    sPdb.GetItem(st);
    I_THD_LOG(gLogName, "SELECT VALUE [%s:%s:%s:%d:%s:%s:%d:%d:%d]",
        st.nodeId,
        st.procId,
        st.pdbType,
        st.shardingId,
        st.dbName,
        st.dbIp,
        st.dbPort,
        st.actCnt,
        st.connCnt);

    dPdb.SetValue(st);
    if(pdbW.Execute(dPdb) == false) {
        E_THD_LOG(gLogName, "DELETE Execute() fail");
        return;
    }

    if(pdbW.Execute(sPdb) == false) {
        E_THD_LOG(gLogName, "Select Execute() fail - OK");
    }

    E_THD_LOG(gLogName, "PdbStatusWorkerTask Terminate");

    return ;
}

void SessionWorkerTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "SessionWorkerTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "SessionWorkerTask::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::SessionDetail) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

    InsertSMFSessionSQL     iSmf;
    if(iSmf.Bind() == false) {
        E_THD_LOG(gLogName, "Insert Bind() fail");
        return ;
    }

    DeleteSMFSessionSQL     dSmf;
    if(dSmf.Bind() == false) {
        E_THD_LOG(gLogName, "Delete Bind() fail");
        return ;
    }

    char    buf[10];
    int     i = 0;

    // int     toggle = 1;
    for(int nLoop=0; nLoop < 100000; nLoop++) {
        sprintf(buf, "%05d", ++i);

        iSmf.SetValue(i, buf);

        if(pdbW.Execute(iSmf, buf, strlen(buf)) == false) {
            E_THD_LOG(gLogName, "SessionWorkerTask::Insert fail");
        } else {
            I_THD_LOG(gLogName, "SessionWorkerTask::Insert Success");
        }
           

        dSmf.SetValue(buf);
        if(pdbW.Execute(dSmf, buf) == false) {
            E_THD_LOG(gLogName, "SessionWorkerTask::Delete fail");
        } else {
            I_THD_LOG(gLogName, "SessionWorkerTask::Delete Success");
        }

        /*-
        if(nLoop % 5 == 0) {

           std::string json("{}");
           int changedCnt = pdbConnectionManager_->SwitchActiveDB(json);
           I_THD_LOG(gLogName, "SwitchPDB cnt [%d]", changedCnt);

           //pdbConnectionManager_->SwitchActiveDB(PDB::eDefDBType::SessionDetail,
           //                                     static_cast<PDB::eDefClusterType>(toggle));
           if(toggle == 1)
              --toggle;
           else
              ++toggle;
        }
        -*/

        poll(nullptr, 0, 500);
    }

    I_THD_LOG(gLogName, "SessionWorkerTask Terminate [%d]", i);

    return;
}

void WorkerTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "WorkerTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "WorkerTask::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::Subscriber, false) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

    if(pdbW.TurnOn(PDB::eDefDBType::SessionDetail) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

    InsertSubsProfileSQL        iSubs;
    if(iSubs.Bind() == false) {
        E_THD_LOG(gLogName, "Insert Bind() fail");
        return ;
    }

    DeleteSubsProfileSQL        dSubs;
    if(dSubs.Bind() == false) {
        E_THD_LOG(gLogName, "Delete Bind() fail");
        return ;
    }


    while(true) {
        iSubs.SetMdn("01028071121");
        iSubs.SetProductId("Banana");
        if(pdbW.Execute(iSubs) == false) {
            E_THD_LOG(gLogName, "1) Insert Execute() fail");
            return ;
        } else {
            I_THD_LOG(gLogName, "1) Insert Execute() success");
        }

        dSubs.SetMdn("01028071121");
        if(pdbW.Execute(dSubs) == false) {
            E_THD_LOG(gLogName, "2) Delete Execute() fail");
        } else {
            I_THD_LOG(gLogName, "2) Delete Execute() success");
        }

        iSubs.SetMdn("01028071122");
        iSubs.SetProductId("Melon");

        if(pdbW.Execute(iSubs) == false) {
            E_THD_LOG(gLogName, "3) Insert Execute() fail");
        } else {
            I_THD_LOG(gLogName, "3) Insert Execute() success");


        }

        dSubs.SetMdn("01028071122");
        if(pdbW.Execute(dSubs) == false) {
            E_THD_LOG(gLogName, "4) Delete Execute() fail");
        } else {
            I_THD_LOG(gLogName, "4) Delete Execute() success");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }

/*-
    const char * ptr = "{\n"
    "\"PdbActive\":\n"
    "\{\n"
        "\"pdb-type-list\":\n"
        "\{\n"
            "\"name\": \"PCF\",\n"
            "\"active\": \"PCF_P\",\n"
            "\"update-time\": \"20190723171120\"\n"
        "},\n"
        "\"pdb-type-list\":\n"
        "{\n"
            "\"name\": \"Subscriber\",\n"
            "\"active\": \"Subscriber_P\",\n"
            "\"update-time\": \"20190723171120\"\n"
        "},\n"
        "\"pdb-type-list\":\n"
        "{\n"
            "\"name\": \"SessionDetail_0\",\n"
            "\"active\": \"SessionDetail_0_P\",\n"
            "\"update-time\": \"20190723171120\"\n"
        "},\n"
        "\"pdb-type-list\":\n"
        "{\n"
            "\"name\": \"SessionDetail_1\",\n"
            "\"active\": \"SessionDetail_1_P\",\n"
            "\"update-time\": \"20190723171120\"\n"
        "}\n"
    "}\n"
"}\n";

    std::string json =  ptr;
    int changedCnt = pdbConnectionManager_->SwitchActiveDB(json);
    I_THD_LOG(gLogName, "SwitchPDB cnt [%d]", changedCnt);
-*/

    return ;
}


void LTESessionWorkerTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "LTESessionWorkerTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "LTESessionWorkerTask::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::SessionDetail) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }


    // GetTotalLTESessionTBLCnt() 은 TurnOn(PDB::eDefDBType::SessionDetail); 이후에 가능합니다.
    int nTBLCnt = pdbW.GetTotalLTESessionTBLCnt();

    std::unique_ptr<InsertLTESessionSQL[]> 
        arrInsertLTESessionSQL(new InsertLTESessionSQL[nTBLCnt]);

    char tblName[64];   
    for(int nLoop=0; nLoop < nTBLCnt; nLoop++) {

        sprintf(tblName, "JHCHOI_%02d", nLoop);
        if(arrInsertLTESessionSQL[nLoop].Bind(tblName) == false) {
            E_THD_LOG(gLogName, "Insert Bind() [%s] fail", tblName);
            return; 
        }
    } 

   
    std::unique_ptr<DeleteLTESessionSQL[]> 
        arrDeleteLTESessionSQL(new DeleteLTESessionSQL[nTBLCnt]);

    for(int nLoop=0; nLoop < nTBLCnt; nLoop++) {

        sprintf(tblName, "JHCHOI_%02d", nLoop);
        if(arrDeleteLTESessionSQL[nLoop].Bind(tblName) == false) {
            E_THD_LOG(gLogName, "Delete Bind() [%s] fail", tblName);
            return; 
        }
    } 


    char    min[10+1];
    int     tblNumber = 0; 
    for(int nLoop=0; nLoop < 10; nLoop++) {
        sprintf(min, "101234000%02d", nLoop);

        tblNumber = pdbW.GetNumberOfSessionTBL(min, strlen(min));
        InsertLTESessionSQL & iSQL = arrInsertLTESessionSQL[tblNumber];

        iSQL.SetValue(min);

        if(pdbW.Execute(iSQL, tblNumber) == false) {
            E_THD_LOG(gLogName, "LTESessionWorkerTask::Insert fail");
        } else {
            I_THD_LOG(gLogName, "LTESessionWorkerTask::Insert Success");
        }

        poll(nullptr, 0, 500);
    }

    I_THD_LOG(gLogName, "LTESessionWorkerTask Terminate");

    return;
}

void PGLocalTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "PGLocalTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "PGLocalTask::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::PGLocal1) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

/*-
    if(pdbW.TurnOn(PDB::eDefDBType::PGLocal2) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }
-*/

    InsertPGLocalSQL        iLocal(PDB::eDefDBType::PGLocal1);
    if(iLocal.Bind() == false) {
        E_THD_LOG(gLogName, "Insert Bind() fail");
        return ;
    }

    DeletePGLocalSQL        dLocal(PDB::eDefDBType::PGLocal1);
    if(dLocal.Bind() == false) {
        E_THD_LOG(gLogName, "Delete Bind() fail");
        return ;
    }

/*-
    InsertPGLocalSQL        iTwo(PDB::eDefDBType::PGLocal2);
    if(iTwo.Bind() == false) {
        E_THD_LOG(gLogName, "Insert Bind() fail");
        return ;
    }

    DeletePGLocalSQL        dTwo(PDB::eDefDBType::PGLocal2);
    if(dTwo.Bind() == false) {
        E_THD_LOG(gLogName, "Delete Bind() fail");
        return ;
    }
-*/

    char    mdn[16];


    while(true) {
        iLocal.SetMdn("01028071121");
        iLocal.SetVal("Banana");
        if(pdbW.Execute(iLocal) == false) {
            E_THD_LOG(gLogName, "1) Insert Execute() - iLocal fail");
            // return ;
        } else {
            I_THD_LOG(gLogName, "1) Insert Execute() - iLocal success");
        }

/*-
        if(pdbW.ResetStmt(iLocal) == false) {
            E_THD_LOG(gLogName, "# ResetStmt() fail");
        } else {
            I_THD_LOG(gLogName, "# ResetStmt() success");
        } 
-*/

/*-
        iTwo.SetMdn("01028071122");
        iTwo.SetVal("Melon");

        if(pdbW.Execute(iTwo) == false) {
            E_THD_LOG(gLogName, "3) Insert Execute() - iTwo fail");
        } else {
            I_THD_LOG(gLogName, "3) Insert Execute() - iTwo success");

        }
-*/

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        dLocal.SetMdn("01028071121");
        if(pdbW.Execute(dLocal) == false) {
            E_THD_LOG(gLogName, "4) Delete Execute() - iLocal fail");
        } else {
            I_THD_LOG(gLogName, "4) Delete Execute() - iLocal success");
        }

/*-
        dTwo.SetMdn("01028071122");
        if(pdbW.Execute(dLocal) == false) {
            E_THD_LOG(gLogName, "2) Delete Execute() - dTwo fail");
        } else {
            I_THD_LOG(gLogName, "2) Delete Execute() - dTwo success");
        }
-*/

    }

    return ;
}

void BusyTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "BusyTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "BusyTask::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::PCF) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

    if(pdbW.TurnOn(PDB::eDefDBType::PGLocal1) == false) {
        E_THD_LOG(gLogName, "PGLocal1 TurnOn() Fail");
        bRunF_.store(false);
        return;
    }
   

    InsertBusySQL iSQL;
    if(iSQL.Bind() == false) {
        E_THD_LOG(gLogName, "Insert Bind() fail - InsertBusyTaskSQL");
        return ;
    }

    DeleteBusySQL  dSQL;
    if(dSQL.Bind() == false) {
        E_THD_LOG(gLogName, "Delete Bind() fail - DeleteBusyTaskSQL");
        return ;
    }

    SelectBusySQL  sSQL;
    if(sSQL.Bind() == false) {
        E_THD_LOG(gLogName, "Select Bind() fail - SelectBusyTaskSQL");
        return ;
    }

    I_THD_LOG(gLogName, "SLEEP [%f]", sleep_);

    unsigned int uVal = 0;

    char            buf[32];

    while(++uVal) {
       
        sprintf(buf, "%.11u", uVal); 
        
        iSQL.SetNAME(buf);
        if(pdbW.Execute(iSQL) == false) {
            E_THD_LOG(gLogName, "1) Insert Execute() - iSQL fail [%s]", buf);
            break;
        }

        sSQL.SetNAME(buf);
        if(pdbW.Execute(sSQL) == false) {
            E_THD_LOG(gLogName, "2) Select Execute() - sSQL fail [%s]", buf);
            break;
        } 

        dSQL.SetNAME(buf);
        if(pdbW.Execute(dSQL) == false) {
            E_THD_LOG(gLogName, "3) Delete Execute() - dSQL fail [%s]", buf);
            break;
        }
        

        if(uVal % 1000 == 0) {
            I_THD_LOG(gLogName, "BusyTask - working [%u] [%d]", uVal);
            sSQL.FetchToggle();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(int(sleep_ * 1000)));

    }

    return ;
}

/*-
void SliceDnnCntTask::run(std::shared_ptr<bool>   _pCounter) {
    (void)_pCounter;

    sprintf(gLogName, "SliceDnnCntTask");

#ifndef __T_DEBUG
    NDF_INIT_THD_LOG(gLogName);
#endif

    PDB::Worker     pdbW;
    pdbW.Assign(pdbConnectionManager_);

    I_THD_LOG(gLogName, "SliceDnnCntTask::run()");

    if(pdbW.TurnOn(PDB::eDefDBType::PCF) == false) {
        E_THD_LOG(gLogName, "TurnOn() Fail");
        bRunF_.store(false);
        return;
    }

    SelectRejectListSDCSQL setSQL;
    if(setSQL.Bind() == false) {
        E_THD_LOG(gLogName, "Select Bind() fail - SelectRejectListSDCSQL");
        return ;
    }

    UpdateRejectSDCSQL uOnSQL;
    if(uOnSQL.Bind() == false) {
        E_THD_LOG(gLogName, "update Bind() fail - UpdateRejectSDCSQL");
        return;
    }

    SelectAllowListSDCSQL relSQL;
    if(relSQL.Bind() == false) {
        E_THD_LOG(gLogName, "Select Bind() fail - SelectAllowListSDCSQL");
        return ;
    }

    UpdateAllowSDCSQL   uOffSQL;
    if(uOffSQL.Bind() == false) {
        E_THD_LOG(gLogName, "update Bind() fail - UpdateAllowSDCSQL");
        return;
    }


    std::vector<stSliceDnnCnt>  v;

    bRun = true;

    while(bRun) {
      
        if(pdbW.Execute(setSQL) == false) {
            E_THD_LOG(gLogName, "Select Execute() - setSQL fail");
            break;
        } 

        v.clear();
        setSQL.GetRows(v);

        for(auto & item : v) {
            uOnSQL.Set(item);
            if(pdbW.Execute(uOnSQL) == false) {
                E_THD_LOG(gLogName, "Update Execute() - uOnSQL fail");
                bRun = false;
                break;
            }
        }

        if(pdbW.Execute(relSQL) == false) {
            E_THD_LOG(gLogName, "Select Execute() - relSQL fail");
            break;
        }

        v.clear();
        relSQL.GetRows(v);

        for(auto & item : v) {
            uOffSQL.Set(item);
            if(pdbW.Execute(uOffSQL) == false) {
                E_THD_LOG(gLogName, "Update Execute() - uOffSQL fail");
                bRun = false;
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));

    }

    return ;
}
--*/
