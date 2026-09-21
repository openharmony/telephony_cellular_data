/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "incall_data_state_machine.h"
#include "mock/mock_network_search.h"
#include "mock/mock_sim_manager.h"
#include "network_search_types.h"
#include "parameters.h"
#include "radio_event.h"
#include "sim_state_type.h"
#include "telephony_types.h"
#include "uri.h"

namespace OHOS {
namespace Telephony {
using namespace testing;
using namespace testing::ext;

namespace {
const std::string BACKUP_SIM_LIST_VALUE_VALID = "0,1,0;1,2,1";
const std::string BACKUP_SIM_LIST_VALUE_SINGLE = "0,1,0";
const std::string BACKUP_SIM_LIST_VALUE_INVALID = "-1,x,0;abc;1,2";
}

class IncallDataTestHandler : public TelEventHandler {
public:
    IncallDataTestHandler() : TelEventHandler("IncallDataTestHandler") {}
    ~IncallDataTestHandler() = default;
};

class IncallDataStateMachineBranchTest : public testing::Test {
public:
    IncallDataStateMachineBranchTest() = default;
    ~IncallDataStateMachineBranchTest() = default;
    static void SetUpTestCase() {}
    static void TearDownTestCase()
    {
        UnmockManager();
    }
    void SetUp()
    {
        InitMockManager();
        CreateIncallDataStateMachine();
    }
    void TearDown()
    {
        UnmockManager();
        system::SetParameter(PERSIST_TSTS_MODE, "0");
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
        }
        if (CoreManagerInner::GetInstance().networkSearchManager_ != nullptr) {
            CoreManagerInner::GetInstance().networkSearchManager_ = nullptr;
        }
    }

    void CreateIncallDataStateMachine(int32_t slotId = 1)
    {
        handler_ = std::make_shared<IncallDataTestHandler>();
        stateMachine_ = std::make_shared<IncallDataStateMachine>();
        sptr<ApnManager> apnManager = nullptr;
        stateMachine_->Init(static_cast<int32_t>(TelCallStatus::CALL_STATUS_DIALING), slotId, handler_, apnManager);
    }

    bool WriteBackupSimList(const std::string &value)
    {
        auto settingHelper = CellularDataSettingsRdbHelper::GetInstance();
        if (settingHelper == nullptr) {
            return false;
        }
        std::shared_ptr<DataShare::DataShareHelper> helper = settingHelper->CreateDataShareHelper();
        if (helper == nullptr) {
            return false;
        }
        Uri backupSimListUri(CELLULAR_DATA_SETTING_BACKUP_SIM_LIST_URI);
        DataShare::DataSharePredicates predicates;
        predicates.EqualTo(CELLULAR_DATA_COLUMN_KEYWORD, BACKUP_SIM_LIST_COLUMN_ENABLE);
        DataShare::DataShareValuesBucket bucket;
        bucket.Put(CELLULAR_DATA_COLUMN_KEYWORD,
            DataShare::DataShareValueObject(std::string(BACKUP_SIM_LIST_COLUMN_ENABLE)));
        bucket.Put(CELLULAR_DATA_COLUMN_VALUE, DataShare::DataShareValueObject(value));
        int32_t result = helper->Update(backupSimListUri, predicates, bucket);
        if (result <= 0) {
            result = helper->Insert(backupSimListUri, bucket);
        }
        helper->NotifyChange(backupSimListUri);
        helper->Release();
        return result > 0;
    }

    bool SetIncallDataSwitchOn()
    {
        auto settingHelper = CellularDataSettingsRdbHelper::GetInstance();
        if (settingHelper == nullptr) {
            return false;
        }
        Uri intelligenceNetworkUri(CELLULAR_DATA_SETTING_INTELLIGENCE_NETWORK_URI);
        Uri smartDualCardUri(CELLULAR_DATA_SETTING_INTELLIGENCE_SWITCH_URI);
        if (settingHelper->PutValue(intelligenceNetworkUri, INTELLIGENCE_NETWORK_COLUMN_ENABLE, 1) !=
            TELEPHONY_ERR_SUCCESS) {
            return false;
        }
        if (settingHelper->PutValue(smartDualCardUri, INTELLIGENCE_SWITCH_COLUMN_ENABLE, 1) != TELEPHONY_ERR_SUCCESS) {
            return false;
        }
        return true;
    }

