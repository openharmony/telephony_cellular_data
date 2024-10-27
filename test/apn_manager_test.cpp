/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define private public
#define protected public

#include "apn_holder.h"
#include "apn_manager.h"
#include "cellular_data_state_machine.h"
#include "cellular_data_client.h"
#include "data_connection_manager.h"
#include "gtest/gtest.h"
#include "tel_event_handler.h"
#include "pdp_profile_data.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class ApnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<ApnManager> apnManager;
};
void ApnManagerTest::SetUpTestCase() {}

void ApnManagerTest::TearDownTestCase() {}

void ApnManagerTest::SetUp()
{
    apnManager = std::make_shared<ApnManager>();
}

void ApnManagerTest::TearDown()
{
    apnManager.reset();
}

class StateMachineTest : public TelEventHandler {
public:
    StateMachineTest() : TelEventHandler("StateMachineTest") {}
    ~StateMachineTest() = default;
    std::shared_ptr<CellularDataStateMachine> CreateCellularDataStateMachine(int32_t slotId);

public:
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine_ = nullptr;
};

std::shared_ptr<CellularDataStateMachine> StateMachineTest::CreateCellularDataStateMachine(int32_t slotId)
{
    if (cellularDataStateMachine_ != nullptr) {
        return cellularDataStateMachine_;
    }
    sptr<DataConnectionManager> connectionManager = std::make_unique<DataConnectionManager>(slotId).release();
    if (connectionManager == nullptr) {
        return nullptr;
    }
    connectionManager->Init();
    cellularDataStateMachine_ = std::make_shared<CellularDataStateMachine>(
        connectionManager, std::static_pointer_cast<TelEventHandler>(shared_from_this()));
    return cellularDataStateMachine_;
}

