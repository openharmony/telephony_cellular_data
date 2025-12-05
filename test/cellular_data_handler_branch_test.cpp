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

#include "common_event_manager.h"
#include "common_event_support.h"
#include "cellular_data_constant.h"
#include "cellular_data_handler.h"
#include "core_manager_inner.h"
#include "mock/mock_sim_manager.h"
#include "mock/mock_network_search.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;

class CellularDataHandlerBranchTest : public testing::Test {
public:
    CellularDataHandlerBranchTest() = default;
    ~CellularDataHandlerBranchTest() = default;
    static void TearDownTestCase()
    {
        UnmockManager();
    }

    void InitMockManager()
    {
        mockSimManager = new MockSimManager();
        std::shared_ptr<MockSimManager> mockSimManagerPtr(mockSimManager);
        CoreManagerInner::GetInstance().simManager_ = mockSimManagerPtr;

        mockNetworkSearchManager = new MockNetworkSearchManager();
        std::shared_ptr<MockNetworkSearchManager> mockNetworkSearchManagerPtr(mockNetworkSearchManager);
        CoreManagerInner::GetInstance().networkSearchManager_ = mockNetworkSearchManagerPtr;
    }

    static void UnmockManager()
    {
        if (CoreManagerInner::GetInstance().simManager_ != nullptr) {
            CoreManagerInner::GetInstance().simManager_ = nullptr;
            std::cout << "CellularDataHandlerBranchTest set simManager_ nullptr" << std::endl;
        }
        if (CoreManagerInner::GetInstance().networkSearchManager_ != nullptr) {
            CoreManagerInner::GetInstance().networkSearchManager_ = nullptr;
            std::cout << "CellularDataHandlerBranchTest set networkSearchManager_ nullptr" << std::endl;
        }
    }

    void InitCellularDataHandler()
    {
        cellularDataHandler_ = std::make_shared<CellularDataHandler>(0);
        cellularDataHandler_->Init();
    }

    std::shared_ptr<CellularDataHandler> cellularDataHandler_;
    MockSimManager *mockSimManager;
    MockNetworkSearchManager *mockNetworkSearchManager;
};

HWTEST_F(CellularDataHandlerBranchTest, RoamingStateOn_001, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();
    // GetPsRoamingState return 1, dataRoamingEnabled is false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler_->RoamingStateOn(event);

    // GetPsRoamingState return 1, dataRoamingEnabled is false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockSimManager, GetSimId(_)).Times(AtLeast(0)).WillRepeatedly(Return(1));
    cellularDataHandler_->defaultDataRoamingEnable_ = true;
    auto event1 = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler_->RoamingStateOn(event1);
    EXPECT_EQ(cellularDataHandler_->apnManager_->GetOverallApnState(), PROFILE_STATE_IDLE);
    UnmockManager();
}

HWTEST_F(CellularDataHandlerBranchTest, CheckRoamingState_001, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();
    // roamingState true, dataRoamingEnabled true, isMmsApn true, isEmergencyApn false, IsRestrictedMode false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _)).WillOnce(Return(0));
    sptr<ApnHolder> apnHolder = new ApnHolder("mms", 0);
    ASSERT_FALSE(cellularDataHandler_ == nullptr);
    ASSERT_FALSE(cellularDataHandler_->dataSwitchSettings_ == nullptr);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(true);
    ASSERT_TRUE(cellularDataHandler_->CheckRoamingState(apnHolder));

    // roamingState false, dataRoamingEnabled true, isMmsApn false, isEmergencyApn true, IsRestrictedMode false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _)).WillOnce(Return(0));
    apnHolder = new ApnHolder("emergency", 0);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(true);
    ASSERT_TRUE(cellularDataHandler_->CheckRoamingState(apnHolder));

    // roamingState true, dataRoamingEnabled false, isMmsApn true, isEmergencyApn false, IsRestrictedMode false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    apnHolder = new ApnHolder("mms", 0);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(false);
    ASSERT_FALSE(cellularDataHandler_->CheckRoamingState(apnHolder));

    // roamingState false, dataRoamingEnabled false, isMmsApn false, isEmergencyApn true, IsRestrictedMode true
    int32_t tech = 1;
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _)).WillOnce(DoAll(SetArgReferee<1>(tech), Return(0)));
    cellularDataHandler_->lastCallState_ = 1;
    apnHolder = new ApnHolder("emergency", 0);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(false);
    ASSERT_FALSE(cellularDataHandler_->CheckRoamingState(apnHolder));
    UnmockManager();
}