    std::shared_ptr<IncallDataStateMachine> stateMachine_ = nullptr;
    std::shared_ptr<IncallDataTestHandler> handler_ = nullptr;
    MockSimManager *mockSimManager = nullptr;
    MockNetworkSearchManager *mockNetworkSearchManager = nullptr;
};

/**
 * @tc.number: IncallDataStateMachine_CanActiveDataByRadioTech_001
 * @tc.name: Test CanActiveDataByRadioTech with different radio tech
 * @tc.desc: Verify CanActiveDataByRadioTech returns true for LTE/NR/WCDMA and false for GSM/INVALID
 */
HWTEST_F(IncallDataStateMachineBranchTest, IncallDataStateMachine_CanActiveDataByRadioTech_001,
    Function | MediumTest | Level3)
{
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE)), Return(0)));
    ASSERT_TRUE(stateMachine_->CanActiveDataByRadioTech());

    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_NR)), Return(0)));
    ASSERT_TRUE(stateMachine_->CanActiveDataByRadioTech());

    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_WCDMA)), Return(0)));
    ASSERT_TRUE(stateMachine_->CanActiveDataByRadioTech());

    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_GSM)), Return(0)));
    ASSERT_FALSE(stateMachine_->CanActiveDataByRadioTech());

    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID)), Return(0)));
    ASSERT_FALSE(stateMachine_->CanActiveDataByRadioTech());
}

/**
 * @tc.number: IncallDataStateMachine_CheckBackupNetworkIfAllow_001
 * @tc.name: Test CheckBackupNetworkIfAllow with empty backup sim list
 * @tc.desc: Verify CheckBackupNetworkIfAllow returns true when backup sim list is not configured
 */
HWTEST_F(IncallDataStateMachineBranchTest, IncallDataStateMachine_CheckBackupNetworkIfAllow_001,
    Function | MediumTest | Level3)
{
    if (!WriteBackupSimList("")) {
        GTEST_SKIP() << "datashare unavailable, skip empty backup list case";
    }
    ASSERT_TRUE(stateMachine_->CheckBackupNetworkIfAllow(0));
}

/**
 * @tc.number: IncallDataStateMachine_CheckBackupNetworkIfAllow_002
 * @tc.name: Test CheckBackupNetworkIfAllow with single backup card
 * @tc.desc: Verify CheckBackupNetworkIfAllow returns true when backup card num is less than VALID_MIN_BACKUP_NUM
 */
HWTEST_F(IncallDataStateMachineBranchTest, IncallDataStateMachine_CheckBackupNetworkIfAllow_002,
    Function | MediumTest | Level3)
{
    if (!WriteBackupSimList(BACKUP_SIM_LIST_VALUE_SINGLE)) {
        GTEST_SKIP() << "datashare unavailable, skip single backup card case";
    }
    ASSERT_TRUE(stateMachine_->CheckBackupNetworkIfAllow(0));
}

/**
 * @tc.number: IncallDataStateMachine_CheckBackupNetworkIfAllow_003
 * @tc.name: Test CheckBackupNetworkIfAllow with matched and unmatched sim label
 * @tc.desc: Verify CheckBackupNetworkIfAllow returns true when target slot is in backup list, false when not
 */
