
#include "SessionSMFSQL.hpp"
#include "PDBOdbcErr.hpp"

InsertSMFSessionSQL::InsertSMFSessionSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[st_.memberCnt];
}

InsertSMFSessionSQL::~InsertSMFSessionSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool InsertSMFSessionSQL::Bind() {
    char buf[512];
    sprintf(buf,
        "INSERT INTO T_SMF_SESSION_INFO ("
        "SM_POLICY_ID,"
        "SUPI,"
        "PDU_SESSION_ID,"
        "DNN,"
        "S_NSSAI_SST,"
        "IN_HTTP_HEADER,"
        "IN_HTTP_PAYLOAD,"
        "RES_URI,"
        "NOTI_URI,"
        "STATUS,"
        "NODE_ID,"
        "PROC_ID,"
        "SMF_ID,"
        "CONN_ID,"
        "STREAM_ID,"
        "CREATE_TIME) "
        "VALUES (?,?,?,?,?,?,?,?,?,'1',?,?,?,?,?,SYSDATE)"
    );

    if(SetSQL(PDB::eDefDBType::SessionDetail, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 14;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_SM_POLICY_ID, 0, st_.smPolicyId, sizeof(st_.smPolicyId), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_SUPI, 0, st_.supi, sizeof(st_.supi), &arrLen_[1] };
    arrBp_[2] = {3, SQL_C_SLONG, SQL_INTEGER, 0, 0, &st_.pduSessionId, 0, &arrLen_[2] };
    arrBp_[3] = {4, SQL_C_CHAR, SQL_CHAR, LEN_DNN, 0, st_.dnn, sizeof(st_.dnn), &arrLen_[3] };
    arrBp_[4] = {5, SQL_C_SLONG, SQL_INTEGER, 0, 0, &st_.s_nssai_sst, 0, &arrLen_[4] };
    arrBp_[5] = {6, SQL_C_CHAR, SQL_CHAR, LEN_IN_HTTP_HEADER, 0, st_.inHttpHeader, sizeof(st_.inHttpHeader), &arrLen_[5] };
    arrBp_[6] = {7, SQL_C_CHAR, SQL_CHAR, LEN_IN_HTTP_PAYLOAD, 0, st_.inHttpPayload, sizeof(st_.inHttpPayload), &arrLen_[6] };
    arrBp_[7] = {8, SQL_C_CHAR, SQL_CHAR, LEN_RES_URI, 0, st_.resUri, sizeof(st_.resUri), &arrLen_[7] };
    arrBp_[8] = {9, SQL_C_CHAR, SQL_CHAR, LEN_NOTI_URI, 0, st_.notiUri, sizeof(st_.notiUri), &arrLen_[8] };
    arrBp_[9] = {10, SQL_C_CHAR, SQL_CHAR, LEN_NODE_ID, 0, st_.nodeId, sizeof(st_.nodeId), &arrLen_[9] };
    arrBp_[10] = {11, SQL_C_CHAR, SQL_CHAR, LEN_PROC_ID, 0, st_.procId, sizeof(st_.procId), &arrLen_[10] };
    arrBp_[11] = {12, SQL_C_CHAR, SQL_CHAR, LEN_SMF_ID, 0, st_.smfId, sizeof(st_.smfId), &arrLen_[11] };
    arrBp_[12] = {13, SQL_C_SLONG, SQL_INTEGER, 0, 0, &st_.connId, 0, &arrLen_[12] };
    arrBp_[13] = {14, SQL_C_SLONG, SQL_INTEGER, 0, 0, &st_.streamId, 0, &arrLen_[13] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void InsertSMFSessionSQL::SetValue(int _n, const char * _val) {
    st_.connId = _n;
    st_.streamId  = _n;

    strcpy(st_.smPolicyId,      _val);
    strcpy(st_.supi,            _val);
    st_.pduSessionId = _n;
    strcpy(st_.dnn,             _val);
    st_.s_nssai_sst = _n;
    strcpy(st_.inHttpHeader,    _val);
    strcpy(st_.inHttpPayload,   _val);
    strcpy(st_.resUri,          _val);
    strcpy(st_.notiUri,         _val);
    strcpy(st_.nodeId,          _val);
    strcpy(st_.procId,          _val);
    strcpy(st_.smfId,          _val);
}

bool InsertSMFSessionSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO) {
        D_THD_LOG(gLogName, "InsertSMFSessionSQL Success [%s] - SQLExecute", st_.smPolicyId);

        SQLLEN  affectedCnt;
        SQLRETURN r = SQLRowCount(_stmt, &affectedCnt);
        D_THD_LOG(gLogName, "InsertSMFSessionSQL RowCount[%zd]", affectedCnt);
   
        return true;
    }

    //std::string state = PDB::ODBCErr::GetStateSTMT(_stmt).c_str();
    //if(state.compare("23000") == 0)
    //    return true;

    D_THD_LOG(gLogName, "InsertSMFSessionSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}


// DELETE ////////////////////////////////////////////////////


DeleteSMFSessionSQL::DeleteSMFSessionSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[st_.memberCnt];
}

DeleteSMFSessionSQL::~DeleteSMFSessionSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool DeleteSMFSessionSQL::Bind() {
    char buf[128];
    sprintf(buf,
        "DELETE FROM T_SMF_SESSION_INFO WHERE SM_POLICY_ID=?");

    if(SetSQL(PDB::eDefDBType::SessionDetail, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_SM_POLICY_ID, 0, st_.smPolicyId, sizeof(st_.smPolicyId), &arrLen_[0] };
    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void DeleteSMFSessionSQL::SetValue(std::string _smPolicyId) {
    strcpy(st_.smPolicyId, _smPolicyId.c_str());
}

bool DeleteSMFSessionSQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS || sret_ == SQL_SUCCESS_WITH_INFO || sret_ == 100) {
        D_THD_LOG(gLogName, "DeleteSMFSessionSQL Success - SQLExecute");

        SQLLEN  affectedCnt;
        SQLRETURN r = SQLRowCount(_stmt, &affectedCnt);
        D_THD_LOG(gLogName, "DeleteSMFSessionSQL RowCount[%zd]", affectedCnt);
        return true;
    }

    D_THD_LOG(gLogName, "DeleteSMFSessionSQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}