HWTEST_F(CellularDataHandlerBranchTest, IsCellularDataRoamingEnabled_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2);

    EXPECT_EQ(cellularDataHandler->dataSwitchSettings_, nullptr);
    cellularDataHandler->defaultDataRoamingEnable_ = false;
    bool result = false;
    cellularDataHandler->IsCellularDataRoamingEnabled(result);
    ASSERT_FALSE(result);

    cellularDataHandler->Init();
    cellularDataHandler->IsCellularDataRoamingEnabled(result);
    ASSERT_TRUE(result);
}

HWTEST_F(CellularDataHandlerBranchTest, SetDataPermittedForMms_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2);

    EXPECT_EQ(cellularDataHandler->incallDataStateMachine_, nullptr);
    cellularDataHandler->incallDataStateMachine_ =
        cellularDataHandler->CreateIncallDataStateMachine(CALL_STATUS_DIALING);
    ASSERT_FALSE(cellularDataHandler->SetDataPermittedForMms(false));
}

HWTEST_F(CellularDataHandlerBranchTest, CheckApnState_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2);
    cellularDataHandler->Init();

    sptr<ApnHolder> apnHolder = new ApnHolder("mms", 0);
    apnHolder->SetApnState(PROFILE_STATE_DISCONNECTING);
    ASSERT_FALSE(cellularDataHandler->CheckApnState(apnHolder));

    apnHolder->SetApnState(PROFILE_STATE_FAILED);
    ASSERT_FALSE(cellularDataHandler->CheckApnState(apnHolder));

    apnHolder->SetApnState(PROFILE_STATE_CONNECTING);
    ASSERT_FALSE(cellularDataHandler->CheckApnState(apnHolder));
}

HWTEST_F(CellularDataHandlerBranchTest, EstablishDataConnection_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2);
    cellularDataHandler->Init();

    sptr<ApnHolder> apnHolder = new ApnHolder("mms", 0);
    ApnItem apnItem;
    sptr<ApnItem> item = apnItem.MakeDefaultApn("mms");
    std::vector<sptr<ApnItem>> matchedApns;
    matchedApns.push_back(item);
    apnHolder->SetAllMatchedApns(matchedApns);
    cellularDataHandler->EstablishDataConnection(apnHolder, 1);
    ASSERT_EQ(apnHolder->GetApnState(), PROFILE_STATE_CONNECTING);
}

HWTEST_F(CellularDataHandlerBranchTest, DisconnectDataComplete_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2);
    cellularDataHandler->Init();

    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler->DisconnectDataComplete(event);
    ASSERT_EQ(event->GetSharedObject<SetupDataCallResultInfo>(), nullptr);
}

HWTEST_F(CellularDataHandlerBranchTest, DisconnectDataComplete_002, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2);
    cellularDataHandler->Init();

    std::shared_ptr<SetupDataCallResultInfo> resultInfo = std::make_shared<SetupDataCallResultInfo>();
    resultInfo->flag = 2;
    auto event = AppExecFwk::InnerEvent::Get(0, resultInfo);
    sptr<ApnHolder> apnHolder = cellularDataHandler->apnManager_->FindApnHolderById(2);
    sptr<DataConnectionManager> connectionManager = std::make_unique<DataConnectionManager>(0).release();
    if (connectionManager == nullptr) {
        return;
    }
    connectionManager->Init();
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine =
        std::make_shared<CellularDataStateMachine>(connectionManager, cellularDataHandler);
    apnHolder->SetCellularDataStateMachine(cellularDataStateMachine);
    cellularDataHandler->DisconnectDataComplete(event);
    ASSERT_EQ(event->GetSharedObject<SetupDataCallResultInfo>(), resultInfo);
}