/**
 * @tc.number   FindApnNameByApnId_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnNameByApnId_001, Function | MediumTest | Level1)
{
    int32_t id = 1;
    std::string result = apnManager->FindApnNameByApnId(id);
    ASSERT_EQ(result, DATA_CONTEXT_ROLE_DEFAULT);
}

/**
 * @tc.number   FindApnNameByApnId_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnNameByApnId_002, Function | MediumTest | Level1)
{
    int32_t id = 2;
    std::string result = apnManager->FindApnNameByApnId(id);
    ASSERT_EQ(result, DATA_CONTEXT_ROLE_MMS);
}

/**
 * @tc.number   FindApnIdByCapability_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_001, Function | MediumTest | Level1)
{
    uint64_t capability = NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET;
    int32_t expected = DATA_CONTEXT_ROLE_DEFAULT_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindApnIdByCapability_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_002, Function | MediumTest | Level1)
{
    uint64_t capability = NetManagerStandard::NetCap::NET_CAPABILITY_MMS;
    int32_t expected = DATA_CONTEXT_ROLE_MMS_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindApnIdByCapability_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_003, Function | MediumTest | Level1)
{
    uint64_t capability = NetManagerStandard::NetCap::NET_CAPABILITY_INTERNAL_DEFAULT;
    int32_t expected = DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindApnIdByCapability_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_004, Function | MediumTest | Level1)
{
    uint64_t capability = NetManagerStandard::NetCap::NET_CAPABILITY_IA;
    int32_t expected = DATA_CONTEXT_ROLE_IA_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindApnIdByCapability_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_005, Function | MediumTest | Level1)
{
    uint64_t capability = NetManagerStandard::NetCap::NET_CAPABILITY_XCAP;
    int32_t expected = DATA_CONTEXT_ROLE_XCAP_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindApnIdByCapability_006
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_006, Function | MediumTest | Level1)
{
    uint64_t capability = NetManagerStandard::NetCap::NET_CAPABILITY_SUPL;
    int32_t expected = DATA_CONTEXT_ROLE_SUPL_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindApnIdByCapability_007
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_007, Function | MediumTest | Level1)
{
    uint64_t capability = NetManagerStandard::NetCap::NET_CAPABILITY_DUN;
    int32_t expected = DATA_CONTEXT_ROLE_DUN_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindApnIdByCapability_008
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnIdByCapability_008, Function | MediumTest | Level1)
{
    uint64_t capability = 100;
    int32_t expected = DATA_CONTEXT_ROLE_INVALID_ID;
    int32_t actual = apnManager->FindApnIdByCapability(capability);
    ASSERT_EQ(actual, expected);
}

/**
 * @tc.number   FindBestCapability_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindBestCapability_001, Function | MediumTest | Level1)
{
    uint64_t capabilities = 1L << NetManagerStandard::NetCap::NET_CAPABILITY_SUPL;
    NetManagerStandard::NetCap netCap = apnManager->FindBestCapability(capabilities);
    ASSERT_EQ(netCap, NetManagerStandard::NetCap::NET_CAPABILITY_SUPL);
}

/**
 * @tc.number   FindBestCapability_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindBestCapability_002, Function | MediumTest | Level1)
{
    uint64_t capabilities = 1L << NetManagerStandard::NetCap::NET_CAPABILITY_DUN;
    NetManagerStandard::NetCap netCap = apnManager->FindBestCapability(capabilities);
    ASSERT_EQ(netCap, NetManagerStandard::NetCap::NET_CAPABILITY_DUN);
}

/**
 * @tc.number   FindBestCapability_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindBestCapability_003, Function | MediumTest | Level1)
{
    uint64_t capabilities = 1L << NetManagerStandard::NetCap::NET_CAPABILITY_XCAP;
    NetManagerStandard::NetCap netCap = apnManager->FindBestCapability(capabilities);
    ASSERT_EQ(netCap, NetManagerStandard::NetCap::NET_CAPABILITY_XCAP);
}

/**
 * @tc.number   FindBestCapability_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindBestCapability_004, Function | MediumTest | Level1)
{
    uint64_t capabilities = 1L << NetManagerStandard::NetCap::NET_CAPABILITY_IA;
    NetManagerStandard::NetCap netCap = apnManager->FindBestCapability(capabilities);
    ASSERT_EQ(netCap, NetManagerStandard::NetCap::NET_CAPABILITY_IA);
}

/**
 * @tc.number   CreateMvnoApnItems_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, CreateMvnoApnItems_001, Function | MediumTest | Level1)
{
    int32_t slotId = 0;
    std::string mcc = "460";
    std::string mnc = "00";
    int32_t result = apnManager->CreateMvnoApnItems(slotId, mcc, mnc);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.number   IsPreferredApnUserEdited_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsPreferredApnUserEdited_001, Function | MediumTest | Level1)
{
    auto preferId = 1;
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->attr_.profileId_ = preferId;
    defaultApnItem->attr_.isEdited_ = true;
    apnManager->allApnItem_.push_back(defaultApnItem);
    apnManager->preferId_ = preferId;
    ASSERT_TRUE(apnManager->IsPreferredApnUserEdited());
}

/**
 * @tc.number   IsPreferredApnUserEdited_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsPreferredApnUserEdited_002, Function | MediumTest | Level1)
{
    auto preferId = 1;
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->attr_.profileId_ = preferId;
    defaultApnItem->attr_.isEdited_ = false;
    apnManager->allApnItem_.push_back(defaultApnItem);
    apnManager->preferId_ = preferId;
    ASSERT_FALSE(apnManager->IsPreferredApnUserEdited());
}

/**
 * @tc.number   IsPreferredApnUserEdited_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsPreferredApnUserEdited_003, Function | MediumTest | Level1)
{
    auto preferId = 2;
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->attr_.profileId_ = 3;
    defaultApnItem->attr_.isEdited_ = true;
    apnManager->allApnItem_.push_back(defaultApnItem);
    apnManager->preferId_ = preferId;
    ASSERT_FALSE(apnManager->IsPreferredApnUserEdited());
}

/**
 * @tc.number   IsDataConnectionNotUsed_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsDataConnectionNotUsed_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularDataStateMachine> stateMachine = nullptr;
    bool result = apnManager->IsDataConnectionNotUsed(stateMachine);
    ASSERT_FALSE(result);
}

/**
 * @tc.number   IsDataConnectionNotUsed_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsDataConnectionNotUsed_002, Function | MediumTest | Level1)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    auto stateMachine = machine->CreateCellularDataStateMachine(0);
    apnManager->apnHolders_.push_back(nullptr);
    bool result = apnManager->IsDataConnectionNotUsed(stateMachine);
    ASSERT_TRUE(result);
}

/**
 * @tc.number   IsDataConnectionNotUsed_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsDataConnectionNotUsed_003, Function | MediumTest | Level1)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    auto stateMachine = machine->CreateCellularDataStateMachine(0);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>(DATA_CONTEXT_ROLE_DEFAULT,
        static_cast<int32_t>(DataContextPriority::PRIORITY_LOW)).release();
    apnHolder->SetCellularDataStateMachine(stateMachine);
    apnManager->apnHolders_.push_back(apnHolder);
    bool result = apnManager->IsDataConnectionNotUsed(stateMachine);
    ASSERT_FALSE(result);
}

/**
 * @tc.number   IsDataConnectionNotUsed_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsDataConnectionNotUsed_004, Function | MediumTest | Level1)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    auto stateMachine = machine->CreateCellularDataStateMachine(0);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>(DATA_CONTEXT_ROLE_DEFAULT,
        static_cast<int32_t>(DataContextPriority::PRIORITY_LOW)).release();
    apnHolder->SetCellularDataStateMachine(nullptr);
    apnManager->apnHolders_.push_back(apnHolder);
    bool result = apnManager->IsDataConnectionNotUsed(stateMachine);
    ASSERT_TRUE(result);
}

/**
 * @tc.number   IsDataConnectionNotUsed_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsDataConnectionNotUsed_005, Function | MediumTest | Level1)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    auto stateMachine = machine->CreateCellularDataStateMachine(0);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>(DATA_CONTEXT_ROLE_DEFAULT,
        static_cast<int32_t>(DataContextPriority::PRIORITY_LOW)).release();
    apnHolder->SetCellularDataStateMachine(stateMachine);
    apnManager->apnHolders_.push_back(apnHolder);
    machine->cellularDataStateMachine_ = nullptr;
    auto stateMachine_1 = machine->CreateCellularDataStateMachine(0);
    bool result = apnManager->IsDataConnectionNotUsed(stateMachine_1);
    ASSERT_TRUE(result);
}

/**
 * @tc.number   MakeSpecificApnItem_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, MakeSpecificApnItem_001, Function | MediumTest | Level1)
{
    auto preferId = 1;
    apnManager->preferId_ = preferId;
    PdpProfile pdpProfile;
    pdpProfile.profileId = preferId;
    pdpProfile.apnTypes = "";
    std::vector<PdpProfile> apnVec;
    apnVec.push_back(pdpProfile);
    bool result = apnManager->MakeSpecificApnItem(apnVec);
    ASSERT_EQ(result, 1);
}

/**
 * @tc.number   MakeSpecificApnItem_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, MakeSpecificApnItem_002, Function | MediumTest | Level1)
{
    auto preferId = 1;
    apnManager->preferId_ = preferId;
    PdpProfile pdpProfile;
    pdpProfile.profileId = preferId;
    pdpProfile.apnTypes = "default";
    std::vector<PdpProfile> apnVec;
    apnVec.push_back(pdpProfile);
    bool result = apnManager->MakeSpecificApnItem(apnVec);
    ASSERT_EQ(result, 1);
}

/**
 * @tc.number   MakeSpecificApnItem_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, MakeSpecificApnItem_003, Function | MediumTest | Level1)
{
    auto preferId = 1;
    apnManager->preferId_ = 2;
    PdpProfile pdpProfile;
    pdpProfile.profileId = preferId;
    pdpProfile.apnTypes = "default";
    std::vector<PdpProfile> apnVec;
    apnVec.push_back(pdpProfile);
    bool result = apnManager->MakeSpecificApnItem(apnVec);
    ASSERT_EQ(result, 1);
}

/**
 * @tc.number   HasAnyConnectedState_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, HasAnyConnectedState_001, Function | MediumTest | Level1)
{
    std::vector<sptr<ApnHolder>> apnHolders;
    apnManager->apnHolders_ = apnHolders;
    bool result = apnManager->HasAnyConnectedState();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   HasAnyConnectedState_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, HasAnyConnectedState_002, Function | MediumTest | Level1)
{
    std::vector<sptr<ApnHolder>> apnHolders;
    sptr<ApnHolder> apnHolder = nullptr;
    apnHolders.push_back(apnHolder);
    apnManager->apnHolders_ = apnHolders;
    bool result = apnManager->HasAnyConnectedState();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   HasAnyConnectedState_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, HasAnyConnectedState_003, Function | MediumTest | Level1)
{
    std::vector<sptr<ApnHolder>> apnHolders;
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_CONNECTED);
    apnHolders.push_back(apnHolder);
    apnManager->apnHolders_ = apnHolders;
    bool result = apnManager->HasAnyConnectedState();
    ASSERT_EQ(result, true);
}

/**
 * @tc.number   HasAnyConnectedState_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, HasAnyConnectedState_004, Function | MediumTest | Level1)
{
    std::vector<sptr<ApnHolder>> apnHolders;
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_DISCONNECTING);
    apnHolders.push_back(apnHolder);
    apnManager->apnHolders_ = apnHolders;
    bool result = apnManager->HasAnyConnectedState();
    ASSERT_EQ(result, true);
}

/**
 * @tc.number   GetRilAttachApn_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetRilAttachApn_001, Function | MediumTest | Level1)
{
    std::vector<sptr<ApnItem>> allApnItem;
    apnManager->allApnItem_ = allApnItem;
    ASSERT_EQ(apnManager->GetRilAttachApn(), nullptr);
}

/**
 * @tc.number   ReleaseDataConnection_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, ReleaseDataConnection_001, TestSize.Level0)
{
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->cellularDataStateMachine_ = nullptr;
    apnHolder->ReleaseDataConnection();
    ASSERT_EQ(apnHolder->cellularDataStateMachine_, nullptr);
}

/**
 * @tc.number   ReleaseDataConnection_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, ReleaseDataConnection_002, TestSize.Level0)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    auto stateMachine = machine->CreateCellularDataStateMachine(0);
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->SetCellularDataStateMachine(stateMachine);
    apnHolder->ReleaseDataConnection();
    ASSERT_EQ(apnHolder->cellularDataStateMachine_, nullptr);
}

/**
 * @tc.number   RequestCellularData_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, RequestCellularData_001, TestSize.Level0)
{
    NetRequest netRequest;
    netRequest.capability = 0;
    netRequest.ident = "ident";
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->RequestCellularData(netRequest);
    ASSERT_EQ(apnHolder->dataCallEnabled_, true);
}

/**
 * @tc.number   RequestCellularData_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, RequestCellularData_002, TestSize.Level0)
{
    NetRequest netRequest;
    netRequest.capability = 0;
    netRequest.ident = "ident";
    NetRequest netRequest1;
    netRequest1.capability = 0;
    netRequest1.ident = "abc";
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->netRequests_.push_back(netRequest1);
    apnHolder->RequestCellularData(netRequest);
    ASSERT_EQ(apnHolder->dataCallEnabled_, true);
}

/**
 * @tc.number   RequestCellularData_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, RequestCellularData_003, TestSize.Level0)
{
    NetRequest netRequest;
    netRequest.capability = 0;
    netRequest.ident = "ident";
    NetRequest netRequest1;
    netRequest1.capability = 1;
    netRequest1.ident = "ident";
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->netRequests_.push_back(netRequest1);
    apnHolder->RequestCellularData(netRequest);
    ASSERT_EQ(apnHolder->dataCallEnabled_, true);
}

/**
 * @tc.number   RequestCellularData_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, RequestCellularData_004, TestSize.Level0)
{
    NetRequest netRequest;
    netRequest.capability = 1;
    netRequest.ident = "ident";
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->netRequests_.push_back(netRequest);
    int size = apnHolder->netRequests_.size();
    apnHolder->RequestCellularData(netRequest);
    ASSERT_EQ(size, apnHolder->netRequests_.size());
}

/**
 * @tc.number   IsCompatibleApnItem_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsCompatibleApnItem_001, TestSize.Level0)
{
    sptr<ApnItem> newApnItem = nullptr;
    sptr<ApnItem> oldApnItem = nullptr;
    bool roamingState = false;
    bool result = ApnHolder::IsCompatibleApnItem(newApnItem, oldApnItem, roamingState);
    ASSERT_FALSE(result);
}

/**
 * @tc.number   IsCompatibleApnItem_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsCompatibleApnItem_002, TestSize.Level0)
{
    sptr<ApnItem> newApnItem = new ApnItem();
    sptr<ApnItem> oldApnItem = new ApnItem();
    std::strcmp(newApnItem->attr_.roamingProtocol_, "test_protocol");
    std::strcmp(newApnItem->attr_.roamingProtocol_, "test_protocol");
    bool roamingState = true;
    bool result = ApnHolder::IsCompatibleApnItem(newApnItem, oldApnItem, roamingState);
    ASSERT_TRUE(result);
}

/**
 * @tc.number   IsCompatibleApnItem_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsCompatibleApnItem_003, TestSize.Level0)
{
    sptr<ApnItem> newApnItem = new ApnItem();
    sptr<ApnItem> oldApnItem = new ApnItem();
    std::strcmp(newApnItem->attr_.roamingProtocol_, "test_protocol");
    std::strcmp(newApnItem->attr_.roamingProtocol_, "test_protocol");
    bool roamingState = false;
    bool result = ApnHolder::IsCompatibleApnItem(newApnItem, oldApnItem, roamingState);
    ASSERT_TRUE(result);
}

/**
 * @tc.number   GetNextRetryApnItem001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryApnItem001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->matchedApns_.clear();
    EXPECT_EQ(connectionRetryPolicy->GetNextRetryApnItem(), nullptr);

    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->MarkBadApn(true);
    connectionRetryPolicy->matchedApns_.push_back(defaultApnItem);
    connectionRetryPolicy->currentApnIndex_ = 0;
    EXPECT_EQ(connectionRetryPolicy->GetNextRetryApnItem(), nullptr);
    defaultApnItem->MarkBadApn(false);
    EXPECT_NE(connectionRetryPolicy->GetNextRetryApnItem(), nullptr);
}

/**
 * @tc.number   GetNextRetryApnItem002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryApnItem002, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->currentApnIndex_ = 10;
    EXPECT_EQ(connectionRetryPolicy->currentApnIndex_, 10);
}

/**
 * @tc.number   GetNextRetryApnItem_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryApnItem_003, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->currentApnIndex_ = -1;
    EXPECT_EQ(connectionRetryPolicy->currentApnIndex_, -1);
}

/**
 * @tc.number   GetNextRetryApnItem_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryApnItem_004, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->MarkBadApn(true);
    connectionRetryPolicy->matchedApns_.push_back(defaultApnItem);
    connectionRetryPolicy->currentApnIndex_ = 0;
    EXPECT_EQ(connectionRetryPolicy->currentApnIndex_, 0);
}

/**
 * @tc.number   GetNextRetryApnItem_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryApnItem_005, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->MarkBadApn(false);
    connectionRetryPolicy->matchedApns_.push_back(defaultApnItem);
    connectionRetryPolicy->tryCount_ = 10;
    connectionRetryPolicy->maxCount_ = 0;
    connectionRetryPolicy->currentApnIndex_ = 0;
    EXPECT_EQ(connectionRetryPolicy->currentApnIndex_, 0);
}

/**
 * @tc.number   GetNextRetryApnItem_006
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryApnItem_006, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->MarkBadApn(false);
    connectionRetryPolicy->matchedApns_.push_back(defaultApnItem);
    connectionRetryPolicy->tryCount_ = -1;
    connectionRetryPolicy->maxCount_ = 0;
    connectionRetryPolicy->currentApnIndex_ = 0;
    EXPECT_EQ(connectionRetryPolicy->currentApnIndex_, 0);
}

/**
 * @tc.number   GetNextRetryApnItem_007
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryApnItem_007, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->matchedApns_.clear();
    connectionRetryPolicy->currentApnIndex_ = 0;
    EXPECT_EQ(connectionRetryPolicy->GetNextRetryApnItem(), nullptr);
}

/**
 * @tc.number   OnPropChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, OnPropChanged_001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->OnPropChanged(nullptr, nullptr, nullptr);
    EXPECT_EQ(connectionRetryPolicy->isPropOn_, true);
    connectionRetryPolicy->OnPropChanged("persist.telephony.retrystrategy.allow", nullptr, nullptr);
    EXPECT_EQ(connectionRetryPolicy->isPropOn_, true);
    connectionRetryPolicy->OnPropChanged("persist.telephony.retrystrategy.allow", "true", nullptr);
    EXPECT_EQ(connectionRetryPolicy->isPropOn_, true);
    connectionRetryPolicy->OnPropChanged("fakeKey", nullptr, nullptr);
    EXPECT_EQ(connectionRetryPolicy->isPropOn_, true);
    connectionRetryPolicy->OnPropChanged("fakeKey", "true", nullptr);
    EXPECT_EQ(connectionRetryPolicy->isPropOn_, true);
}

/**
 * @tc.number   IsAllBadApn_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, IsAllBadApn_001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->matchedApns_.clear();
    EXPECT_TRUE(connectionRetryPolicy->IsAllBadApn());
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->MarkBadApn(false);
    connectionRetryPolicy->matchedApns_.push_back(defaultApnItem);
    EXPECT_FALSE(connectionRetryPolicy->IsAllBadApn());
    defaultApnItem->MarkBadApn(true);
    EXPECT_TRUE(connectionRetryPolicy->IsAllBadApn());
}

/**
 * @tc.number   GetNextRetryDelay_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryDelay_001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->matchedApns_.clear();
    auto delay = connectionRetryPolicy->GetNextRetryDelay(DATA_CONTEXT_ROLE_DEFAULT, 0, 0,
        RetryScene::RETRY_SCENE_OTHERS);
    EXPECT_GE(delay, 0);
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->MarkBadApn(true);
    connectionRetryPolicy->matchedApns_.push_back(defaultApnItem);
    delay = connectionRetryPolicy->GetNextRetryDelay(DATA_CONTEXT_ROLE_DEFAULT, 0, 0, RetryScene::RETRY_SCENE_OTHERS);
    EXPECT_GE(delay, 0);
}

/**
 * @tc.number   GetNextRetryDelay_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetNextRetryDelay_002, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->matchedApns_.clear();
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    defaultApnItem->MarkBadApn(false);
    connectionRetryPolicy->matchedApns_.push_back(defaultApnItem);
    auto delay = connectionRetryPolicy->GetNextRetryDelay(DATA_CONTEXT_ROLE_DEFAULT, 0, 0,
        RetryScene::RETRY_SCENE_OTHERS);
    EXPECT_GE(delay, 0);
    connectionRetryPolicy->isPropOn_ = false;
    delay = connectionRetryPolicy->GetNextRetryDelay(DATA_CONTEXT_ROLE_DEFAULT, 0, 0, RetryScene::RETRY_SCENE_OTHERS);
    EXPECT_GE(delay, 0);
    connectionRetryPolicy->isPropOn_ = true;
    delay = connectionRetryPolicy->GetNextRetryDelay(DATA_CONTEXT_ROLE_DEFAULT, 0, 0, RetryScene::RETRY_SCENE_OTHERS);
    EXPECT_GE(delay, 0);
    delay = connectionRetryPolicy->GetNextRetryDelay(DATA_CONTEXT_ROLE_MMS, 0, 0, RetryScene::RETRY_SCENE_OTHERS);
    EXPECT_GE(delay, 0);
}

/**
 * @tc.number   SetMatchedApns_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, SetMatchedApns_001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    std::vector<sptr<ApnItem>> matchedApns = {};
    connectionRetryPolicy->SetMatchedApns(matchedApns);
    EXPECT_EQ(connectionRetryPolicy->GetMatchedApns().size(), 0);
}

/**
 * @tc.number   GetRandomDelay_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetRandomDelay_001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    EXPECT_GE(connectionRetryPolicy->GetRandomDelay(), 0);
    EXPECT_LE(connectionRetryPolicy->GetRandomDelay(), 2000);
}

/**
 * @tc.number   ConvertPdpErrorToDisconnReason_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, ConvertPdpErrorToDisconnReason_001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    auto res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_OPERATOR_DETERMINED_BARRING);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_MISSING_OR_UNKNOWN_APN);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_UNKNOWN_PDP_ADDR_OR_TYPE);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_USER_VERIFICATION);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_ACTIVATION_REJECTED_GGSN);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_SERVICE_OPTION_NOT_SUPPORTED);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_NSAPI_ALREADY_USED);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_IPV4_ONLY_ALLOWED);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_IPV6_ONLY_ALLOWED);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_PROTOCOL_ERRORS);
    EXPECT_EQ(res, DisConnectionReason::REASON_PERMANENT_REJECT);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_UNKNOWN_TO_CLEAR_CONNECTION);
    EXPECT_EQ(res, DisConnectionReason::REASON_CLEAR_CONNECTION);
    res = connectionRetryPolicy->ConvertPdpErrorToDisconnReason(
        PdpErrorReason::PDP_ERR_RETRY);
    EXPECT_EQ(res, DisConnectionReason::REASON_RETRY_CONNECTION);
}

/**
 * @tc.number   InitialRetryCountValue_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, InitialRetryCountValue_001, TestSize.Level0)
{
    std::shared_ptr<ConnectionRetryPolicy> connectionRetryPolicy = std::make_shared<ConnectionRetryPolicy>();
    connectionRetryPolicy->InitialRetryCountValue();
    EXPECT_EQ(connectionRetryPolicy->tryCount_, 0);
}

/**
 * @tc.number   EnableCellularDataRoaming_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, EnableCellularDataRoaming_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().EnableCellularDataRoaming(0, true);
    EXPECT_NE(result, true);
}

/**
 * @tc.number   HasInternetCapability_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, HasInternetCapability_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().HasInternetCapability(0, 0);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   HandleApnChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, HandleApnChanged_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().HandleApnChanged(0);
    EXPECT_NE(result, true);
}

/**
 * @tc.number   UpdateDefaultCellularDataSlotId_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, UpdateDefaultCellularDataSlotId_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().UpdateDefaultCellularDataSlotId();
    EXPECT_EQ(result, 0);
}
} // namespace Telephony
} // namespace OHOS