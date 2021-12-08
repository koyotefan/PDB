
#include "PdbStatusSQL.hpp"
#include "PDBOdbcErr.hpp"

InsertPdbStatusSQL::InsertPdbStatusSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[item_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[item_.memberCnt];
}

InsertPdbStatusSQL::~InsertPdbStatusSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool InsertPdbStatusSQL::Bind() {
    char buf[512];
    sprintf(buf,
        "INSERT INTO T_PCF_PDB_STATUS ("
        "NODE_ID,"
        "PROC_ID,"
        "PDB_TYPE,"
        "SHARDING_ID,"
        "DB_NAME,"
        "DB_IP,"
        "DB_PORT,"
        "ACTIVE,"
        "CONN,"
        "UPDATE_TIME) "
        "VALUES (?,?,?,?,?,?,?,?,?,SYSDATE)"
    );

    if(SetSQL(PDB::eDefDBType::PCF, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 9;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NODE_ID, 0, item_.nodeId, sizeof(item_.nodeId), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_PROC_ID, 0, item_.procId, sizeof(item_.procId), &arrLen_[1] };
    arrBp_[2] = {3, SQL_C_CHAR, SQL_CHAR, LEN_PDB_TYPE,0, item_.pdbType,sizeof(item_.pdbType),&arrLen_[2] };
    arrBp_[3] = {4, SQL_C_SLONG, SQL_INTEGER, 0, 0, &item_.shardingId, 0, &arrLen_[3] };
    arrBp_[4] = {5, SQL_C_CHAR, SQL_CHAR, LEN_DB_NAME, 0, item_.dbName, sizeof(item_.dbName),&arrLen_[4] };
    arrBp_[5] = {6, SQL_C_CHAR, SQL_CHAR, LEN_DB_IP, 0, item_.dbIp, sizeof(item_.dbName),&arrLen_[5] };
    arrBp_[6] = {7, SQL_C_SLONG, SQL_INTEGER, 0, 0, &item_.dbPort, 0, &arrLen_[6] };
    arrBp_[7] = {8, SQL_C_SLONG, SQL_INTEGER, 0, 0, &item_.actCnt, 0, &arrLen_[7] };
    arrBp_[8] = {9, SQL_C_SLONG, SQL_INTEGER, 0, 0, &item_.connCnt, 0, &arrLen_[8] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void InsertPdbStatusSQL::SetValue(const stPdbStatusItem & _item) {
    // 걱정 말아요.. Default operation=() 잘 되요.
    item_ = _item;
}

bool InsertPdbStatusSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "InsertPdbStatusSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "InsertPdbStatusSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}


// DELETE ////////////////////////////////////////////////////


DeletePdbStatusSQL::DeletePdbStatusSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[item_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[item_.memberCnt];
}

DeletePdbStatusSQL::~DeletePdbStatusSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool DeletePdbStatusSQL::Bind() {
    char buf[128];
    sprintf(buf,
        "DELETE FROM T_PCF_PDB_STATUS WHERE "
        "NODE_ID=? "
        "AND PROC_ID=? "
        "AND PDB_TYPE=? "
        "AND DB_NAME=?");

    if(SetSQL(PDB::eDefDBType::PCF, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 4;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NODE_ID, 0, item_.nodeId, sizeof(item_.nodeId), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_PROC_ID, 0, item_.procId, sizeof(item_.procId), &arrLen_[1] };
    arrBp_[2] = {3, SQL_C_CHAR, SQL_CHAR, LEN_PDB_TYPE,0, item_.pdbType,sizeof(item_.pdbType),&arrLen_[2] };
    arrBp_[3] = {4, SQL_C_CHAR, SQL_CHAR, LEN_DB_NAME, 0, item_.dbName, sizeof(item_.dbName),&arrLen_[3] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void DeletePdbStatusSQL::SetValue(const stPdbStatusItem &  _item) {
    // 걱정 말아요.. Default operation=() 잘 되요.
    item_ = _item;
}

bool DeletePdbStatusSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO || sret_ == 100) {
        D_THD_LOG(gLogName, "DeletePdbStatusSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "DeletePdbStatusSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}


// SELECT ////////////////////////////////////////////////////


SelectPdbStatusSQL::SelectPdbStatusSQL() {

    arrInd_ = new (std::nothrow) SQLLEN[item_.memberCnt];
    arrBc_ = new (std::nothrow) STBindColumn[item_.memberCnt];

    arrLen_ = new (std::nothrow) SQLLEN[item_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[item_.memberCnt];
}

SelectPdbStatusSQL::~SelectPdbStatusSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }

    if(arrInd_) {
        delete [] arrInd_;
        arrInd_ = nullptr;
    }

    if(arrBc_) {
        delete [] arrBc_;
        arrBc_ = nullptr;
    }
}

bool SelectPdbStatusSQL::Bind() {

    char buf[256];
    sprintf(buf,
        "SELECT "
            "SHARDING_ID,"
            "DB_IP,"
            "DB_PORT,"
            "ACTIVE,"
            "CONN "
        "FROM T_PCF_PDB_STATUS WHERE "
            "NODE_ID=? "
            "AND PROC_ID=? "
            "AND PDB_TYPE=? "
            "AND DB_NAME=?");

    if(SetSQL(PDB::eDefDBType::PCF, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 4;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NODE_ID, 0, item_.nodeId, sizeof(item_.nodeId), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_PROC_ID, 0, item_.procId, sizeof(item_.procId), &arrLen_[1] };
    arrBp_[2] = {3, SQL_C_CHAR, SQL_CHAR, LEN_PDB_TYPE,0, item_.pdbType,sizeof(item_.pdbType),&arrLen_[2] };
    arrBp_[3] = {4, SQL_C_CHAR, SQL_CHAR, LEN_DB_NAME, 0, item_.dbName, sizeof(item_.dbName),&arrLen_[3] };

    SetBindParameter(arrBp_, parameterCnt);

    arrBc_[0] = {1, SQL_C_SLONG, &item_.shardingId, sizeof(item_.shardingId), &arrInd_[0] };
    arrBc_[1] = {2, SQL_C_CHAR, &item_.dbIp, sizeof(item_.dbIp)-1, &arrInd_[1] };
    arrBc_[2] = {3, SQL_C_SLONG, &item_.dbPort, sizeof(item_.dbPort), &arrInd_[2] };
    arrBc_[3] = {4, SQL_C_SLONG, &item_.actCnt, sizeof(item_.actCnt), &arrInd_[3] };
    arrBc_[4] = {5, SQL_C_SLONG, &item_.connCnt, sizeof(item_.connCnt), &arrInd_[4] };

    SetBindColumn(arrBc_, 5);

    return true;
}

void SelectPdbStatusSQL::SetValue(const stPdbStatusItem &  _item) {
    // 걱정 말아요.. Default operation=() 잘 되요.
    item_ = _item;
}

bool SelectPdbStatusSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "SelectPdbStatusSQL SQLExecute() fail [%d] [%s]",
            sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
        return false;
    }

    sret_ = SQLFetch(_stmt);

    // NO DATA 도 true Return 하니까 조심하세요.
    if(sret_ == SQL_NO_DATA) {
        D_THD_LOG(gLogName, "SelectPdbStatusSQL SQL_NO_DATA");
        return false;
    }

    if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "SelectPdbStatusSQL SQLFetch() fail [%d] [%s]",
            sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
        return false;
    }

    return true;
}

// UPDATE ////////////////////////////////////////////////////


UpdatePdbStatusSQL::UpdatePdbStatusSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[item_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[item_.memberCnt];
}

UpdatePdbStatusSQL::~UpdatePdbStatusSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool UpdatePdbStatusSQL::Bind() {
    char buf[128];
    sprintf(buf,
        "UPDATE T_PCF_PDB_STATUS SET "
            "ACTIVE=?,"
            "CONN=? "
        "WHERE "
            "NODE_ID=? "
            "AND PROC_ID=? "
            "AND PDB_TYPE=? "
            "AND DB_NAME=?");

    if(SetSQL(PDB::eDefDBType::PCF, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 6;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_SLONG, SQL_INTEGER, 0, 0, &item_.actCnt, 0, &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_SLONG, SQL_INTEGER, 0, 0, &item_.connCnt, 0, &arrLen_[1] };
    arrBp_[2] = {3, SQL_C_CHAR, SQL_CHAR, LEN_NODE_ID, 0, item_.nodeId, sizeof(item_.nodeId), &arrLen_[2] };
    arrBp_[3] = {4, SQL_C_CHAR, SQL_CHAR, LEN_PROC_ID, 0, item_.procId, sizeof(item_.procId), &arrLen_[3] };
    arrBp_[4] = {5, SQL_C_CHAR, SQL_CHAR, LEN_PDB_TYPE,0, item_.pdbType,sizeof(item_.pdbType),&arrLen_[4] };
    arrBp_[5] = {6, SQL_C_CHAR, SQL_CHAR, LEN_DB_NAME, 0, item_.dbName, sizeof(item_.dbName),&arrLen_[5] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void UpdatePdbStatusSQL::SetValue(const stPdbStatusItem &  _item) {
    // 걱정 말아요.. Default operation=() 잘 되요.
    item_ = _item;
}

bool UpdatePdbStatusSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "UpdatePdbStatusSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "UpdatePdbStatusSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}