HWTEST_F(CellularDataHandlerBranchTest, UpdatePhysicalConnectionState_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2);
    cellularDataHandler->Init();

    cellularDataHandler->physicalConnectionActiveState_ = true;
    cellularDataHandler->UpdatePhysicalConnectionState(true);
    ASSERT_FALSE(cellularDataHandler->physicalConnectionActiveState_);

    cellularDataHandler->physicalConnectionActiveState_ = false;
    cellularDataHandler->UpdatePhysicalConnectionState(true);
    ASSERT_FALSE(cellularDataHandler->physicalConnectionActiveState_);

    cellularDataHandler->physicalConnectionActiveState_ = true;
    cellularDataHandler->UpdatePhysicalConnectionState(false);
    ASSERT_TRUE(cellularDataHandler->physicalConnectionActiveState_);

    cellularDataHandler->physicalConnectionActiveState_ = false;
    cellularDataHandler->UpdatePhysicalConnectionState(false);
    ASSERT_TRUE(cellularDataHandler->physicalConnectionActiveState_);
}

HWTEST_F(CellularDataHandlerBranchTest, HandleScreenStateChanged_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->HandleScreenStateChanged(true);
    ASSERT_EQ(cellularDataHandler->connectionManager_, nullptr);
    cellularDataHandler->Init();
    cellularDataHandler->HandleScreenStateChanged(true);
    ASSERT_NE(cellularDataHandler->connectionManager_, nullptr);
}

HWTEST_F(CellularDataHandlerBranchTest, UpdateCellularDataConnectState_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();

    cellularDataHandler->UpdateCellularDataConnectState("default");
    ASSERT_NE(cellularDataHandler->apnManager_, nullptr);

    cellularDataHandler->UpdateCellularDataConnectState("internal_default");
    ASSERT_NE(cellularDataHandler->apnManager_, nullptr);

    cellularDataHandler->UpdateCellularDataConnectState("mss");
    ASSERT_NE(cellularDataHandler->apnManager_, nullptr);
}

HWTEST_F(CellularDataHandlerBranchTest, HandleDBSettingIncallChanged_001, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();
    cellularDataHandler->HandleImsCallChanged(TelCallStatus::CALL_STATUS_DIALING);

    auto event = AppExecFwk::InnerEvent::Get(0, static_cast<int64_t>(DataSwitchCode::CELLULAR_DATA_ENABLED));
    cellularDataHandler->HandleDBSettingIncallChanged(event);
    ASSERT_NE(cellularDataHandler->incallDataStateMachine_, nullptr);

    event = AppExecFwk::InnerEvent::Get(0, static_cast<int64_t>(DataSwitchCode::CELLULAR_DATA_DISABLED));
    cellularDataHandler->HandleDBSettingIncallChanged(event);
    ASSERT_NE(cellularDataHandler->incallDataStateMachine_, nullptr);

    cellularDataHandler->IncallDataComplete(event);
    ASSERT_NE(cellularDataHandler->incallDataStateMachine_, nullptr);
}

HWTEST_F(CellularDataHandlerBranchTest, HandleImsCallChanged, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();

    ASSERT_EQ(cellularDataHandler->incallDataStateMachine_, nullptr);
    cellularDataHandler->HandleImsCallChanged(TelCallStatus::CALL_STATUS_DIALING);
    cellularDataHandler->HandleImsCallChanged(TelCallStatus::CALL_STATUS_INCOMING);
    ASSERT_NE(cellularDataHandler->incallDataStateMachine_, nullptr);
    cellularDataHandler->HandleVoiceCallChanged(TelCallStatus::CALL_STATUS_DISCONNECTED);
    cellularDataHandler->HandleVoiceCallChanged(TelCallStatus::CALL_STATUS_IDLE);
    ASSERT_NE(cellularDataHandler->incallDataStateMachine_, nullptr);
}

HWTEST_F(CellularDataHandlerBranchTest, ReleaseAllNetworkRequest, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();
    cellularDataHandler->ReleaseAllNetworkRequest();
    ASSERT_NE(cellularDataHandler->apnManager_, nullptr);
}

