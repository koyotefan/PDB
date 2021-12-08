
#include "PgNotiListSQL.hpp"
#include "PDBOdbcErr.hpp"

InsertPgNotiListSQL::InsertPgNotiListSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[item_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[item_.memberCnt];
}

InsertPgNotiListSQL::~InsertPgNotiListSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool InsertPgNotiListSQL::Bind() {
    char buf[512];
    sprintf(buf,
        "INSERT INTO T_5G_PG_NOTI_LIST ("
        "MDN,"
        "TID,"
        "SYS,"
        "CREATE_TIME) "
        "VALUES (?,?,?,SYSDATE)"
    );

    if(SetSQL(PDB::eDefDBType::Subscriber, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 3;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_MDN, 0, item_.mdn, sizeof(item_.mdn), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_TID, 0, item_.tid, sizeof(item_.tid), &arrLen_[1] };
    arrBp_[2] = {3, SQL_C_CHAR, SQL_CHAR, LEN_SYS, 0, item_.sys,sizeof(item_.sys),&arrLen_[2] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void InsertPgNotiListSQL::SetValue(const stPgNotiList & _item) {
    // 걱정 말아요.. Default operation=() 잘 되요.
    item_ = _item;
}

bool InsertPgNotiListSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "InsertPgNotiListSQL Success - SQLExecute");
        return true;
    }

    D_THD_LOG(gLogName, "InsertPgNotiListSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}