HWTEST_F(IncallDataStateMachineBranchTest, IncallDataStateMachine_CheckBackupNetworkIfAllow_003,
    Function | MediumTest | Level3)
{
    if (!WriteBackupSimList(BACKUP_SIM_LIST_VALUE_VALID)) {
        GTEST_SKIP() << "datashare unavailable, skip matched sim label case";
    }
    // sim label index 2 esim matches the second backup card entry "1,2,1"
    SimLabel esimLabel;
    esimLabel.simType = SimType::ESIM;
    esimLabel.index = 2;
    EXPECT_CALL(*mockSimManager, GetSimLabel(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(esimLabel), Return(0)));
    ASSERT_TRUE(stateMachine_->CheckBackupNetworkIfAllow(0));

    // sim label not in backup list
    SimLabel unmatchedLabel;
    unmatchedLabel.simType = SimType::PSIM;
    unmatchedLabel.index = 9;
    EXPECT_CALL(*mockSimManager, GetSimLabel(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(unmatchedLabel), Return(0)));
    ASSERT_FALSE(stateMachine_->CheckBackupNetworkIfAllow(0));
}

/**
 * @tc.number: IncallDataStateMachine_CheckBackupNetworkIfAllow_004
 * @tc.name: Test CheckBackupNetworkIfAllow with invalid backup sim list
 * @tc.desc: Verify CheckBackupNetworkIfAllow returns true when backup list content is malformed
 */
HWTEST_F(IncallDataStateMachineBranchTest, IncallDataStateMachine_CheckBackupNetworkIfAllow_004,
    Function | MediumTest | Level3)
{
    if (!WriteBackupSimList(BACKUP_SIM_LIST_VALUE_INVALID)) {
        GTEST_SKIP() << "datashare unavailable, skip invalid backup list case";
    }
    SimLabel simLabel;
    simLabel.simType = SimType::PSIM;
    simLabel.index = 2;
    EXPECT_CALL(*mockSimManager, GetSimLabel(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(simLabel), Return(0)));
    ASSERT_TRUE(stateMachine_->CheckBackupNetworkIfAllow(0));
}

/**
 * @tc.number: IdleState_ProcessCallStarted_001
 * @tc.name: Test ProcessCallStarted when backup network is not allowed
 * @tc.desc: Verify ProcessCallStarted keeps idle state when target slot is not allowed to switch in TSTS mode
 */
HWTEST_F(IncallDataStateMachineBranchTest, IdleState_ProcessCallStarted_001, Function | MediumTest | Level3)
{
    system::SetParameter(PERSIST_TSTS_MODE, "1");
    CreateIncallDataStateMachine(CELLDATA_SLOT_ID_3);
    if (!SetIncallDataSwitchOn() || !WriteBackupSimList(BACKUP_SIM_LIST_VALUE_VALID)) {
        GTEST_SKIP() << "datashare unavailable, skip backup not allowed case";
    }
    EXPECT_CALL(*mockSimManager, GetActiveSimAccountInfoList(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(std::vector<IccAccountInfo>()), Return(0)));
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(static_cast<int32_t>(DSDS_MODE_V2)), Return(0)));
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(CELLDATA_SLOT_ID_0), Return(0)));
    EXPECT_CALL(*mockSimManager, HasSimCard(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(true), Return(0)));
    ImsRegInfo registered;
    registered.imsRegState = ImsRegState::IMS_REGISTERED;
    EXPECT_CALL(*mockNetworkSearchManager, GetImsRegStatus(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(registered), Return(0)));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE)), Return(0)));
    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId())
        .WillRepeatedly(Return(CELLDATA_SLOT_ID_0));
    SimLabel unmatchedLabel;
    unmatchedLabel.simType = SimType::PSIM;
    unmatchedLabel.index = 9;
    EXPECT_CALL(*mockSimManager, GetSimLabel(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(unmatchedLabel), Return(0)));

    auto idleState = std::static_pointer_cast<IdleState>(stateMachine_->idleState_);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_STARTED);
    ASSERT_TRUE(idleState->ProcessCallStarted(event));
    ASSERT_FALSE(stateMachine_->IsSecondaryActiveState());
}

/**
 * @tc.number: IdleState_ProcessCallStarted_002
 * @tc.name: Test ProcessCallStarted when backup network is allowed
 * @tc.desc: Verify ProcessCallStarted switches default slot and transitions to activating state in TSTS mode
 */
