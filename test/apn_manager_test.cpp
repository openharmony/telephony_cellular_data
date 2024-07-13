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
#include "data_connection_manager.h"
#include "gtest/gtest.h"
#include "tel_event_handler.h"
#include "pdp_profile_data.h"

namespace OHOS {
namespace Telephony{
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

void ApnManagerTest::SetUp() {
    apnManager = std::make_shared<ApnManager>();
}

void ApnManagerTest::TearDown() {
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
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnNameByApnId_001, Function | MediumTest | Level1)
{
    int32_t id = 1;
    std::string result = apnManager->FindApnNameByApnId(id);
    ASSERT_EQ(result, DATA_CONTEXT_ROLE_DEFAULT_ID);
}

/**
 * @tc.number   FindApnNameByApnId_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, FindApnNameByApnId_002, Function | MediumTest | Level1)
{
    int32_t id = 2;
    std::string result = apnManager->FindApnNameByApnId(id);
    ASSERT_EQ(result, DATA_CONTEXT_ROLE_MMS_ID);
}

/**
 * @tc.number   FindApnIdByCapability_001
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
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
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, GetRilAttachApn_001, Function | MediumTest | Level1)
{
    std::vector<sptr<ApnItem>> allApnItem;
    apnManager->allApnItem_ = allApnItem;
    ASSERT_EQ(apnManager->GetRilAttachApn(), nullptr);
}
} // namespace Telephony
} // namespace OHOS