#ifndef PGLOCAL_SQL_HPP
#define PGLOCAL_SQL_HPP

#include "PDBPreparedSQL.hpp"
#include "PDBDataRecord.hpp"

class InsertPGLocalSQL: public PDB::PreparedSQL {
public:
    explicit InsertPGLocalSQL(PDB::eDefDBType _dbType);
    ~InsertPGLocalSQL();

    bool Bind();
    void SetMdn(const char * _val);
    void SetVal(const char * _val);

    bool Execute(SQLHSTMT & _stmt);

private:
    PDB::eDefDBType   dbType_; 
    stPGLocal         profile_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class DeletePGLocalSQL : public PDB::PreparedSQL {
public:
    explicit DeletePGLocalSQL(PDB::eDefDBType _dbType);
    ~DeletePGLocalSQL();

    bool Bind();
    void SetMdn(const char * _mdn);

    bool Execute(SQLHSTMT & _hstmt);

private:
    PDB::eDefDBType   dbType_; 
    stPGLocal         profile_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};


#endif // PGLOCAL_SQL_HPP