HWTEST_F(IncallDataStateMachineBranchTest, IdleState_ProcessCallStarted_002, Function | MediumTest | Level3)
{
    system::SetParameter(PERSIST_TSTS_MODE, "1");
    CreateIncallDataStateMachine(CELLDATA_SLOT_ID_3);
    if (!SetIncallDataSwitchOn() || !WriteBackupSimList(BACKUP_SIM_LIST_VALUE_VALID)) {
        GTEST_SKIP() << "datashare unavailable, skip backup allowed case";
    }
    EXPECT_CALL(*mockSimManager, GetActiveSimAccountInfoList(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(std::vector<IccAccountInfo>()), Return(0)));
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(static_cast<int32_t>(DSDS_MODE_V2)), Return(0)));
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(CELLDATA_SLOT_ID_0), Return(0)));
    EXPECT_CALL(*mockSimManager, HasSimCard(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(true), Return(0)));
    ImsRegInfo registered;
    registered.imsRegState = ImsRegState::IMS_REGISTERED;
    EXPECT_CALL(*mockNetworkSearchManager, GetImsRegStatus(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(registered), Return(0)));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE)), Return(0)));
    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId())
        .WillRepeatedly(Return(CELLDATA_SLOT_ID_0));
    SimLabel esimLabel;
    esimLabel.simType = SimType::ESIM;
    esimLabel.index = 2;
    EXPECT_CALL(*mockSimManager, GetSimLabel(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(esimLabel), Return(0)));
    EXPECT_CALL(*mockSimManager, SetDefaultCellularDataSlotId(CELLDATA_SLOT_ID_1))
        .Times(1)
        .WillOnce(Return(0));

    auto idleState = std::static_pointer_cast<IdleState>(stateMachine_->idleState_);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_STARTED);
    ASSERT_TRUE(idleState->ProcessCallStarted(event));
    ASSERT_EQ(stateMachine_->stateMachineEventHandler_->destState_, stateMachine_->activatingSecondaryState_);
}

/**
 * @tc.number: SecondaryActiveState_ProcessCallEnded_001
 * @tc.name: Test ProcessCallEnded transitions in secondary active state
 * @tc.desc: Verify ProcessCallEnded transitions to deactivating state when default slot differs from primary slot,
 *           and to idle state when they are the same
 */
HWTEST_F(IncallDataStateMachineBranchTest, SecondaryActiveState_ProcessCallEnded_001, Function | MediumTest | Level3)
{
    stateMachine_->UpdateCallState(static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE));
    auto secondaryActiveState = std::static_pointer_cast<SecondaryActiveState>(stateMachine_->secondaryActiveState_);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_ENDED);

    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).WillOnce(Return(CELLDATA_SLOT_ID_2));
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillOnce(DoAll(SetArgReferee<0>(CELLDATA_SLOT_ID_0), Return(0)));
    ASSERT_TRUE(secondaryActiveState->ProcessCallEnded(event));
    ASSERT_EQ(stateMachine_->stateMachineEventHandler_->destState_, stateMachine_->deactivatingSecondaryState_);

    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).WillOnce(Return(CELLDATA_SLOT_ID_0));
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillOnce(DoAll(SetArgReferee<0>(CELLDATA_SLOT_ID_0), Return(0)));
    ASSERT_TRUE(secondaryActiveState->ProcessCallEnded(event));
    ASSERT_EQ(stateMachine_->stateMachineEventHandler_->destState_, stateMachine_->idleState_);
}

/**
 * @tc.number: DeactivatingSecondaryState_StateBegin_001
 * @tc.name: Test DeactivatingSecondaryState StateBegin branches
 * @tc.desc: Verify StateBegin restores primary slot when default slot differs and transitions to idle state,
 *           and transitions to idle state directly when they are the same
 */