HWTEST_F(CellularDataHandlerBranchTest, HandleSimStateChanged, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();

    SimState simState = SimState::SIM_STATE_READY;
    std::u16string iccId = u"iccId";
    EXPECT_CALL(*mockSimManager, GetSimState(_, _)).WillOnce(DoAll(SetArgReferee<1>(simState), Return(0)));
    EXPECT_CALL(*mockSimManager, GetSimIccId(_, _)).WillOnce(DoAll(SetArgReferee<1>(iccId), Return(0)));
    cellularDataHandler_->HandleSimStateChanged();
    ASSERT_NE(cellularDataHandler_->lastIccId_, iccId);

    cellularDataHandler_->lastIccId_ = u"iccId";
    EXPECT_CALL(*mockSimManager, GetSimState(_, _)).WillOnce(DoAll(SetArgReferee<1>(simState), Return(0)));
    EXPECT_CALL(*mockSimManager, GetSimIccId(_, _)).WillOnce(DoAll(SetArgReferee<1>(iccId), Return(0)));
    cellularDataHandler_->HandleSimStateChanged();
    ASSERT_EQ(cellularDataHandler_->lastIccId_, iccId);

    cellularDataHandler_->isRilApnAttached_ = true;
    simState = SimState::SIM_STATE_NOT_PRESENT;
    EXPECT_CALL(*mockSimManager, GetSimState(_, _)).WillOnce(DoAll(SetArgReferee<1>(simState), Return(0)));
    cellularDataHandler_->HandleSimStateChanged();

    simState = SimState::SIM_STATE_NOT_READY;
    EXPECT_CALL(*mockSimManager, GetSimState(_, _)).WillOnce(DoAll(SetArgReferee<1>(simState), Return(0)));
    cellularDataHandler_->HandleSimStateChanged();
    ASSERT_FALSE(cellularDataHandler_->isRilApnAttached_);

    UnmockManager();
}

HWTEST_F(CellularDataHandlerBranchTest, HandleRecordsChanged, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();

    cellularDataHandler_->lastIccId_ = u"";
    std::u16string iccId = u"iccId";
    EXPECT_CALL(*mockSimManager, GetSimIccId(_, _)).WillOnce(DoAll(SetArgReferee<1>(iccId), Return(0)));
    EXPECT_CALL(*mockSimManager, GetOperatorConfigs(_, _)).Times(AtLeast(0));
    EXPECT_CALL(*mockSimManager, GetSimId(_)).Times(AtLeast(0));
    EXPECT_CALL(*mockSimManager, GetSimOperatorNumeric(_, _)).Times(AtLeast(0));
    EXPECT_CALL(*mockSimManager, IsCTSimCard(_, _)).Times(AtLeast(0));
    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).WillRepeatedly(Return(0));
    cellularDataHandler_->HandleSimAccountLoaded();
    cellularDataHandler_->HandleRecordsChanged();
    ASSERT_EQ(cellularDataHandler_->lastIccId_, iccId);

    UnmockManager();
}

HWTEST_F(CellularDataHandlerBranchTest, SetPolicyDataOn, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();

    cellularDataHandler->dataSwitchSettings_->SetPolicyDataOn(true);
    cellularDataHandler->SetPolicyDataOn(false);
    ASSERT_FALSE(cellularDataHandler->dataSwitchSettings_->IsPolicyDataOn());

    cellularDataHandler->SetPolicyDataOn(true);
    ASSERT_TRUE(cellularDataHandler->dataSwitchSettings_->IsPolicyDataOn());
}

HWTEST_F(CellularDataHandlerBranchTest, ChangeConnectionForDsds, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();

    cellularDataHandler->ChangeConnectionForDsds(false);
    ASSERT_TRUE(cellularDataHandler->ChangeConnectionForDsds(false));
    ASSERT_TRUE(cellularDataHandler->ChangeConnectionForDsds(true));
}

HWTEST_F(CellularDataHandlerBranchTest, HandleDBSettingRoamingChanged, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();

    auto event = AppExecFwk::InnerEvent::Get(0, static_cast<int64_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED));
    cellularDataHandler->HandleDBSettingRoamingChanged(event);

    event = AppExecFwk::InnerEvent::Get(0, static_cast<int64_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED));
    cellularDataHandler->HandleDBSettingRoamingChanged(event);
    ASSERT_EQ(event->GetParam(), 0);
}

HWTEST_F(CellularDataHandlerBranchTest, UnRegisterDataSettingObserver, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();

    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    cellularDataHandler_->UnRegisterDataSettingObserver();
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    cellularDataHandler_->RegisterDataSettingObserver();
    ASSERT_NE(cellularDataHandler_->settingObserver_, nullptr);
    ASSERT_NE(cellularDataHandler_->roamingObserver_, nullptr);
    ASSERT_NE(cellularDataHandler_->incallObserver_, nullptr);
    ASSERT_NE(cellularDataHandler_->cellularDataRdbObserver_, nullptr);

    UnmockManager();
}

