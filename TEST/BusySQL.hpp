#ifndef BUSY_SQL_HPP
#define BUSY_SQL_HPP

#include "PDBPreparedSQL.hpp"
#include "PDBDataRecord.hpp"


class InsertBusySQL: public PDB::PreparedSQL {
public:
    explicit InsertBusySQL();
    ~InsertBusySQL();

    bool Bind();
    void SetNAME(const char * _val);

    bool Execute(SQLHSTMT & _stmt);

private:
    stBusy            st_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class DeleteBusySQL : public PDB::PreparedSQL {
public:
    explicit DeleteBusySQL();
    ~DeleteBusySQL();

    bool Bind();
    void SetNAME(const char * _mdn);

    bool Execute(SQLHSTMT & _hstmt);

private:
    stBusy            st_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class SelectBusySQL : public PDB::PreparedSQL {
public:
    explicit SelectBusySQL();
    ~SelectBusySQL();

    bool Bind();
    void SetNAME(const char * _mdn);
    void FetchToggle();

    bool Execute(SQLHSTMT & _hstmt);

private:
    stBusy            st_;
    bool              bToggle_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;

    SQLLEN *          arrInd_;
    STBindColumn *    arrBc_;

};

#endif // BUSY_SQL_HPP
