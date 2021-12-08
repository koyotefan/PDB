#ifndef     PDB_DATA_RECORD_HPP
#define     PDB_DATA_RECORD_HPP

#include <cstring>

#define         LEN_IMSI        32
#define         LEN_MDN         16
#define         LEN_MSISDN      32
#define         LEN_GROUP_ID    32
#define         LEN_MVNO        4
#define         LEN_OCS         1
#define         LEN_UE_OS_VER       4
#define         LEN_UE_MODEL        16
#define         LEN_CATEGORY_LTE    4
#define         LEN_CATEGORY_5G     4
#define         LEN_PRODUCT_ID      32
#define         LEN_BLACK_LIST      1


struct stSubscriberProfile {
    char    imsi[LEN_IMSI+1];
    char    mdn[LEN_MDN+1];
    char    msisdn[LEN_MSISDN+1];
    char    groupId[LEN_GROUP_ID+1];
    char    mvno[LEN_MVNO+1];
    char    ocs[LEN_OCS+1];
    char    ueOsVer[LEN_UE_OS_VER+1];
    char    ueModel[LEN_UE_MODEL+1];
    char    ueCategoryLTE[LEN_CATEGORY_LTE+1];
    char    ueCategory5G[LEN_CATEGORY_5G+1];
    char    productId[LEN_PRODUCT_ID+1];
    char    blackList[LEN_BLACK_LIST+1];
    int     memberCnt;

    stSubscriberProfile() {
        imsi[0]          = '\0';
        mdn[0]           = '\0';
        msisdn[0]        = '\0';
        groupId[0]       = '\0';
        mvno[0]          = '\0';
        ocs[0]           = '\0';
        ueOsVer[0]       = '\0';
        ueModel[0]       = '\0';
        ueCategoryLTE[0] = '\0';
        ueCategory5G[0]  = '\0';
        productId[0]     = '\0';
        blackList[0]     = '\0';
        memberCnt        = 12;
    }
    stSubscriberProfile & operator=(const stSubscriberProfile & _st) {
        if(this != &_st)
            memcpy(this, &_st, sizeof(_st));

        return *this;
    }
};

/*-
enum class eSubscriberProfile : int {
    imsi,
    mdn,
    msisdn,
    groupId,
    mvno,
    ocs,
    ueOsVer,
    ueModel,
    categoryLTE,
    category5G,
    productId,
    blackList,
    cnt
};
-*/


// JUST TEST

#define         LEN_SM_POLICY_ID        128
#define         LEN_SUPI                32
#define         LEN_DNN                 64
#define         LEN_IN_HTTP_HEADER      1024
#define         LEN_IN_HTTP_PAYLOAD     1024
#define         LEN_RES_URI             256
#define         LEN_NOTI_URI            256
#define         LEN_STATUS              1
#define         LEN_NODE_ID             64
#define         LEN_PROC_ID             64
#define         LEN_SMF_ID              64

// NOT NULL 만 모음..
struct stSMFSessionRecord {
    char    smPolicyId[LEN_SM_POLICY_ID+1];
    int     pduSessionId;
    char    supi[LEN_SUPI+1];
    char    dnn[LEN_DNN+1];
    int     s_nssai_sst;
    char    inHttpHeader[LEN_IN_HTTP_HEADER+1];
    char    inHttpPayload[LEN_IN_HTTP_PAYLOAD+1];
    char    resUri[LEN_RES_URI+1];
    char    notiUri[LEN_NOTI_URI+1];
    char    status[LEN_STATUS+1];
    char    nodeId[LEN_NODE_ID+1];
    char    procId[LEN_PROC_ID+1];
    char    smfId[LEN_SMF_ID+1];
    int     connId;
    int     streamId;
    int     memberCnt;

    stSMFSessionRecord() {
        smPolicyId[0]       = '\0';
        pduSessionId        = 0;
        supi[0]             = '\0';
        dnn[0]              = '\0';
        s_nssai_sst         = 0;
        inHttpHeader[0]     = '\0';
        inHttpPayload[0]    = '\0';
        resUri[0]           = '\0';
        notiUri[0]          = '\0';
        nodeId[0]           = '\0';
        procId[0]           = '\0';
        smfId[0]            = '\0';
        connId              = 0;
        streamId            = 0;
        memberCnt           = 14;
    }

    stSMFSessionRecord & operator=(const stSMFSessionRecord & _st) {
        if(this != &_st)
            memcpy(this, &_st, sizeof(_st));

        return *this;
    }
};

#define     LEN_NAME    32

struct stLTESessionTEST {
    char    name[LEN_NAME+1];
    int     memberCnt;

    stLTESessionTEST() {
        name[0] = '\0';
        memberCnt = 1;
    }

    stLTESessionTEST & operator=(const stLTESessionTEST & _st) {
        if(this != &_st)
            memcpy(this, &_st, sizeof(_st));

        return *this;
    }

};

#define     LEN_VAL     16

struct stPGLocal {
    char    name[LEN_NAME+1];
    char    val[LEN_VAL+1];
    int     memberCnt;

    stPGLocal() {
        name[0] = '\0';
        val[0] ='\0';
        memberCnt = 2;
    }

    stPGLocal & operator=(const stPGLocal & _st) {
        if(this != &_st)
            memcpy(this, &_st, sizeof(_st));

        return *this;
    }

};

#define     LEN_DATA    32
#define     LEN_TEST    32

struct stBusy {
    char    name[LEN_NAME+1];
    char    data[LEN_DATA+1];
    char    test[LEN_TEST+1]; 
    int     memberCnt;

    stBusy() {
        name[0] = '\0';
        data[0] ='\0';
        test[0] ='\0';
        memberCnt = 3;
    }

    stBusy & operator=(const stBusy & _st) {
        if(this != &_st)
            memcpy(this, &_st, sizeof(_st));

        return *this;
    }
};

 

#endif //   PDB_DATA_RECORD_HPP
