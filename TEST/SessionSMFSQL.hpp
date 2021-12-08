#ifndef SESSION_SMFSQL_HPP
#define SESSION_SMFSQL_HPP

#include "PDBPreparedSQL.hpp"
#include "PDBDataRecord.hpp"

class InsertSMFSessionSQL : public PDB::PreparedSQL {
public:
    explicit InsertSMFSessionSQL();
    ~InsertSMFSessionSQL();

    bool Bind();
    void SetValue(int _n, const char * _val);

    bool Execute(SQLHSTMT & _stmt);

private:
    stSMFSessionRecord  st_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class DeleteSMFSessionSQL : public PDB::PreparedSQL {
public:
    explicit DeleteSMFSessionSQL();
    ~DeleteSMFSessionSQL();

    bool Bind();
    void SetValue(std::string _smPolicyId);
    bool Execute(SQLHSTMT & _stmt);

private:
    stSMFSessionRecord  st_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

#endif // SESSION_SMFSQL_HPP
