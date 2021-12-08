
#include "SubscriberSQL.hpp"
#include "BusySQL.hpp"
#include "PDBOdbcErr.hpp"



// InsertBusySQL ///////////////////////////////////////////////

InsertBusySQL::InsertBusySQL() {
    arrLen_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[st_.memberCnt];
}

InsertBusySQL::~InsertBusySQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool InsertBusySQL::Bind() {


    char buf[512];

    sprintf(buf,
        "INSERT INTO T_TEST_PDBLIB ("
        "NAME, "
        "DATA, "
		"TEST) "
        "VALUES (?, ?, ?)");

    if(SetSQL(PDB::eDefDBType::PCF, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 3;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop) {
        arrLen_[nLoop] = SQL_NTS;
    }

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, st_.name, sizeof(st_.name), &arrLen_[0] };
    arrBp_[1] = {2, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, st_.data, sizeof(st_.data), &arrLen_[1] };
    arrBp_[2] = {3, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, st_.test, sizeof(st_.test), &arrLen_[2] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void InsertBusySQL::SetNAME(const char * _val) {
	Subscriber::SetValue(st_.name, sizeof(st_.name), _val);
	Subscriber::SetValue(st_.data, sizeof(st_.data), _val);
	Subscriber::SetValue(st_.test, sizeof(st_.test), _val);
}


bool InsertBusySQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS ) {
        // D_THD_LOG(gLogName, "InsertBusySQL Success - SQLExecute");
        return true;
    }

	if(sret_ == SQL_SUCCESS_WITH_INFO) {
        E_THD_LOG(gLogName, 
			"InsertBusySQL Success With INFO - SQLExecute [%d] [%s]",
			sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
		return false;	
	}

    E_THD_LOG(gLogName, "InsertBusySQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}


// DeleteBusySQL ///////////////////////////////////////////////

DeleteBusySQL::DeleteBusySQL() {
    arrLen_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[st_.memberCnt];
}

DeleteBusySQL::~DeleteBusySQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBp_) {
        delete [] arrBp_;
        arrBp_ = nullptr;
    }
}

bool DeleteBusySQL::Bind() {


    char buf[128];

    sprintf(buf,
        "DELETE FROM T_TEST_PDBLIB WHERE NAME=?");

    if(SetSQL(PDB::eDefDBType::PCF, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;
    for(int nLoop=0; nLoop < parameterCnt; ++nLoop) {
        arrLen_[nLoop] = SQL_NTS;
    }

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, st_.name, sizeof(st_.name), &arrLen_[0] };

    SetBindParameter(arrBp_, parameterCnt);

    return true;

}

void DeleteBusySQL::SetNAME(const char * _val) {
	Subscriber::SetValue(st_.name, sizeof(st_.name), _val);
}


bool DeleteBusySQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ == SQL_SUCCESS ) {
        // D_THD_LOG(gLogName, "DeleteBusySQL Success - SQLExecute");
        return true;
    }

	if(sret_ == SQL_SUCCESS_WITH_INFO) {
        E_THD_LOG(gLogName, 
			"DeleteBusySQL Success With INFO - SQLExecute [%d] [%s]",
			sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
		return false;
	}

    E_THD_LOG(gLogName, "DeleteBusySQL fail [%d] [%s]",
        sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
    return false;
}



// SelectBusySQL ///////////////////////////////////////////////

SelectBusySQL::SelectBusySQL() {
    arrInd_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBp_  = new (std::nothrow) STBindParameter[st_.memberCnt];

    arrLen_ = new (std::nothrow) SQLLEN[st_.memberCnt];
    arrBc_  = new (std::nothrow) STBindColumn[st_.memberCnt];

    bToggle_ = true;
}

SelectBusySQL::~SelectBusySQL() {

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

bool SelectBusySQL::Bind() {

    char buf[256];
    sprintf(buf,
        "SELECT "
            "NAME,"
            "DATA,"
            "TEST "
        " FROM T_TEST_PDBLIB WHERE "
        " NAME=?");

    if(SetSQL(PDB::eDefDBType::PCF, buf, strlen(buf)) == false) {
        E_THD_LOG(gLogName, "SetSQL() fail [%s]", buf);
        return false;
    }

    int parameterCnt = 1;
    for(int nLoop=0; nLoop < st_.memberCnt; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBp_[0] = {1, SQL_C_CHAR, SQL_CHAR, LEN_NAME, 0, st_.name, sizeof(st_.name), &arrLen_[0] };

    SetBindParameter(arrBp_, parameterCnt);

    arrBc_[0] = {1, SQL_C_CHAR, st_.name, sizeof(st_.name)-1, &arrInd_[0] };
    arrBc_[1] = {2, SQL_C_CHAR, st_.data, sizeof(st_.data)-1, &arrInd_[1] };
    arrBc_[2] = {3, SQL_C_CHAR, st_.test, sizeof(st_.test)-1, &arrInd_[2] };

    SetBindColumn(arrBc_, 3);

    return true;

}


void SelectBusySQL::SetNAME(const char * _val) {
    Subscriber::SetValue(st_.name, sizeof(st_.name), _val);
}

void SelectBusySQL::FetchToggle() {
	bToggle_ = !bToggle_;
	I_THD_LOG(gLogName, "Fetch [%s]", (bToggle_)?"True":"False");
}

bool SelectBusySQL::Execute(SQLHSTMT & _stmt) {

    sret_ = SQLExecute(_stmt);
    if(sret_ != SQL_SUCCESS ) {
        E_THD_LOG(gLogName, "SelectBusySQL fail- SQLExecute [%d] [%s]",
			sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
        return false;
    }

	int nRet = 0;


    while(bToggle_) {
        sret_ = SQLFetch(_stmt);

        if(sret_ == SQL_NO_DATA)
            break;

        if(sret_ != SQL_SUCCESS) {

            E_THD_LOG(gLogName, "SelectSubsProfileSQL SQLFetch() fail [%d] [%s]",
                sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
            return false;
        }

		++nRet;
    }

	if(bToggle_ && nRet == 0) {
        E_THD_LOG(gLogName, "SelectSubsProfileSQL fail- SQLExecute NO DATA!");
        return false;
	}
    
    // D_THD_LOG(gLogName, "SelectSubsProfileSQL success [%d]", nRet);
    return true;
}
