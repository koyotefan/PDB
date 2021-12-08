#ifndef PDB_PREPARED_SQL_HPP
#define PDB_PREPARED_SQL_HPP

#include <string>
#include <vector>
#include <functional>

#include <sql.h>
#include <sqlext.h>

#include "PDBDefine.hpp"
#include "PDBConnection.hpp"
#include "CNDFODBCAdaptor.hpp"

namespace PDB {

struct stParameterInfo {
    STBindParameter * ptr;
    size_t            cnt;

    stParameterInfo()
        : ptr(nullptr),
          cnt(0) {
    }
};

struct stColumnInfo {
    STBindColumn *  ptr;
    size_t          cnt;

    stColumnInfo()
        : ptr(nullptr),
          cnt(0) {
    }
};

class PreparedSQL {
public:
    explicit PreparedSQL();
    virtual ~PreparedSQL();

    bool        SetSQL(eDefDBType _pdbT, const char * _sql, size_t _sqlLength);
    const char * GetSQL();
    size_t      GetSQLSize();

    eDefDBType  GetDBType() { return myT_; }

    void        SetBindParameter(STBindParameter * _arrP, size_t _cnt);
    void        SetBindColumn(STBindColumn * _arrP, size_t _cnt);

    // Lib 에서 DirectExecute() 를 할때만 사용합니다.
    void        SetFunc(std::function<bool(SQLHSTMT & _stmt)> _func);

    void        PreparedOff() { bPreparedUsed_ = false; }  
    void        PreparedOn()  { bPreparedUsed_ = true; }  


    SQLHSTMT &  AllocStatement(Connection * _c, size_t _shardingNum);
    void        CloseStatement(SQLHSTMT & stmt);
    // void        DropStatement(SQLHSTMT & stmt);

    bool        BindParameter(SQLHSTMT & _stmt,
                              STBindParameter * _arrP,
                              size_t _cnt);
    bool        BindColumn(SQLHSTMT & _stmt,
                            STBindColumn * _arrP,
                            size_t _cnt);

    SQLRETURN   GetSQLReturn() { return sret_; }
    const char * GetClassName() { return className_.c_str(); }

    virtual bool Execute(SQLHSTMT & _hstmt);

    void        AllPrint();

protected:
    std::string     className_;
    SQLRETURN       sret_;

private:
    void        clear();
    bool        bindParameter(SQLHSTMT & _stmt);
    bool        bindColumn(SQLHSTMT & _stmt);

private:

    eDefDBType          myT_;
    std::string         sql_;

    stParameterInfo     paramInfo_;
    stColumnInfo        colInfo_;

    bool                bPreparedUsed_ = true;

    std::vector<stStmtInfo>         vecStmtInfo_;

    std::function<bool(SQLHSTMT & _stmt)> func_;

};

}

#endif // PDB_PREPARED_SQL_HPP
