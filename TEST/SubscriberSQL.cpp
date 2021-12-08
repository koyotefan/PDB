
#include "SubscriberSQL.hpp"
#include "PDBOdbcErr.hpp"

// SubscriberProfile ///////////////////////////////////////

void Subscriber::SetValue(char * _target, size_t _targetSize, const char * _val) {
    if(_val == nullptr)
        return ;

    size_t len  = strlen(_val);

    len = (len >= _targetSize)?_targetSize-1:len;

    strncpy(_target, _val, len);
    _target[len] = '\0';
}

// InsertSubsProfileSQL ///////////////////////////////////////

InsertSubsProfileSQL::InsertSubsProfileSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[profile_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[profile_.memberCnt];
}

InsertSubsProfileSQL::~InsertSubsProfileSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool InsertSubsProfileSQL::Bind() {

    char buf[512];

    sprintf(buf,
        "INSERT INTO T_5G_SUBS_PROFILE ("
        "MDN, "
        "PRODUCT_ID ) "
        "VALUES (?, ?)");

    if(SetSQL(PDB::eDefDBType::Subscriber, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 2;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop) {
        arrLen_[nLoop] = SQL_NTS;
    }

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_MDN, 0, profile_.mdn, sizeof(profile_.mdn), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_PRODUCT_ID, 0, profile_.productId, sizeof(profile_.productId), &arrLen_[1] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;
}

void InsertSubsProfileSQL::SetMdn(const char * _val) {
    Subscriber::SetValue(profile_.mdn, sizeof(profile_.mdn), _val);
}

void InsertSubsProfileSQL::SetProductId(const char * _val) {
    Subscriber::SetValue(profile_.productId, sizeof(profile_.productId), _val);
}

bool InsertSubsProfileSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "InsertSubsProfileSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "InsertSubsProfileSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}

// Select /////////////////////////////////////////////////////////////////

SelectSubsProfileSQL::SelectSubsProfileSQL() {
    arrInd_ = new (std::nothrow) SQLLEN[profile_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[profile_.memberCnt];

    arrLen_ = new (std::nothrow) SQLLEN[profile_.memberCnt];
    arrBc_  = new (std::nothrow) STBindColumn[profile_.memberCnt];
}

SelectSubsProfileSQL::~SelectSubsProfileSQL() {

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

bool SelectSubsProfileSQL::Bind() {

    char buf[256];
    sprintf(buf,
        "SELECT "
            "IMSI,"
            "MDN,"
            "MSISDN,"
            "GROUP_ID,"
            "MVNO,"
            "OCS,"
            "UE_OS_VER,"
            "UE_MODEL,"
            "UE_CATEGORY_LTE,"
            "UE_CATEGORY_5G,"
            "PRODUCT_ID,"
            "BLACK_LIST"
        " FROM T_5G_SUBS_PROFILE WHERE "
        " MDN=?");

    if(SetSQL(PDB::eDefDBType::Subscriber, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;
    for(int nLoop=0; nLoop < profile_.memberCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_IMSI, 0, profile_.mdn, sizeof(profile_.mdn), &arrLen_[0] };

    SetBindParameter(arrBp_, parameterCnt);

    arrBc_[0] = {1, SQL_C_CHAR, profile_.imsi, sizeof(profile_.imsi)-1, &arrInd_[0] };
    arrBc_[1] = {2, SQL_C_CHAR, profile_.mdn, sizeof(profile_.mdn)-1, &arrInd_[1] };
    arrBc_[2] = {3, SQL_C_CHAR, profile_.msisdn, sizeof(profile_.msisdn)-1, &arrInd_[2] };
    arrBc_[3] = {4, SQL_C_CHAR, profile_.groupId, sizeof(profile_.groupId)-1, &arrInd_[3] };
    arrBc_[4] = {5, SQL_C_CHAR, profile_.mvno, sizeof(profile_.mvno)-1, &arrInd_[4] };
    arrBc_[5] = {6, SQL_C_CHAR, profile_.ocs, sizeof(profile_.ocs)-1, &arrInd_[5] };
    arrBc_[6] = {7, SQL_C_CHAR, profile_.ueOsVer, sizeof(profile_.ueOsVer)-1, &arrInd_[6] };
    arrBc_[7] = {8, SQL_C_CHAR, profile_.ueModel, sizeof(profile_.ueModel)-1, &arrInd_[7] };
    arrBc_[8] = {9, SQL_C_CHAR, profile_.ueCategoryLTE, sizeof(profile_.ueCategoryLTE)-1, &arrInd_[8] };
    arrBc_[9] = {10, SQL_C_CHAR,profile_.ueCategory5G, sizeof(profile_.ueCategory5G)-1, &arrInd_[9] };
    arrBc_[10] = {11, SQL_C_CHAR, profile_.productId, sizeof(profile_.productId)-1, &arrInd_[10] };
    arrBc_[11] = {12, SQL_C_CHAR, profile_.blackList, sizeof(profile_.blackList)-1, &arrInd_[11] };

    SetBindColumn(arrBc_, 12);

    return true;

}

void SelectSubsProfileSQL::SetMdn(const char * _val) {
    Subscriber::SetValue(profile_.mdn, sizeof(profile_.mdn), _val);
}

bool SelectSubsProfileSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "SelectSubsProfileSQL fail- SQLExecute");
        return false;
    }

    while(true) {
        sret_ = SQLFetch(_stmt);

        if(sret_ == SQL_NO_DATA)
            break;

        if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {

            D_THD_LOG(gLogName, "SelectSubsProfileSQL SQLFetch() fail [%d] [%s]",
                sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
            return false;
        }
    }

    D_THD_LOG(gLogName, "SelectSubsProfileSQL success");
    return true;
}

// DeleteSubsProfileSQL ///////////////////////////////////////

DeleteSubsProfileSQL::DeleteSubsProfileSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[profile_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[profile_.memberCnt];
}

DeleteSubsProfileSQL::~DeleteSubsProfileSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool DeleteSubsProfileSQL::Bind() {

    char buf[256];

    sprintf(buf, "DELETE FROM T_5G_SUBS_PROFILE WHERE MDN=?");


    if(SetSQL(PDB::eDefDBType::Subscriber, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;

    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_MDN, 0, profile_.mdn, sizeof(profile_.mdn), &arrLen_[0] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;
}

void DeleteSubsProfileSQL::SetMdn(const char * _val) {
    Subscriber::SetValue(profile_.mdn, sizeof(profile_.mdn), _val);
}

bool DeleteSubsProfileSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "DeleteSubsProfileSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "DeleteSubsProfileSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}

