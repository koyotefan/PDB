
#ifndef PDB_ODBC_ERR_HPP
#define PDB_ODBC_ERR_HPP

#include <string>
#include <sql.h>
#include <sqlext.h>

namespace PDB {

class ODBCErr {
public:
    explicit ODBCErr() = delete;
    ~ODBCErr() = delete;

    static std::string GetStateDBC(SQLHDBC _hdbc);
    static std::string GetStringDBC(SQLHDBC _hdbc);

    static std::string GetStateSTMT(SQLHSTMT _hstmt);
    static std::string GetStringSTMT(SQLHSTMT _hstmt);


};

}


#endif // ODBC_ERR_HPP
