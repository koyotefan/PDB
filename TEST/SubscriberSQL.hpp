#ifndef SUBSCRIBER_SQL_HPP
#define SUBSCRIBER_SQL_HPP

#include "PDBPreparedSQL.hpp"
#include "PDBDataRecord.hpp"

class Subscriber {
public:
    static void SetValue(char * _target, size_t _targetSize, const char * _val);

private:
    explicit Subscriber() = delete;
    ~Subscriber() = delete;
};


class InsertSubsProfileSQL : public PDB::PreparedSQL {
public:
    explicit InsertSubsProfileSQL();
    ~InsertSubsProfileSQL();

    bool Bind();
    void SetMdn(const char * _val);
    void SetProductId(const char * _val);

    bool Execute(SQLHSTMT & _stmt);

private:
    stSubscriberProfile  profile_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};

class SelectSubsProfileSQL : public PDB::PreparedSQL {
public:
    explicit SelectSubsProfileSQL();
    ~SelectSubsProfileSQL();

    bool Bind();
    void SetMdn(const char * _val);
    void GetProfile(stSubscriberProfile & _profile) { _profile = profile_; }

    bool Execute(SQLHSTMT & _hstmt);

private:
    stSubscriberProfile         profile_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;

    SQLLEN *          arrInd_;
    STBindColumn *    arrBc_;
};

class DeleteSubsProfileSQL : public PDB::PreparedSQL {
public:
    explicit DeleteSubsProfileSQL();
    ~DeleteSubsProfileSQL();

    bool Bind();
    void SetMdn(const char * _mdn);

    bool Execute(SQLHSTMT & _hstmt);

private:
    stSubscriberProfile         profile_;

    SQLLEN *          arrLen_;
    STBindParameter * arrBp_;
};


#endif // SUBSCRIBER_SQL_HPP
