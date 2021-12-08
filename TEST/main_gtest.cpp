#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include <gtest/gtest.h>

#include "PDBConnectionManager.hpp"
#include "PDBWorker.hpp"
#include "SubscriberSQL.hpp"

std::shared_ptr<PDB::ConnectionManager> pcm;

TEST(PDB, turn_on) {

    PDB::Worker     pdbW;
    pdbW.Assign(pcm);

    ASSERT_TRUE(pdbW.TurnOn(PDB::eDefDBType::Subscriber));
}

TEST(PDB, subscriber) {

    PDB::Worker     pdbW;
    pdbW.Assign(pcm);

    ASSERT_TRUE(pdbW.TurnOn(PDB::eDefDBType::Subscriber));

    DeleteSubsProfileSQL        dSubs;
    ASSERT_TRUE(dSubs.Bind());

    dSubs.SetMdn("01028071121");
    pdbW.Execute(dSubs);

    InsertSubsProfileSQL        iSubs;
    ASSERT_TRUE(iSubs.Bind());

    iSubs.SetMdn("01028071121");
    iSubs.SetProductId("Banana");
    EXPECT_TRUE(pdbW.Execute(iSubs));


    SelectSubsProfileSQL        sSubs;
    ASSERT_TRUE(sSubs.Bind());

    sSubs.SetMdn("01028071121");
    EXPECT_TRUE(pdbW.Execute(sSubs));

    stSubscriberProfile     profile;
    sSubs.GetProfile(profile);
    EXPECT_STREQ(profile.productId, "Banana");

    // ASSERT_TRUE(dSubs.Bind());
    dSubs.SetMdn("01028071121");
    EXPECT_TRUE(pdbW.Execute(dSubs));

}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);


#ifndef __T_DEBUG
    NDF_OPEN_SERVICE_LOG("./", "pdbt", 1, 15, 0, 0);
    NDF_INIT_THD_LOG(gLogName);
#endif

    if(argc != 4) {

        E_THD_LOG(gLogName, "invalid argument");
        I_THD_LOG(gLogName, "ex) pdbt [nodeId] [procName] [config filename]");

        return 0;
    }

    pcm = std::make_shared<PDB::ConnectionManager>();

    if(pcm->Init(argv[1], argv[2], argv[3]) == false) {
        E_THD_LOG(gLogName, "ConnectionManager Init() fail");
        return 0;
    }
    pcm->SetLogName("MAIN");

    return RUN_ALL_TESTS();
}

