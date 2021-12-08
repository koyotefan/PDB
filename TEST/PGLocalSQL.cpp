
#include "PGLocalSQL.hpp"
#include "PDBOdbcErr.hpp"

// Profile ///////////////////////////////////////

void SetValue(char * _target, size_t _targetSize, const char * _val) {
    if(_val == nullptr)
        return ;

    size_t len  = strlen(_val);

    len = (len >= _targetSize)?_targetSize-1:len;

    strncpy(_target, _val, len);
    _target[len] = '\0';
}

// InsertPGLocalSQL ///////////////////////////////////////

InsertPGLocalSQL::InsertPGLocalSQL(PDB::eDefDBType  _dbType) 
    : dbType_(_dbType) {
    arrLen_ = new (std::nothrow) SQLLEN[profile_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[profile_.memberCnt];
}

InsertPGLocalSQL::~InsertPGLocalSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool InsertPGLocalSQL::Bind() {

    char buf[512];

    sprintf(buf,
        "INSERT INTO T_PG_LOCAL_TEST ("
        "NAME, "
        "VAL) "
        "VALUES (?, ?)");

    if(SetSQL(dbType_, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 2;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop) {
        arrLen_[nLoop] = SQL_NTS;
    }

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, profile_.name, sizeof(profile_.name), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_VAL, 0, profile_.val, sizeof(profile_.val), &arrLen_[1] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;
}

void InsertPGLocalSQL::SetMdn(const char * _val) {
    ::SetValue(profile_.name, sizeof(profile_.name), _val);
}

void InsertPGLocalSQL::SetVal(const char * _val) {
    ::SetValue(profile_.val, sizeof(profile_.val), _val);
}

bool InsertPGLocalSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "InsertPGLocalSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "InsertPGLocalSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}


// DeletePGLocalSQL ///////////////////////////////////////

DeletePGLocalSQL::DeletePGLocalSQL(PDB::eDefDBType _dbType) 
    : dbType_(_dbType) {
    arrLen_ = new (std::nothrow) SQLLEN[profile_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[profile_.memberCnt];
}

DeletePGLocalSQL::~DeletePGLocalSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool DeletePGLocalSQL::Bind() {

    char buf[256];

    sprintf(buf, "DELETE FROM T_PG_LOCAL_TEST WHERE NAME=?");


    if(SetSQL(dbType_, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;

    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, profile_.name, sizeof(profile_.name), &arrLen_[0] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;
}

void DeletePGLocalSQL::SetMdn(const char * _val) {
    ::SetValue(profile_.name, sizeof(profile_.name), _val);
}

bool DeletePGLocalSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "DeletePGLocalSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "DeletePGLocalSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}

