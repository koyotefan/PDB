#ifndef PDB_DEFINE_HPP
#define PDB_DEFINE_HPP

#include <thread>
#include <string>
#include <sqlext.h>
#include <sys/syscall.h>

extern thread_local     char gLogName[32];

#ifdef __T_DEBUG
    #include <cstring>
    #define I_THR_LOG(_logName, msg, ...)     printf("[%16s:%04d]" msg "\n", basename(__FILE__),__LINE__, ## __VA_ARGS__)
    #define D_THD_LOG(_logName, msg, ...)     printf("[%16s:%04d]" msg "\n", basename(__FILE__),__LINE__, ## __VA_ARGS__)
    #define W_THR_LOG(_logName, msg, ...)     printf("[%16s:%04d]" msg "\n", basename(__FILE__),__LINE__, ## __VA_ARGS__)
    #define E_THR_LOG(_logName, msg, ...)     printf("[%16s:%04d]" msg "\n", basename(__FILE__),__LINE__, ## __VA_ARGS__)
#else
    #include "NDFServiceLog.hpp"
#endif

namespace PDB {

// MAX 32 개 까지 가능
// SessionDetail 은 언제나 마지막에 정의되어 있어야 해요.
enum class eDefDBType : size_t {
    PCF,
    SessionMaster,
    Subscriber,
    PG,
    PGLocal1,
    PGLocal2,
    SessionDetail
};

enum class eDefClusterType : size_t {
    P,
    S,
    Cnt
};

class ToString {
public:
    explicit ToString() = delete;
    ~ToString() = delete;

    static const char * Get(const eDefDBType _type) {
        switch(_type) {
        case eDefDBType::PCF:               return "PCF";
        case eDefDBType::SessionMaster:     return "SessionMaster";
        case eDefDBType::Subscriber:        return "Subscriber";
        case eDefDBType::PG:                return "PG";
        case eDefDBType::SessionDetail:     return "SessionDetail";
        case eDefDBType::PGLocal1:          return "PGLocal1";
        case eDefDBType::PGLocal2:          return "PGLocal2";
        default:                            break;
        }

        return "Unkwon";
    }

    static const char * Get(const eDefClusterType _type) {
        switch(_type) {
        case eDefClusterType::P:     return "P";
        case eDefClusterType::S:     return "S";
        default:                     break;
        }

        return "N";
    }
};

class ToEnumClass {
public:
    explicit ToEnumClass() = delete;
    ~ToEnumClass() = delete;

    static eDefDBType GetDBType(const std::string & _str) {
        if(_str.compare("PCF") == 0)
            return eDefDBType::PCF;

        if(_str.compare("SessionMaster") == 0)
            return eDefDBType::SessionMaster;

        if(_str.compare("Subscriber") == 0)
            return eDefDBType::Subscriber;

        if(_str.compare("PG") == 0)
            return eDefDBType::PG;

        if(_str.compare("SessionDetail") == 0)
            return eDefDBType::SessionDetail;

        if(_str.compare("PGLocal1") == 0)
            return eDefDBType::PGLocal1;

        if(_str.compare("PGLocal2") == 0)
            return eDefDBType::PGLocal2;

        return eDefDBType::PCF;
    }

    static eDefClusterType GetClusterType(const std::string & _str) {
        if(_str.compare("P") == 0)
            return eDefClusterType::P;

        if(_str.compare("S") == 0)
            return eDefClusterType::S;

        return eDefClusterType::P;
    }
};


// 전체 PDB Instance 종류를 확인하려고 넣은 값 이예요. 총 36가지 DB Instance 를 연결할 수 있지요.
enum class eDefValue : size_t {
    NonShardingCnt = 6,
    MaxShardingCnt = 32
};

struct stConnectionInfo {

    std::string     DBName;
    std::string     DSN;
    std::string     Host;
    std::string     Id;
    std::string     Pwd;
    int             nConnType = 1;
    bool            bNcharCheck = true;
    int             nPort = 20300;
    int             nTimeout = 5;
    int             nConnectionTimeout = 3;

    stConnectionInfo & operator=(const stConnectionInfo & _st) {
        if(this != &_st) {
            DBName = _st.DBName;
            DSN  = _st.DSN;
            Host = _st.Host;
            Id   = _st.Id;
            Pwd  = _st.Pwd;
            nConnType = _st.nConnType;
            bNcharCheck = _st.bNcharCheck;
            nPort = _st.nPort;
            nTimeout = _st.nTimeout;
            nConnectionTimeout = _st.nConnectionTimeout;
        }

        return *this;
    }
};

struct stStmtInfo {
    SQLHSTMT        hstmt = SQL_NULL_HSTMT;
    unsigned long   uuidForConnection = 0;

    // 소멸자를 만들면 안되요... hstmt 는 pointer 라네요..
};

const int COL_LEN_NODE_ID = 64;
const int COL_LEN_PROC_ID = 64;
const int COL_LEN_PDB_TYPE = 32;
const int COL_LEN_DB_NAME = 64;
const int COL_LEN_DB_IP = 64;


struct stPdbStatusItem {
    char     nodeId[COL_LEN_NODE_ID+1] = {0, };
    char     procName[COL_LEN_PROC_ID+1] = {0, };
    char     pdbType[COL_LEN_PDB_TYPE+1] = {0, };
    int      shardingId = 0;
    char     dbName[COL_LEN_DB_NAME+1] = {0, };
    char     dbIp[COL_LEN_DB_IP+1] = {0, };
    int      dbPort  = 0;
    int      actCnt  = 0;
    int      connCnt = 0;

    int      memberCnt = 9;

    stPdbStatusItem & operator=(const stPdbStatusItem & _st) {
        if(this != &_st)
            memcpy(this, &_st, sizeof(_st));

        return *this;
    }

    bool Compare(const stPdbStatusItem & _st) {
        if(this == &_st)
            return true;

        if(actCnt != _st.actCnt)
            return false;

        if(connCnt != _st.connCnt)
            return false;

        if(strlen(nodeId) != strlen(_st.nodeId) || memcmp(nodeId, _st.nodeId, strlen(nodeId)) != 0)
            return false;

        if(strlen(procName) != strlen(_st.procName) || memcmp(procName, _st.procName, strlen(procName)) != 0)
            return false;

        if(strlen(pdbType) != strlen(_st.pdbType) || memcmp(pdbType, _st.pdbType, strlen(pdbType)) != 0)
            return false;

        if(shardingId != _st.shardingId)
            return false;

        if(strlen(dbName) != strlen(_st.dbName) || memcmp(dbName, _st.dbName, strlen(dbName)) != 0)
            return false;

        if(strlen(dbIp) != strlen(_st.dbIp) || memcmp(dbIp, _st.dbIp, strlen(dbIp)) != 0)
            return false;

        if(dbPort != _st.dbPort)
            return false;

        return true;
    }
};

const int minHangupValidTime = 60;

const size_t sessionTBLCntByInstance = 2;

}

#endif // PDB_DEFINE_HPP