HWTEST_F(IncallDataStateMachineBranchTest, DeactivatingSecondaryState_StateBegin_001, Function | MediumTest | Level3)
{
    auto deactivatingSecondaryState =
        std::static_pointer_cast<DeactivatingSecondaryState>(stateMachine_->deactivatingSecondaryState_);

    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).WillOnce(Return(CELLDATA_SLOT_ID_2));
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillOnce(DoAll(SetArgReferee<0>(CELLDATA_SLOT_ID_0), Return(0)));
    EXPECT_CALL(*mockSimManager, SetDefaultCellularDataSlotId(CELLDATA_SLOT_ID_0))
        .Times(1)
        .WillOnce(Return(0));
    deactivatingSecondaryState->StateBegin();
    ASSERT_EQ(stateMachine_->stateMachineEventHandler_->destState_, stateMachine_->idleState_);

    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).WillOnce(Return(CELLDATA_SLOT_ID_0));
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillOnce(DoAll(SetArgReferee<0>(CELLDATA_SLOT_ID_0), Return(0)));
    deactivatingSecondaryState->StateBegin();
    ASSERT_EQ(stateMachine_->stateMachineEventHandler_->destState_, stateMachine_->idleState_);
}

/**
 * @tc.number: DeactivatingSecondaryState_StateProcess_001
 * @tc.name: Test DeactivatingSecondaryState StateProcess branches
 * @tc.desc: Verify StateProcess handles data disconnected and settings on events correctly
 */
HWTEST_F(IncallDataStateMachineBranchTest, DeactivatingSecondaryState_StateProcess_001, Function | MediumTest | Level3)
{
    auto deactivatingSecondaryState =
        std::static_pointer_cast<DeactivatingSecondaryState>(stateMachine_->deactivatingSecondaryState_);
    AppExecFwk::InnerEvent::Pointer event(nullptr, nullptr);
    ASSERT_FALSE(deactivatingSecondaryState->StateProcess(event));

    // current slot differs from primary slot, transition to idle state
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillOnce(DoAll(SetArgReferee<0>(CELLDATA_SLOT_ID_0), Return(0)));
    auto disconnectedEvent =
        AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    ASSERT_TRUE(deactivatingSecondaryState->StateProcess(disconnectedEvent));
    ASSERT_EQ(stateMachine_->stateMachineEventHandler_->destState_, stateMachine_->idleState_);

    // current slot equals primary slot, no transition
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_))
        .WillOnce(DoAll(SetArgReferee<0>(stateMachine_->GetSlotId()), Return(0)));
    ASSERT_TRUE(deactivatingSecondaryState->StateProcess(disconnectedEvent));

    auto settingsOnEvent = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_ON);
    ASSERT_TRUE(deactivatingSecondaryState->StateProcess(settingsOnEvent));

    auto unknownEvent = AppExecFwk::InnerEvent::Get(0);
    ASSERT_FALSE(deactivatingSecondaryState->StateProcess(unknownEvent));
}

/**
 * @tc.number: ActivatingSecondaryState_StateProcess_001
 * @tc.name: Test ActivatingSecondaryState StateProcess branches
 * @tc.desc: Verify StateProcess transitions to activated state on data connected event
 */
HWTEST_F(IncallDataStateMachineBranchTest, ActivatingSecondaryState_StateProcess_001, Function | MediumTest | Level3)
{
    auto activatingSecondaryState =
        std::static_pointer_cast<ActivatingSecondaryState>(stateMachine_->activatingSecondaryState_);
    AppExecFwk::InnerEvent::Pointer event(nullptr, nullptr);
    ASSERT_FALSE(activatingSecondaryState->StateProcess(event));

    auto connectedEvent = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    ASSERT_TRUE(activatingSecondaryState->StateProcess(connectedEvent));
    ASSERT_EQ(stateMachine_->stateMachineEventHandler_->destState_, stateMachine_->activatedSecondaryState_);

    auto unknownEvent = AppExecFwk::InnerEvent::Get(0);
    ASSERT_FALSE(activatingSecondaryState->StateProcess(unknownEvent));
}
} // namespace Telephony
} // namespace OHOS
