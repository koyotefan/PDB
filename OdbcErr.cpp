
#include "PDBDefine.hpp"
#include "PDBOdbcErr.hpp"

namespace PDB {

std::string ODBCErr::GetStateDBC(SQLHDBC _hdbc) {
    char    state[7] = {};
    SQLINTEGER  native = 0;
    char    text[256] = {};
    SQLSMALLINT len = 0;
    int         seq =0;

    SQLRETURN ret = SQLGetDiagRec(SQL_HANDLE_DBC,
                                  _hdbc,
                                  seq,
                                  (SQLCHAR *)state,
                                  &native,
                                  (SQLCHAR *)text,
                                  sizeof(text),
                                  &len);
    if(SQL_SUCCEEDED(ret))
      return std::string(state);

    return std::string();
}

std::string ODBCErr::GetStringDBC(SQLHDBC _hdbc) {

    char    state[7] = {};
    SQLINTEGER  native = 0;
    char    text[256] = {};
    SQLSMALLINT len =0;
    int         seq =0;

    SQLRETURN ret = SQLGetDiagRec(SQL_HANDLE_DBC,
                                  _hdbc,
                                  seq,
                                  (SQLCHAR *)state,
                                  &native,
                                  (SQLCHAR *)text,
                                  sizeof(text),
                                  &len);
    if(SQL_SUCCEEDED(ret)) {
        char    cbuf[512];
        snprintf(cbuf, sizeof(cbuf), "%s:%d:%d:%s", state, seq, native, text);
        return std::string(cbuf);
    }

    return std::string();
}


std::string ODBCErr::GetStateSTMT(SQLHSTMT _hstmt) {

    SQLLEN  numRecs = 0;
    SQLGetDiagField(SQL_HANDLE_STMT, _hstmt, 0, SQL_DIAG_NUMBER, &numRecs, 0, 0);

    char    state[7] = {};
    SQLINTEGER  native = 0;
    char    text[256] = {};
    SQLSMALLINT len = 0;

    if(SQLGetDiagRec(SQL_HANDLE_STMT,
                     _hstmt,
                     numRecs,
                     (SQLCHAR *)state,
                     &native,
                     (SQLCHAR *)text,
                     sizeof(text),
                     &len) != SQL_NO_DATA) {

        return std::string(state);
    }

    /*-
    SQLRETURN   sret;
    char    state[7];
    SQLINTEGER  native = 0;
    char    text[256];
    SQLSMALLINT len;
    int         seq =0;

    while(seq <= numRecs &&
          (sret = SQLGetDiagRec(SQL_HANDLE_STMT,
                                _hstmt,
                                seq,
                                (SQLCHAR *)state,
                                &native,
                                (SQLCHAR *)text,
                                sizeof(text),
                                &len))) {

      if(SQL_SUCCEEDED(sret)) {
        return std::string(state);
      }

      ++seq;
    }
    -*/

    return std::string();
}

std::string ODBCErr::GetStringSTMT(SQLHSTMT _hstmt) {

    SQLLEN  numRecs = 0;
    SQLGetDiagField(SQL_HANDLE_STMT, _hstmt, 0, SQL_DIAG_NUMBER, &numRecs, 0, 0);

    char    state[7] = {};
    SQLINTEGER  native = 0;
    char    text[256] = {};
    SQLSMALLINT len = 0;

    std::string   sbuf;

    if(SQLGetDiagRec(SQL_HANDLE_STMT,
                     _hstmt,
                     numRecs,
                     (SQLCHAR *)state,
                     &native,
                     (SQLCHAR *)text,
                     sizeof(text),
                     &len) != SQL_NO_DATA) {
        char    cbuf[512];
        snprintf(cbuf, sizeof(cbuf), "%s:%lu:%d:%s", state, numRecs, native, text);
        sbuf = cbuf;
    }

    /*-
    while(seq <= numRecs &&
          (sret = SQLGetDiagRec(SQL_HANDLE_STMT,
                                _hstmt,
                                seq,
                                (SQLCHAR *)state,
                                &native,
                                (SQLCHAR *)text,
                                sizeof(text),
                                &len)) != SQL_NO_DATA) {

        snprintf(cbuf, sizeof(cbuf), "%s:%d:%d:%s", state, seq, native, text);
        D_THD_LOG(gLogName, "########## GetStringSTMT [%s]", cbuf);
        sbuf.append(cbuf);
        sbuf.append("\n");

        ++seq;

    }
    -*/

    return sbuf;
}





}