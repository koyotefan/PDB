#ifndef LTE_SESSION_TEST_SQL_HPP
#define LTE_SESSION_TEST_SQL_HPP

#include "PDBPreparedSQL.hpp"
#include "PDBDataRecord.hpp"

class InsertLTESessionSQL : public PDB::PreparedSQL {
public:
    explicit InsertLTESessionSQL();
    ~InsertLTESessionSQL();

    bool Bind(std::string _tblName);
    void SetValue(const char * _val);

    bool Execute(SQLHSTMT & _stmt);

private:
    stLTESessionTEST  st_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class DeleteLTESessionSQL : public PDB::PreparedSQL {
public:
    explicit DeleteLTESessionSQL();
    ~DeleteLTESessionSQL();

    bool Bind(std::string _tblName);
    void SetValue(std::string _smPolicyId);
    bool Execute(SQLHSTMT & _stmt);

private:
    stLTESessionTEST st_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};


#endif // LTE_SESSION_TEST_SQL_HPP