HWTEST_F(CellularDataHandlerBranchTest, CheckForCompatibleDataConnection, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);
    cellularDataHandler->Init();

    sptr<ApnHolder> apnHolder = new ApnHolder("dun", 0);
    cellularDataHandler->CheckForCompatibleDataConnection(apnHolder);
    ASSERT_EQ(apnHolder->GetApnType(), DATA_CONTEXT_ROLE_DUN);
}

HWTEST_F(CellularDataHandlerBranchTest, ReleaseCellularDataConnection, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(0);

    cellularDataHandler->ReleaseCellularDataConnection();
    ASSERT_EQ(cellularDataHandler->apnManager_, nullptr);
    cellularDataHandler->Init();
    cellularDataHandler->ReleaseCellularDataConnection();
    ASSERT_NE(cellularDataHandler->apnManager_, nullptr);
}

HWTEST_F(CellularDataHandlerBranchTest, CheckCellularDataSlotId, Function | MediumTest | Level3)
{
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(2); // 2: vsim slot id
    cellularDataHandler->Init();
    sptr<ApnHolder> apnHolder = new ApnHolder(DATA_CONTEXT_ROLE_DEFAULT, 0);
    cellularDataHandler->CheckCellularDataSlotId(apnHolder);

    cellularDataHandler.reset();
    cellularDataHandler = std::make_shared<CellularDataHandler>(0); // 0: sim0 slot id
    cellularDataHandler->Init();
    apnHolder = new ApnHolder(DATA_CONTEXT_ROLE_MMS, 0);
    cellularDataHandler->CheckCellularDataSlotId(apnHolder);
    apnHolder->apnType_ = DATA_CONTEXT_ROLE_DEFAULT;
    bool ret = cellularDataHandler->CheckCellularDataSlotId(apnHolder);
    EXPECT_FALSE(ret);

    cellularDataHandler.reset();
    cellularDataHandler = std::make_shared<CellularDataHandler>(1); // 1: sim1 slot id
    cellularDataHandler->Init();
    apnHolder = new ApnHolder(DATA_CONTEXT_ROLE_MMS, 0);
    cellularDataHandler->CheckCellularDataSlotId(apnHolder);
    apnHolder->apnType_ = DATA_CONTEXT_ROLE_DEFAULT;
    ret = cellularDataHandler->CheckCellularDataSlotId(apnHolder);
    EXPECT_FALSE(ret);
}

HWTEST_F(CellularDataHandlerBranchTest, ResumeDataPermittedTimerOut_001, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();

    auto event = AppExecFwk::InnerEvent::Get(0);
    auto apnHolder =
        cellularDataHandler_->apnManager_->FindApnHolderById(DataContextRolesId::DATA_CONTEXT_ROLE_MMS_ID);
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    cellularDataHandler_->ResumeDataPermittedTimerOut(event);
    EXPECT_TRUE(cellularDataHandler_->dataSwitchSettings_->IsInternalDataOn());

    cellularDataHandler_->apnManager_ = nullptr;
    cellularDataHandler_->ResumeDataPermittedTimerOut(event);
    EXPECT_TRUE(cellularDataHandler_->dataSwitchSettings_->IsInternalDataOn());
}

HWTEST_F(CellularDataHandlerBranchTest, CheckAttachAndSimState_001, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();

    auto apnHolder =
        cellularDataHandler_->apnManager_->FindApnHolderById(DataContextRolesId::DATA_CONTEXT_ROLE_MMS_ID);
    SimState simState = SimState::SIM_STATE_READY;
    EXPECT_CALL(*mockSimManager, GetSimState(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(simState), Return(0)));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRegState(_)).WillOnce(Return(0));
    EXPECT_FALSE(cellularDataHandler_->CheckAttachAndSimState(apnHolder));

    cellularDataHandler_->RemoveAllEvents();
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRegState(_)).WillOnce(Return(1));
    cellularDataHandler_->CheckAttachAndSimState(apnHolder);
    EXPECT_FALSE(cellularDataHandler_->HasInnerEvent(CellularDataEventCode::MSG_RESUME_DATA_PERMITTED_TIMEOUT));
}
}  // namespace Telephony
}  // namespace OHOS