#ifndef PDB_STATUS_SQL_HPP
#define PDB_STATUS_SQL_HPP

#include "PDBPreparedSQL.hpp"
#include "PDBDataRecord.hpp"

#define LEN_NODE_ID     64
#define LEN_PROC_ID     64
#define LEN_PDB_TYPE    32
#define LEN_DB_NAME     64
#define LEN_DB_IP       64
#define LEN_TIME

struct stPdbStatusItem {
    char     nodeId[LEN_NODE_ID+1] = {'\0'};
    char     procId[LEN_PROC_ID+1] = {'\0'};
    char     pdbType[LEN_PDB_TYPE+1] = {'\0'};
    int      shardingId = 0;
    char     dbName[LEN_DB_NAME+1] = {'\0'};
    char     dbIp[LEN_DB_IP+1] = {'\0'};
    int      dbPort  = 0;
    int      actCnt  = 0;
    int      connCnt = 0;
    int      memberCnt = 9;
};

class InsertPdbStatusSQL : public PDB::PreparedSQL {
public:
    explicit InsertPdbStatusSQL();
    ~InsertPdbStatusSQL();

    bool Bind();
    void SetValue(const stPdbStatusItem & _item);

    bool Execute(SQLHSTMT & _stmt);

private:
    stPdbStatusItem   item_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class DeletePdbStatusSQL : public PDB::PreparedSQL {
public:
    explicit DeletePdbStatusSQL();
    ~DeletePdbStatusSQL();

    bool Bind();
    void SetValue(const stPdbStatusItem & _item);
    bool Execute(SQLHSTMT & _stmt);

private:
    stPdbStatusItem   item_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class SelectPdbStatusSQL : public PDB::PreparedSQL {
public:
    explicit SelectPdbStatusSQL();
    ~SelectPdbStatusSQL();

    bool Bind();
    void SetValue(const stPdbStatusItem & _item);
    void GetItem(stPdbStatusItem & _item) { _item = item_; }

    // NO DATA 도 true Return 하니까 조심하세요.
    bool Execute(SQLHSTMT & _hstmt);

private:
    stPdbStatusItem   item_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;

    SQLLEN *          arrInd_;
    STBindColumn *    arrBc_;
};

// 자동 절체를 위한 SELECT 를 만들어야 해요.

class UpdatePdbStatusSQL : public PDB::PreparedSQL {
public:
    explicit UpdatePdbStatusSQL();
    ~UpdatePdbStatusSQL();

    bool Bind();
    void SetValue(const stPdbStatusItem & _item);
    bool Execute(SQLHSTMT & _stmt);

private:
    stPdbStatusItem   item_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

#endif // PDB_STATUS_SQL_HPP
