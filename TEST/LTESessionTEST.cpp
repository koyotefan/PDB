
#include "LTESessionTEST.hpp"
#include "PDBOdbcErr.hpp"

InsertLTESessionSQL::InsertLTESessionSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[st_.memberCnt];
}

InsertLTESessionSQL::~InsertLTESessionSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool InsertLTESessionSQL::Bind(std::string _tblName) {
    char buf[512];
    sprintf(buf,
        "INSERT INTO %s ("
        "NAME) "
        "VALUES (?)",
        _tblName.c_str()
    );

    if(SetSQL(PDB::eDefDBType::SessionDetail, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, st_.name, sizeof(st_.name), &arrLen_[0] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void InsertLTESessionSQL::SetValue(const char * _val) {
    strcpy(st_.name,      _val);
}

bool InsertLTESessionSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "InsertLTESessionSQL Success [%s] - SQLExecute", st_.name);

        SQLLEN  affectedCnt;
        // SQLRETURN r = SQLRowCount(_stmt, &affectedCnt);
        // D_THD_LOG(gLogName, "InsertLTESessionSQL RowCount[%zd]", affectedCnt);
   
        return true;
    }

    //std::string state = PDB::ODBCErr::GetStateSTMT(_stmt).c_str();
    //if(state.compare("23000") == 0)
    //    return true;

    D_THD_LOG(gLogName, "InsertLTESessionSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}


// DELETE ////////////////////////////////////////////////////


DeleteLTESessionSQL::DeleteLTESessionSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[st_.memberCnt];
}

DeleteLTESessionSQL::~DeleteLTESessionSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool DeleteLTESessionSQL::Bind(std::string _tblName) {
    char buf[128];
    sprintf(buf,
        "DELETE FROM %s WHERE NAME=?", _tblName.c_str());

    if(SetSQL(PDB::eDefDBType::SessionDetail, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, st_.name, sizeof(st_.name), &arrLen_[0] };
    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void DeleteLTESessionSQL::SetValue(std::string _name) {
    strcpy(st_.name, _name.c_str());
}

bool DeleteLTESessionSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO || sret_ == 100) {
        D_THD_LOG(gLogName, "DeleteLTESessionSQL Success - SQLExecute");

        SQLLEN  affectedCnt;
        SQLRETURN r = SQLRowCount(_stmt, &affectedCnt);
        D_THD_LOG(gLogName, "DeleteLTESessionSQL RowCount[%zd]", affectedCnt);
        return true;
    }

    D_THD_LOG(gLogName, "DeleteLTESessionSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}
