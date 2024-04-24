/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "activating.h"
#include "active.h"
#include "apn_holder.h"
#include "apn_item.h"
#include "apn_manager.h"
#include "cellular_data_client.h"
#include "cellular_data_constant.h"
#include "cellular_data_controller.h"
#include "cellular_data_dump_helper.h"
#include "cellular_data_handler.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_rdb_observer.h"
#include "cellular_data_roaming_observer.h"
#include "cellular_data_service.h"
#include "cellular_data_service_stub.h"
#include "cellular_data_setting_observer.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_state_machine.h"
#include "cellular_data_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "data_access_token.h"
#include "data_connection_manager.h"
#include "data_connection_monitor.h"
#include "datashare_result_set.h"
#include "default.h"
#include "disconnecting.h"
#include "gtest/gtest.h"
#include "inactive.h"
#include "incall_data_state_machine.h"
#include "net_manager_call_back.h"
#include "net_manager_tactics_call_back.h"
#include "network_search_callback.h"
#include "pdp_profile_data.h"
#include "state_notification.h"
#include "telephony_errors.h"
#include "telephony_hisysevent.h"
#include "telephony_log_wrapper.h"
#include "uri.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

namespace {
const int32_t INVALID_SLOTID = -1;
const int32_t INVALID_SLOTID_TWO = 5;
const int32_t INVALID_CID = -1;
const int32_t INVALID_FD = -1;
const int32_t DEFAULT_SIM_SLOT_ID = 0;
const int32_t SLEEP_TIME_SECONDS = 3;
const std::string ADDRESS = "127.0.0.1";
const std::string FLAG = ".";
} // namespace

class DemoHandler : public AppExecFwk::EventHandler {
public:
    explicit DemoHandler(std::shared_ptr<AppExecFwk::EventRunner> &runner) : AppExecFwk::EventHandler(runner) {}
    virtual ~DemoHandler() {}
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) {}
};

class BranchTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void BranchTest::SetUpTestCase() {}

void BranchTest::TearDownTestCase() {}

void BranchTest::SetUp() {}

void BranchTest::TearDown() {}

class StateMachineTest : public TelEventHandler {
public:
    StateMachineTest() : TelEventHandler("StateMachineTest") {}
    ~StateMachineTest() = default;
    std::shared_ptr<CellularDataStateMachine> CreateCellularDataConnect(int32_t slotId);

public:
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine_ = nullptr;
};

std::shared_ptr<CellularDataStateMachine> StateMachineTest::CreateCellularDataConnect(int32_t slotId)
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

class IncallStateMachineTest : public TelEventHandler {
public:
    IncallStateMachineTest() : TelEventHandler("IncallStateMachineTest") {}
    ~IncallStateMachineTest() = default;
    std::shared_ptr<IncallDataStateMachine> CreateIncallDataStateMachine(int32_t slotId);

public:
    std::shared_ptr<IncallDataStateMachine> incallStateMachine_ = nullptr;
};

std::shared_ptr<IncallDataStateMachine> IncallStateMachineTest::CreateIncallDataStateMachine(int32_t slotId)
{
    if (incallStateMachine_ != nullptr) {
        return incallStateMachine_;
    }
    sptr<ApnManager> apnManager = std::make_unique<ApnManager>().release();
    if (apnManager == nullptr) {
        return nullptr;
    }
    incallStateMachine_ = std::make_shared<IncallDataStateMachine>(slotId,
        std::weak_ptr<TelEventHandler>(std::static_pointer_cast<TelEventHandler>(shared_from_this())), apnManager);
    return incallStateMachine_;
}

/**
 * @tc.number   Telephony_CellularDataHandler_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_001, Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    CellularDataHandler cellularDataHandler { subscriberInfo, 0 };
    NetRequest request { 0, "simId1" };
    cellularDataHandler.HandleSimStateChanged();
    cellularDataHandler.ReleaseAllNetworkRequest();
    ASSERT_FALSE(cellularDataHandler.ReleaseNet(request));
    ASSERT_FALSE(cellularDataHandler.RequestNet(request));
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    cellularDataHandler.ProcessEvent(event);
    EventFwk::CommonEventData data;
    cellularDataHandler.OnReceiveEvent(data);
    ASSERT_NE(cellularDataHandler.SetCellularDataEnable(true), TELEPHONY_ERR_SUCCESS);
    bool dataEnaled = false;
    cellularDataHandler.IsCellularDataEnabled(dataEnaled);
    ASSERT_FALSE(dataEnaled);
    bool dataRoamingEnabled = false;
    cellularDataHandler.IsCellularDataRoamingEnabled(dataRoamingEnabled);
    ASSERT_FALSE(dataRoamingEnabled);
    ASSERT_NE(cellularDataHandler.SetCellularDataRoamingEnabled(true), TELEPHONY_ERR_SUCCESS);
    ASSERT_EQ(ApnProfileState::PROFILE_STATE_IDLE, cellularDataHandler.GetCellularDataState());
    ASSERT_EQ(ApnProfileState::PROFILE_STATE_IDLE, cellularDataHandler.GetCellularDataState(""));
    sptr<ApnHolder> apnHolder;
    DisConnectionReason reason = cellularDataHandler.GetDisConnectionReason();
    cellularDataHandler.ClearConnection(apnHolder, reason);
    cellularDataHandler.EstablishAllApnsIfConnectable();
    cellularDataHandler.ClearAllConnections(reason);
    cellularDataHandler.GetSlotId();
    ASSERT_FALSE(cellularDataHandler.HandleApnChanged());
    cellularDataHandler.HandleApnChanged(event);
    cellularDataHandler.GetCellularDataFlowType();
    cellularDataHandler.SetPolicyDataOn(true);
    ASSERT_FALSE(cellularDataHandler.IsRestrictedMode());
    cellularDataHandler.GetDisConnectionReason();
    ASSERT_FALSE(cellularDataHandler.HasInternetCapability(0));
    ASSERT_EQ(nullptr, cellularDataHandler.FindIdleCellularDataConnection());
    cellularDataHandler.AttemptEstablishDataConnection(apnHolder);
    ASSERT_FALSE(cellularDataHandler.CheckAttachAndSimState(apnHolder));
    ASSERT_FALSE(cellularDataHandler.CheckRoamingState(apnHolder));
    ASSERT_FALSE(cellularDataHandler.CheckApnState(apnHolder));
    cellularDataHandler.RemoveAllEvents();
    sleep(SLEEP_TIME_SECONDS);
}

/**
 * @tc.number   Telephony_CellularDataHandler_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_002, Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    CellularDataHandler cellularDataHandler { subscriberInfo, 0 };
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    sptr<ApnHolder> apnHolder;
    ApnItem::Attribute apnAttr;
    cellularDataHandler.RadioPsConnectionAttached(event);
    cellularDataHandler.RadioPsConnectionDetached(event);
    cellularDataHandler.RoamingStateOn(event);
    cellularDataHandler.RoamingStateOff(event);
    cellularDataHandler.EstablishDataConnectionComplete(event);
    cellularDataHandler.DisconnectDataComplete(event);
    cellularDataHandler.MsgEstablishDataConnection(event);
    cellularDataHandler.MsgRequestNetwork(event);
    cellularDataHandler.HandleSettingSwitchChanged(event);
    cellularDataHandler.HandleDBSettingIncallChanged(event);
    cellularDataHandler.HandleDefaultDataSubscriptionChanged();
    cellularDataHandler.IncallDataComplete(event);
    cellularDataHandler.HandleCallChanged(0);
    cellularDataHandler.HandleImsCallChanged(0);
    cellularDataHandler.HandleVoiceCallChanged(0);
    cellularDataHandler.HandleSimStateOrRecordsChanged(event);
    cellularDataHandler.HandleSimAccountLoaded(event);
    cellularDataHandler.HandleRadioStateChanged(event);
    cellularDataHandler.HandleDsdsModeChanged(event);
    cellularDataHandler.SetRilAttachApnResponse(event);
    cellularDataHandler.GetDefaultConfiguration();
    cellularDataHandler.GetDefaultDataRoamingConfig();
    cellularDataHandler.HandleRadioNrStateChanged(event);
    cellularDataHandler.HandleRadioNrFrequencyChanged(event);
    cellularDataHandler.HandleDBSettingEnableChanged(event);
    cellularDataHandler.HandleDBSettingRoamingChanged(event);
    cellularDataHandler.SetDataPermittedResponse(event);
    cellularDataHandler.OnRilAdapterHostDied(event);
    cellularDataHandler.OnCleanAllDataConnectionsDone(event);
    cellularDataHandler.RegisterDataSettingObserver();
    cellularDataHandler.UnRegisterDataSettingObserver();
    cellularDataHandler.GetDataConnApnAttr(apnAttr);
    cellularDataHandler.HandleFactoryReset(event);
    ASSERT_FALSE(cellularDataHandler.HasAnyHigherPriorityConnection(apnHolder));
    cellularDataHandler.RemoveAllEvents();
    sleep(SLEEP_TIME_SECONDS);
}

/**
 * @tc.number   Telephony_CellularDataHandler_003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_003, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    CellularDataHandler cellularDataHandler { subscriberInfo, INVALID_SLOTID };
    cellularDataHandler.apnManager_ = std::make_unique<ApnManager>().release();
    NetRequest request;
    ASSERT_FALSE(cellularDataHandler.ReleaseNet(request));
    cellularDataHandler.SetDataPermittedForMms(false);
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    cellularDataHandler.ClearConnection(apnHolder, reason);
    cellularDataHandler.EstablishAllApnsIfConnectable();
    cellularDataHandler.ClearAllConnections(reason);
    cellularDataHandler.ChangeConnectionForDsds(false);
    cellularDataHandler.connectionManager_ = std::make_unique<DataConnectionManager>(INVALID_SLOTID).release();
    cellularDataHandler.ClearAllConnections(reason);
    cellularDataHandler.EstablishAllApnsIfConnectable();
    cellularDataHandler.UpdatePhysicalConnectionState(true);
    cellularDataHandler.ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    ASSERT_FALSE(cellularDataHandler.CheckApnState(apnHolder));
    cellularDataHandler.AttemptEstablishDataConnection(apnHolder);
    cellularDataHandler.connectionManager_ = nullptr;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    cellularDataHandler.MsgEstablishDataConnection(event);
    ASSERT_FALSE(cellularDataHandler.HasAnyHigherPriorityConnection(apnHolder));
    cellularDataHandler.connectionManager_ = std::make_unique<DataConnectionManager>(INVALID_SLOTID).release();
    ASSERT_FALSE(cellularDataHandler.HasInternetCapability(INVALID_CID));
    cellularDataHandler.RemoveAllEvents();
    sleep(SLEEP_TIME_SECONDS);
}

/**
 * @tc.number   Telephony_CellularDataHandler_004
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_004, Function | MediumTest | Level1)
{
    CellularDataController controller { 0 };
    controller.Init();
    ASSERT_FALSE(controller.cellularDataHandler_ == nullptr);
    NetRequest request { 0, "simId1" };
    controller.cellularDataHandler_->ReleaseNet(request);
    controller.cellularDataHandler_->RequestNet(request);
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    controller.cellularDataHandler_->ProcessEvent(event);
    EventFwk::CommonEventData data;
    controller.cellularDataHandler_->OnReceiveEvent(data);
    ASSERT_EQ(controller.cellularDataHandler_->SetCellularDataEnable(true), TELEPHONY_ERR_SUCCESS);
    bool dataEnaled = false;
    controller.cellularDataHandler_->IsCellularDataEnabled(dataEnaled);
    bool dataRoamingEnabled = false;
    controller.cellularDataHandler_->IsCellularDataRoamingEnabled(dataRoamingEnabled);
    ASSERT_NE(controller.cellularDataHandler_->SetCellularDataRoamingEnabled(true), TELEPHONY_ERR_SUCCESS);
    controller.cellularDataHandler_->CreateCellularDataConnect();
    ASSERT_EQ(ApnProfileState::PROFILE_STATE_IDLE, controller.cellularDataHandler_->GetCellularDataState());
    ASSERT_EQ(ApnProfileState::PROFILE_STATE_IDLE, controller.cellularDataHandler_->GetCellularDataState(""));
    sptr<ApnHolder> apnHolder = controller.cellularDataHandler_->apnManager_->FindApnHolderById(1);
    DisConnectionReason reason = controller.cellularDataHandler_->GetDisConnectionReason();
    controller.cellularDataHandler_->ClearConnection(apnHolder, reason);
    controller.cellularDataHandler_->EstablishAllApnsIfConnectable();
    controller.cellularDataHandler_->ClearAllConnections(reason);
    controller.cellularDataHandler_->GetSlotId();
    ASSERT_TRUE(controller.cellularDataHandler_->HandleApnChanged());
    controller.cellularDataHandler_->HandleApnChanged(event);
    controller.cellularDataHandler_->GetCellularDataFlowType();
    controller.cellularDataHandler_->SetPolicyDataOn(true);
    ASSERT_FALSE(controller.cellularDataHandler_->IsRestrictedMode());
    controller.cellularDataHandler_->GetDisConnectionReason();
    ASSERT_FALSE(controller.cellularDataHandler_->HasInternetCapability(0));
    ASSERT_EQ(nullptr, controller.cellularDataHandler_->FindIdleCellularDataConnection());
    controller.cellularDataHandler_->AttemptEstablishDataConnection(apnHolder);
    controller.cellularDataHandler_->EstablishDataConnection(apnHolder, 1);
    controller.cellularDataHandler_->CheckRoamingState(apnHolder);
    controller.cellularDataHandler_->CheckApnState(apnHolder);
    ASSERT_FALSE(controller.cellularDataHandler_->CheckAttachAndSimState(apnHolder));
    controller.cellularDataHandler_->UnRegisterDataSettingObserver();
    controller.cellularDataHandler_->RemoveAllEvents();
    sleep(SLEEP_TIME_SECONDS);
}

/**
 * @tc.number   Telephony_CellularDataHandler_005
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_005, Function | MediumTest | Level1)
{
    CellularDataController controller { 0 };
    controller.Init();
    ASSERT_FALSE(controller.cellularDataHandler_ == nullptr);
    auto event = AppExecFwk::InnerEvent::Get(0);
    controller.cellularDataHandler_->RadioPsConnectionAttached(event);
    controller.cellularDataHandler_->RadioPsConnectionDetached(event);
    controller.cellularDataHandler_->RoamingStateOn(event);
    controller.cellularDataHandler_->RoamingStateOff(event);
    controller.cellularDataHandler_->EstablishDataConnectionComplete(event);
    controller.cellularDataHandler_->MsgEstablishDataConnection(event);
    controller.cellularDataHandler_->MsgRequestNetwork(event);
    controller.cellularDataHandler_->HandleSettingSwitchChanged(event);
    controller.cellularDataHandler_->HandleDBSettingIncallChanged(event);
    controller.cellularDataHandler_->HandleDefaultDataSubscriptionChanged();
    controller.cellularDataHandler_->IncallDataComplete(event);
    controller.cellularDataHandler_->HandleCallChanged(0);
    controller.cellularDataHandler_->HandleImsCallChanged(0);
    controller.cellularDataHandler_->HandleVoiceCallChanged(0);
    controller.cellularDataHandler_->HandleSimStateOrRecordsChanged(event);
    controller.cellularDataHandler_->HandleSimAccountLoaded(event);
    controller.cellularDataHandler_->HandleRadioStateChanged(event);
    controller.cellularDataHandler_->HandleDsdsModeChanged(event);
    controller.cellularDataHandler_->SetRilAttachApnResponse(event);
    controller.cellularDataHandler_->GetDefaultConfiguration();
    controller.cellularDataHandler_->HandleRadioNrStateChanged(event);
    controller.cellularDataHandler_->HandleRadioNrFrequencyChanged(event);
    controller.cellularDataHandler_->HandleDBSettingEnableChanged(event);
    controller.cellularDataHandler_->HandleDBSettingRoamingChanged(event);
    controller.cellularDataHandler_->SetDataPermittedResponse(event);
    controller.cellularDataHandler_->OnRilAdapterHostDied(event);
    controller.cellularDataHandler_->OnCleanAllDataConnectionsDone(event);
    controller.cellularDataHandler_->HandleFactoryReset(event);
    sptr<ApnHolder> apnHolder = controller.cellularDataHandler_->apnManager_->FindApnHolderById(1);
    ASSERT_FALSE(controller.cellularDataHandler_->HasAnyHigherPriorityConnection(apnHolder));
    controller.cellularDataHandler_->UnRegisterDataSettingObserver();
    controller.cellularDataHandler_->RemoveAllEvents();
    sleep(SLEEP_TIME_SECONDS);
}

/**
 * @tc.number   Telephony_CellularDataService_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataService_001, Function | MediumTest | Level3)
{
    DataAccessToken token;
    CellularDataService service;
    std::vector<std::u16string> strV;
    ASSERT_EQ(TELEPHONY_ERR_FAIL, service.Dump(INVALID_FD, strV));
    service.state_ = ServiceRunningState::STATE_RUNNING;
    service.OnStart();
    service.InitModule();
    bool dataEnabled = false;
    bool dataRoamingEnabled = false;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataEnabled(dataEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularData(false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularData(true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetCellularDataState());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataRoamingEnabled(INVALID_SLOTID, dataRoamingEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularDataRoaming(INVALID_SLOTID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataRoamingEnabled(DEFAULT_SIM_SLOT_ID, dataRoamingEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularDataRoaming(DEFAULT_SIM_SLOT_ID, true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HandleApnChanged(INVALID_SLOTID));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service.HandleApnChanged(DEFAULT_SIM_SLOT_ID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetDefaultCellularDataSlotId());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.SetDefaultCellularDataSlotId(INVALID_SLOTID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.SetDefaultCellularDataSlotId(DEFAULT_SIM_SLOT_ID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetCellularDataFlowType());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HasInternetCapability(INVALID_SLOTID, 0));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service.HasInternetCapability(DEFAULT_SIM_SLOT_ID, 0));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearCellularDataConnections(INVALID_SLOTID));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service.ClearCellularDataConnections(DEFAULT_SIM_SLOT_ID));
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearAllConnections(INVALID_SLOTID, reason));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service.ClearAllConnections(DEFAULT_SIM_SLOT_ID, reason));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ChangeConnectionForDsds(INVALID_SLOTID, false));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service.ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, false));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service.ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.StrategySwitch(INVALID_SLOTID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.StrategySwitch(DEFAULT_SIM_SLOT_ID, false));
    NetRequest request;
    request.ident = "simId12";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.RequestNet(request));
    request.ident = "simId2";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ReleaseNet(request));
    ASSERT_TRUE(service.CheckParamValid(DEFAULT_SIM_SLOT_ID));
    ASSERT_FALSE(service.CheckParamValid(INVALID_SLOTID));
    ASSERT_FALSE(service.CheckParamValid(INVALID_SLOTID_TWO));
}

/**
 * @tc.number   Telephony_CellularDataService_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataService_002, Function | MediumTest | Level3)
{
    CellularDataService service;
    std::vector<std::u16string> strV;
    ASSERT_EQ(TELEPHONY_ERR_FAIL, service.Dump(INVALID_FD, strV));
    service.state_ = ServiceRunningState::STATE_RUNNING;
    service.OnStart();
    service.InitModule();
    bool dataEnabled = false;
    bool dataRoamingEnabled = false;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataEnabled(dataEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularData(false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetCellularDataState());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataRoamingEnabled(INVALID_SLOTID, dataRoamingEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularDataRoaming(INVALID_SLOTID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HandleApnChanged(INVALID_SLOTID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetDefaultCellularDataSlotId());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.SetDefaultCellularDataSlotId(INVALID_SLOTID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetCellularDataFlowType());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HasInternetCapability(INVALID_SLOTID, 0));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearCellularDataConnections(INVALID_SLOTID));
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearAllConnections(INVALID_SLOTID, reason));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ChangeConnectionForDsds(INVALID_SLOTID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.StrategySwitch(INVALID_SLOTID, false));
    NetRequest request;
    request.ident = "simId12";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.RequestNet(request));
    request.ident = "simId2";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ReleaseNet(request));
    ASSERT_FALSE(service.CheckParamValid(INVALID_SLOTID));
    ASSERT_FALSE(service.CheckParamValid(INVALID_SLOTID_TWO));
    service.OnStop();
}

/**
 * @tc.number   Telephony_CellularDataService_003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataService_003, Function | MediumTest | Level3)
{
    DataAccessToken token;
    CellularDataService service;
    std::vector<std::u16string> strV;
    ASSERT_EQ(TELEPHONY_ERR_FAIL, service.Dump(INVALID_FD, strV));
    service.state_ = ServiceRunningState::STATE_RUNNING;
    service.OnStart();
    service.InitModule();
    service.cellularDataControllers_.clear();
    bool dataEnabled = false;
    bool dataRoamingEnabled = false;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataEnabled(dataEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularData(false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularData(true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularDataRoaming(INVALID_SLOTID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataRoamingEnabled(DEFAULT_SIM_SLOT_ID, dataRoamingEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.EnableCellularDataRoaming(DEFAULT_SIM_SLOT_ID, true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetCellularDataState());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.IsCellularDataRoamingEnabled(INVALID_SLOTID, dataRoamingEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HandleApnChanged(INVALID_SLOTID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HandleApnChanged(DEFAULT_SIM_SLOT_ID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.SetDefaultCellularDataSlotId(INVALID_SLOTID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.SetDefaultCellularDataSlotId(DEFAULT_SIM_SLOT_ID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetCellularDataFlowType());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HasInternetCapability(INVALID_SLOTID, 0));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.HasInternetCapability(DEFAULT_SIM_SLOT_ID, 0));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.GetDefaultCellularDataSlotId());
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.StrategySwitch(INVALID_SLOTID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.StrategySwitch(DEFAULT_SIM_SLOT_ID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearCellularDataConnections(INVALID_SLOTID));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearCellularDataConnections(DEFAULT_SIM_SLOT_ID));
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearAllConnections(INVALID_SLOTID, reason));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ClearAllConnections(DEFAULT_SIM_SLOT_ID, reason));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ChangeConnectionForDsds(INVALID_SLOTID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, true));
    NetRequest request;
    request.ident = "simId12";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.RequestNet(request));
    request.ident = "simId2";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ReleaseNet(request));
    ASSERT_FALSE(service.CheckParamValid(DEFAULT_SIM_SLOT_ID));
    ASSERT_FALSE(service.CheckParamValid(INVALID_SLOTID));
    ASSERT_FALSE(service.CheckParamValid(INVALID_SLOTID_TWO));
}

/**
 * @tc.number  CellularDataController_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataController_001, Function | MediumTest | Level3)
{
    CellularDataController controller { 0 };
    controller.RegisterEvents();
    controller.Init();
    controller.cellularDataHandler_ = nullptr;
    NetRequest request;
    ASSERT_FALSE(controller.ReleaseNet(request));
    ASSERT_FALSE(controller.RequestNet(request));
    ASSERT_NE(controller.SetCellularDataEnable(true), TELEPHONY_ERR_SUCCESS);
    if (controller.systemAbilityListener_ != nullptr) {
        controller.systemAbilityListener_->OnAddSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(0, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(0, "test");
    }
    bool dataEnabled = false;
    controller.IsCellularDataEnabled(dataEnabled);
    ASSERT_FALSE(dataEnabled);
    controller.SetPolicyDataOn(true);
    ASSERT_NE(controller.SetCellularDataRoamingEnabled(true), TELEPHONY_ERR_SUCCESS);
    EXPECT_EQ(ApnProfileState::PROFILE_STATE_FAILED, controller.GetCellularDataState());
    EXPECT_EQ(ApnProfileState::PROFILE_STATE_FAILED, controller.GetCellularDataState(""));
    bool dataRoamingEnabled = false;
    controller.IsCellularDataRoamingEnabled(dataRoamingEnabled);
    ASSERT_FALSE(dataRoamingEnabled);
    ASSERT_FALSE(controller.HandleApnChanged());
    auto event = AppExecFwk::InnerEvent::Get(0);
    controller.ProcessEvent(event);
    event = nullptr;
    controller.ProcessEvent(event);
    controller.RegisterEvents();
    ASSERT_FALSE(controller.IsRestrictedMode());
    ASSERT_EQ(DisConnectionReason::REASON_NORMAL, controller.GetDisConnectionReason());
    controller.HasInternetCapability(0);
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    ASSERT_FALSE(controller.ClearAllConnections(reason));
    ASSERT_FALSE(controller.ChangeConnectionForDsds(false));
    controller.UnRegisterEvents();
    ASSERT_FALSE(controller.HandleApnChanged());
    ASSERT_FALSE(controller.GetCellularDataFlowType());
}

/**
 * @tc.number  CellularDataController_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataController_002, Function | MediumTest | Level3)
{
    CellularDataController controller { 0 };
    controller.RegisterEvents();
    controller.Init();
    NetRequest request;
    controller.ReleaseNet(request);
    controller.RequestNet(request);
    controller.SetCellularDataEnable(true);
    controller.SetPolicyDataOn(true);
    ASSERT_TRUE(controller.HandleApnChanged());
    bool dataEnabled = false;
    controller.IsCellularDataEnabled(dataEnabled);
    ASSERT_TRUE(dataEnabled);
    ASSERT_NE(controller.SetCellularDataRoamingEnabled(true), TELEPHONY_ERR_SUCCESS);
    ASSERT_NE(ApnProfileState::PROFILE_STATE_FAILED, controller.GetCellularDataState());
    ASSERT_NE(ApnProfileState::PROFILE_STATE_FAILED, controller.GetCellularDataState(""));
    if (controller.systemAbilityListener_ != nullptr) {
        controller.systemAbilityListener_->OnAddSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnAddSystemAbility(0, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "test");
        controller.systemAbilityListener_->OnRemoveSystemAbility(0, "test");
    }
    bool dataRoamingEnabled = false;
    controller.IsCellularDataRoamingEnabled(dataRoamingEnabled);
    ASSERT_FALSE(dataRoamingEnabled);
    ASSERT_TRUE(controller.HandleApnChanged());
    auto event = AppExecFwk::InnerEvent::Get(0);
    controller.ProcessEvent(event);
    event = nullptr;
    controller.ProcessEvent(event);
    controller.RegisterEvents();
    ASSERT_FALSE(controller.IsRestrictedMode());
    ASSERT_EQ(DisConnectionReason::REASON_NORMAL, controller.GetDisConnectionReason());
    controller.HasInternetCapability(0);
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    ASSERT_TRUE(controller.ClearAllConnections(reason));
    controller.ChangeConnectionForDsds(false);
    ASSERT_FALSE(controller.GetCellularDataFlowType());
    controller.UnRegisterEvents();
    if (controller.cellularDataHandler_ != nullptr) {
        controller.cellularDataHandler_->UnRegisterDataSettingObserver();
        controller.cellularDataHandler_->RemoveAllEvents();
        sleep(SLEEP_TIME_SECONDS);
    }
}

/**
 * @tc.number  CellularDataConnectionManager_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataConnectionManager_001, Function | MediumTest | Level3)
{
    DataConnectionManager con { 0 };
    con.Init();
    con.connectionMonitor_ = nullptr;
    con.ccmDefaultState_ = nullptr;
    con.stateMachineEventHandler_ = nullptr;
    std::shared_ptr<CellularDataStateMachine> stateMachine = nullptr;
    con.AddConnectionStateMachine(stateMachine);
    con.RemoveConnectionStateMachine(stateMachine);
    con.AddActiveConnectionByCid(stateMachine);
    con.GetActiveConnectionByCid(1);
    con.GetAllConnectionMachine();
    con.StartStallDetectionTimer();
    con.StopStallDetectionTimer();
    con.RegisterRadioObserver();
    con.UnRegisterRadioObserver();
    con.UpdateBandWidthsUseLte();
    ASSERT_TRUE(con.GetActiveConnectionByCid(0) == nullptr);
    ASSERT_TRUE(con.isNoActiveConnection());
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    CcmDefaultState ccmDefaultState { con, "CcmDefaultState" };
    ASSERT_FALSE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_CONNECTED);
    ASSERT_TRUE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_DATA_CALL_LIST_CHANGED);
    ASSERT_TRUE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_LINK_CAPABILITY_CHANGED);
    ASSERT_TRUE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(0);
    ASSERT_FALSE(ccmDefaultState.StateProcess(event));
    ccmDefaultState.RadioDataCallListChanged(event);
    ccmDefaultState.UpdateNetworkInfo(event);
    ccmDefaultState.RadioLinkCapabilityChanged(event);
    con.GetDataFlowType();
    con.GetDefaultBandWidthsConfig();
    con.GetDefaultTcpBufferConfig();
    con.SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    con.UpdateCallState(0);
    ASSERT_EQ("", con.GetTcpBufferByRadioTech(0));
    ASSERT_TRUE(con.GetBandwidthsByRadioTech(0).upBandwidth == DEFAULT_BANDWIDTH);
}

/**
 * @tc.number  CellularDataConnectionManager_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataConnectionManager_002, Function | MediumTest | Level3)
{
    DataConnectionManager con { 0 };
    con.Init();
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> stateMachine = machine->CreateCellularDataConnect(0);
    con.AddConnectionStateMachine(stateMachine);
    con.RemoveConnectionStateMachine(stateMachine);
    con.AddActiveConnectionByCid(stateMachine);
    con.GetActiveConnectionByCid(1);
    con.GetAllConnectionMachine();
    con.StartStallDetectionTimer();
    con.StopStallDetectionTimer();
    con.RegisterRadioObserver();
    con.UnRegisterRadioObserver();
    con.uplinkUseLte_ = true;
    con.UpdateBandWidthsUseLte();
    con.GetActiveConnectionByCid(0);
    con.isNoActiveConnection();
    auto event = AppExecFwk::InnerEvent::Get(0);
    CcmDefaultState ccmDefaultState { con, "CcmDefaultState" };
    ASSERT_FALSE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_CONNECTED);
    ASSERT_TRUE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_DATA_CALL_LIST_CHANGED);
    ASSERT_TRUE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_LINK_CAPABILITY_CHANGED);
    ASSERT_TRUE(ccmDefaultState.StateProcess(event));
    event = AppExecFwk::InnerEvent::Get(0);
    ASSERT_FALSE(ccmDefaultState.StateProcess(event));
    ccmDefaultState.RadioDataCallListChanged(event);
    ccmDefaultState.UpdateNetworkInfo(event);
    ccmDefaultState.RadioLinkCapabilityChanged(event);
    con.GetDataFlowType();
    con.GetDefaultBandWidthsConfig();
    con.GetDefaultTcpBufferConfig();
    con.SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    con.UpdateCallState(0);
    ASSERT_EQ("", con.GetTcpBufferByRadioTech(0));
    ASSERT_TRUE(con.GetBandwidthsByRadioTech(0).upBandwidth == DEFAULT_BANDWIDTH);
}

/**
 * @tc.number  DataConnectionMonitor_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_DataConnectionMonitor_001, Function | MediumTest | Level3)
{
    DataConnectionManager con { 0 };
    ASSERT_FALSE(con.connectionMonitor_ == nullptr);
    con.connectionMonitor_->trafficManager_ = nullptr;
    con.connectionMonitor_->stallDetectionTrafficManager_ = nullptr;
    con.connectionMonitor_->UpdateFlowInfo();
    con.connectionMonitor_->UpdateCallState(0);
    con.connectionMonitor_->OnStallDetectionTimer();
    con.connectionMonitor_->StopStallDetectionTimer();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_CLEANUP_CONNECTIONS;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_REREGISTER_NETWORK;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_RADIO_STATUS_RESTART;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->EndNetStatistics();
    con.connectionMonitor_->UpdateNetTrafficState();
    auto event = AppExecFwk::InnerEvent::Get(0);
    con.connectionMonitor_->SetPreferredNetworkPara(event);
    con.connectionMonitor_->UpdateDataFlowType();
    con.connectionMonitor_->ProcessEvent(event);
    event = nullptr;
    con.connectionMonitor_->ProcessEvent(event);
    ASSERT_EQ(CellDataFlowType::DATA_FLOW_TYPE_NONE, con.connectionMonitor_->GetDataFlowType());
}

/**
 * @tc.number  DataConnectionMonitor_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_DataConnectionMonitor_002, Function | MediumTest | Level3)
{
    DataConnectionManager con { 0 };
    ASSERT_FALSE(con.connectionMonitor_ == nullptr);
    con.connectionMonitor_->UpdateFlowInfo();
    con.connectionMonitor_->UpdateCallState(0);
    con.connectionMonitor_->OnStallDetectionTimer();
    con.connectionMonitor_->StopStallDetectionTimer();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_CLEANUP_CONNECTIONS;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_REREGISTER_NETWORK;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->dataRecoveryState_ = RecoveryState::STATE_RADIO_STATUS_RESTART;
    con.connectionMonitor_->HandleRecovery();
    con.connectionMonitor_->EndNetStatistics();
    con.connectionMonitor_->UpdateNetTrafficState();
    auto event = AppExecFwk::InnerEvent::Get(0);
    con.connectionMonitor_->SetPreferredNetworkPara(event);
    con.connectionMonitor_->UpdateDataFlowType();
    con.connectionMonitor_->ProcessEvent(event);
    event = nullptr;
    con.connectionMonitor_->ProcessEvent(event);
    ASSERT_EQ(CellDataFlowType::DATA_FLOW_TYPE_NONE, con.connectionMonitor_->GetDataFlowType());
}

/**
 * @tc.number  CellularDataUtils_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataUtils_001, Function | MediumTest | Level3)
{
    ASSERT_EQ("unknown",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_UNKNOWN)));
    ASSERT_EQ(
        "EDGE", CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_GSM)));
    ASSERT_EQ("1xRTT",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_1XRTT)));
    ASSERT_EQ("UMTS",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_WCDMA)));
    ASSERT_EQ(
        "HSPA", CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_HSPA)));
    ASSERT_EQ("HSPAP",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_HSPAP)));
    ASSERT_EQ("TD-SCDMA",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_TD_SCDMA)));
    ASSERT_EQ(
        "EVDO", CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_EVDO)));
    ASSERT_EQ("eHRPD",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_EHRPD)));
    ASSERT_EQ(
        "LTE", CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE)));
    ASSERT_EQ("LTE_CA",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE_CA)));
    ASSERT_EQ("IWAN",
        CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_IWLAN)));
    ASSERT_EQ(
        "NR", CellularDataUtils::ConvertRadioTechToRadioName(static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_NR)));
    ASSERT_FALSE(CellularDataUtils::IsDigit(""));
    ASSERT_TRUE(CellularDataUtils::Split("", "").empty());
    AddressInfo info;
    ASSERT_TRUE(CellularDataUtils::ParseDotIpData(
        "a1.a2.a3.a4.a5.a6.a7.a8.a9.a10.a11.a12.a13.a14.a15.a16.m1.m2.m3.m4.m5.m6.m7.m8.m9.m10.m11.m12.m13.m14.m15.m16",
        info));
    ASSERT_TRUE(CellularDataUtils::ParseDotIpData("a1.a2.a3.a4.m1.m2.m3.m4", info));
    EXPECT_GE(DelayedSingleton<CellularDataClient>::GetInstance()->GetCellularDataFlowType(), 0);
    auto recipient =
        std::make_shared<CellularDataClient::CellularDataDeathRecipient>(CellularDataClient::GetInstance());
    recipient->OnRemoteDied(nullptr);
    EXPECT_GE(DelayedSingleton<CellularDataClient>::GetInstance()->GetCellularDataFlowType(), 0);
}

/**
 * @tc.number   Telephony_ApnHolder_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_ApnHolder_001, Function | MediumTest | Level3)
{
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    apnHolder->ReleaseAllCellularData();
    apnHolder->GetNextRetryApn();
    std::vector<sptr<ApnItem>> matchedApns;
    apnHolder->SetAllMatchedApns(matchedApns);
    apnHolder->GetRetryDelay();
    sptr<ApnItem> apnItem;
    apnHolder->SetCurrentApn(apnItem);
    apnHolder->GetCurrentApn();
    apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_IDLE);
    apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_FAILED);
    apnHolder->GetApnState();
    apnHolder->IsDataCallEnabled();
    apnHolder->IsDataCallConnectable();
    apnHolder->GetApnType();
    apnHolder->ReleaseDataConnection();
    apnHolder->cellularDataStateMachine_ = nullptr;
    apnHolder->ReleaseDataConnection();
    apnHolder->SetCellularDataStateMachine(apnHolder->cellularDataStateMachine_);
    apnHolder->InitialApnRetryCount();
    apnHolder->GetCellularDataStateMachine();
    apnHolder->GetCapability();
    apnHolder->GetPriority();
    apnHolder->InitialApnRetryCount();
    NetRequest request;
    request.ident = "test";
    request.capability = -1;
    apnHolder->RequestCellularData(request);
    apnHolder->ReleaseCellularData(request);
    apnHolder->RequestCellularData(request);
    apnHolder->ReleaseCellularData(request);
    ASSERT_FALSE(apnHolder->IsEmergencyType());
    ASSERT_FALSE(apnHolder->IsMmsType());
    EXPECT_GE(apnHolder->GetProfileId(DATA_CONTEXT_ROLE_DEFAULT), DATA_PROFILE_DEFAULT);
    EXPECT_GE(apnHolder->GetProfileId("test"), DATA_PROFILE_DEFAULT);
}

/**
 * @tc.number   Telephony_ApnHolder_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_ApnHolder_002, Function | MediumTest | Level3)
{
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    std::vector<sptr<ApnItem>> newMatchedApns;
    std::vector<sptr<ApnItem>> oldMatchedApns;
    apnHolder->SetAllMatchedApns(oldMatchedApns);
    ASSERT_FALSE(apnHolder->IsSameMatchedApns(newMatchedApns, true));
    sptr<ApnItem> apnItemFirst = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    newMatchedApns.push_back(apnItemFirst);
    oldMatchedApns.push_back(apnItemFirst);
    apnHolder->SetAllMatchedApns(oldMatchedApns);
    ASSERT_TRUE(apnHolder->IsSameMatchedApns(newMatchedApns, true));
    sptr<ApnItem> apnItemSecond = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    newMatchedApns.push_back(apnItemSecond);
    ASSERT_FALSE(apnHolder->IsSameMatchedApns(newMatchedApns, true));
}

/**
 * @tc.number   Telephony_ApnHolder_003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_ApnHolder_003, Function | MediumTest | Level3)
{
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    sptr<ApnItem> newApnItem;
    sptr<ApnItem> oldApnItem;
    ASSERT_FALSE(apnHolder->IsSameApnItem(newApnItem, oldApnItem, true));
    newApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    oldApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    ASSERT_TRUE(apnHolder->IsSameApnItem(newApnItem, oldApnItem, true));
    oldApnItem->CanDealWithType(DATA_CONTEXT_ROLE_DEFAULT);
    oldApnItem->GetApnTypes();
    PdpProfile apnBean;
    oldApnItem->MakeApn(apnBean);
    oldApnItem->MarkBadApn(false);
    ASSERT_FALSE(oldApnItem->IsBadApn());
}

/**
 * @tc.number   NetworkSearchCallback_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, NetworkSearchCallback_Test_01, Function | MediumTest | Level3)
{
    auto networkSearchCallback = std::make_shared<NetworkSearchCallback>();
    networkSearchCallback->ClearCellularDataConnections(0);
    networkSearchCallback->ClearCellularDataConnections(-1);
    networkSearchCallback->HasInternetCapability(0, 0);
    ASSERT_FALSE(networkSearchCallback->HasInternetCapability(-1, -1));
}

/**
 * @tc.number   StateNotification_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, StateNotification_Test_01, Function | MediumTest | Level3)
{
    StateNotification::GetInstance().UpdateCellularDataConnectState(0, PROFILE_STATE_DISCONNECTING, 0);
    StateNotification::GetInstance().OnUpDataFlowtype(0, CellDataFlowType::DATA_FLOW_TYPE_NONE);
    StateNotification::GetInstance().OnUpDataFlowtype(1, CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN);
    std::shared_ptr<CellularDataHandler> cellularDataHandler = nullptr;
    auto cellularDataRdbObserver = std::make_shared<CellularDataRdbObserver>(cellularDataHandler);
    cellularDataRdbObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);
}

/**
 * @tc.number   Active_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Active_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    std::weak_ptr<CellularDataStateMachine> stateMachine1;
    active->stateMachine_ = stateMachine1;
    active->StateBegin();
    active->StateEnd();
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    active->RefreshTcpBufferSizes();
    active->RefreshConnectionBandwidths();
    active->ProcessConnectDone(event);
    ASSERT_FALSE(active->StateProcess(event));
    ASSERT_FALSE(active->ProcessDisconnectDone(event));
    ASSERT_FALSE(active->ProcessDisconnectAllDone(event));
    ASSERT_FALSE(active->ProcessLostConnection(event));
    ASSERT_FALSE(active->ProcessRilAdapterHostDied(event));
    ASSERT_FALSE(active->ProcessLinkCapabilityChanged(event));
    ASSERT_FALSE(active->ProcessNrStateChanged(event));
    ASSERT_FALSE(active->ProcessNrFrequencyChanged(event));
    ASSERT_FALSE(active->ProcessDataConnectionComplete(event));
    ASSERT_FALSE(active->ProcessDataConnectionVoiceCallStartedOrEnded(event));
    ASSERT_FALSE(active->ProcessDataConnectionRoamOn(event));
    ASSERT_FALSE(active->ProcessDataConnectionRoamOff(event));
}

/**
 * @tc.number   Activating_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Activating_Test_02, Function | MediumTest | Level3)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    std::weak_ptr<CellularDataStateMachine> stateMachine1;
    activating->stateMachine_ = stateMachine1;
    activating->StateBegin();
    activating->StateEnd();
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    activating->ProcessConnectTimeout(event);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_RETRY);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_UNKNOWN);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_SHORTAGE_RESOURCES);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_ACTIVATION_REJECTED_UNSPECIFIED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_APN_NOT_SUPPORTED_IN_CURRENT_RAT_PLMN);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_APN_RESTRICTION_VALUE_INCOMPATIBLE);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_MULT_ACCESSES_PDN_NOT_ALLOWED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_OPERATOR_DETERMINED_BARRING);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_MISSING_OR_UNKNOWN_APN);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_UNKNOWN_PDP_ADDR_OR_TYPE);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_USER_VERIFICATION);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_ACTIVATION_REJECTED_GGSN);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_SERVICE_OPTION_NOT_SUPPORTED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_NSAPI_ALREADY_USED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_IPV4_ONLY_ALLOWED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_IPV6_ONLY_ALLOWED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_IPV4V6_ONLY_ALLOWED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_NON_IP_ONLY_ALLOWED);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_MAX_NUM_OF_PDP_CONTEXTS);
    activating->DataCallPdpError(PdpErrorReason::PDP_ERR_PROTOCOL_ERRORS);
    activating->DataCallPdpError(-1);
    ASSERT_FALSE(activating->RilActivatePdpContextDone(event));
    ASSERT_FALSE(activating->RilErrorResponse(event));
    ASSERT_FALSE(activating->StateProcess(event));
}

/**
 * @tc.number   Inactive_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Inactive_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto inactive = static_cast<Inactive *>(cellularMachine->inActiveState_.GetRefPtr());
    std::weak_ptr<CellularDataStateMachine> stateMachine1;
    inactive->SetStateMachine(stateMachine1);
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    inactive->StateBegin();
    inactive->StateEnd();
    inactive->SetDeActiveApnTypeId(0);
    inactive->SetReason(DisConnectionReason::REASON_NORMAL);
    ASSERT_FALSE(inactive->StateProcess(event));
}

/**
 * @tc.number   Disconnecting_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Disconnecting_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    std::weak_ptr<CellularDataStateMachine> stateMachine1;
    disconnecting->stateMachine_ = stateMachine1;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    disconnecting->StateBegin();
    disconnecting->StateEnd();
    disconnecting->ProcessDisconnectTimeout(event);
    disconnecting->ProcessRilAdapterHostDied(event);
    ASSERT_FALSE(disconnecting->StateProcess(event));
}

/**
 * @tc.number   Default_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Default_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    std::weak_ptr<CellularDataStateMachine> stateMachine1;
    mDefault->stateMachine_ = stateMachine1;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    mDefault->StateBegin();
    mDefault->StateEnd();
    ASSERT_FALSE(mDefault->StateProcess(event));
    ASSERT_FALSE(mDefault->ProcessConnectDone(event));
    ASSERT_FALSE(mDefault->ProcessDisconnectDone(event));
    ASSERT_FALSE(mDefault->ProcessDisconnectAllDone(event));
    ASSERT_FALSE(mDefault->ProcessDataConnectionDrsOrRatChanged(event));
    ASSERT_FALSE(mDefault->ProcessDataConnectionRoamOn(event));
    ASSERT_FALSE(mDefault->ProcessDataConnectionRoamOff(event));
}

/**
 * @tc.number   ApnManager_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, ApnManager_Test_01, Function | MediumTest | Level3)
{
    auto apnManager = std::make_shared<ApnManager>();
    apnManager->CreateAllApnItem();
    EXPECT_GE(apnManager->CreateAllApnItemByDatabase(0), 0);
    EXPECT_EQ(apnManager->CreateAllApnItemByDatabase(0), 0);
    apnManager->ResetApns(0);
    std::string operatorNumeric = "46011";
    apnManager->GetCTOperator(0, operatorNumeric);
    EXPECT_EQ(operatorNumeric, "46011");
    apnManager->GetApnHolder(DATA_CONTEXT_ROLE_DEFAULT);
    apnManager->FindApnNameByApnId(1);
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    ASSERT_TRUE(apnManager->IsDataConnectionNotUsed(cellularMachine));
    auto helper = CellularDataRdbHelper::GetInstance();
    helper->ResetApns(0);
    std::shared_ptr<DataShare::DataShareResultSet> result = nullptr;
    std::vector<PdpProfile> apnVec;
    helper->ReadMvnoApnResult(result, "", apnVec);
    PdpProfile apnBean;
    ASSERT_FALSE(helper->IsMvnoDataMatched("", apnBean));
    apnBean.mvnoType = MvnoType::ICCID;
    apnBean.mvnoMatchData = "test";
    ASSERT_TRUE(helper->IsMvnoDataMatched("test", apnBean));
    apnBean.mvnoType = MvnoType::SPN;
    ASSERT_TRUE(helper->IsMvnoDataMatched("test", apnBean));
    apnBean.mvnoType = MvnoType::GID1;
    ASSERT_TRUE(helper->IsMvnoDataMatched("test", apnBean));
    apnBean.mvnoType = MvnoType::IMSI;
    ASSERT_TRUE(helper->IsMvnoDataMatched("test", apnBean));
    apnBean.mvnoType = "error";
    ASSERT_FALSE(helper->IsMvnoDataMatched("test", apnBean));
    helper->QueryPreferApn(0, apnVec);
    ASSERT_FALSE(helper->QueryPreferApn(-1, apnVec));
}

/**
 * @tc.number   IdleState_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Idle_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<IncallStateMachineTest> incallStateMachineTest = std::make_shared<IncallStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->GetCurrentState();
    incallStateMachine->GetSlotId();
    incallStateMachine->GetCallState();
    incallStateMachine->HasAnyConnectedState();
    incallStateMachine->UpdateCallState(TelCallStatus::CALL_STATUS_ALERTING);
    incallStateMachine->IsIncallDataSwitchOn();
    incallStateMachine->IsSecondaryCanActiveData();
    incallStateMachine->CanActiveDataByRadioTech();
    auto idleState = static_cast<IdleState *>(incallStateMachine->idleState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    idleState->StateBegin();
    ASSERT_FALSE(idleState->StateProcess(event));
    ASSERT_TRUE(idleState->ProcessCallStarted(event));
    ASSERT_TRUE(idleState->ProcessCallEnded(event));
    ASSERT_TRUE(idleState->ProcessSettingsOn(event));
    ASSERT_TRUE(idleState->ProcessDsdsChanged(event));
    idleState->StateEnd();
}

/**
 * @tc.number   ActivatingSecondaryState_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, ActivatingSecondaryState_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<IncallStateMachineTest> incallStateMachineTest = std::make_shared<IncallStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    auto activatingSecondaryState =
        static_cast<ActivatingSecondaryState *>(incallStateMachine->activatingSecondaryState_.GetRefPtr());
    auto secondaryActiveState =
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    secondaryActiveState->StateBegin();
    activatingSecondaryState->StateBegin();
    ASSERT_FALSE(activatingSecondaryState->StateProcess(event));
    ASSERT_FALSE(secondaryActiveState->StateProcess(event));
    ASSERT_TRUE(secondaryActiveState->ProcessSettingsOn(event));
    ASSERT_TRUE(secondaryActiveState->ProcessCallEnded(event));
    ASSERT_TRUE(secondaryActiveState->ProcessSettingsOff(event));
    ASSERT_TRUE(secondaryActiveState->ProcessDsdsChanged(event));
    activatingSecondaryState->StateEnd();
    secondaryActiveState->StateEnd();
}

/**
 * @tc.number   ActivatedSecondaryState_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, ActivatedSecondaryState_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<IncallStateMachineTest> incallStateMachineTest = std::make_shared<IncallStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    auto activatedSecondaryState =
        static_cast<ActivatedSecondaryState *>(incallStateMachine->activatedSecondaryState_.GetRefPtr());
    auto secondaryActiveState =
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    secondaryActiveState->StateBegin();
    activatedSecondaryState->StateBegin();
    ASSERT_FALSE(activatedSecondaryState->StateProcess(event));
    ASSERT_FALSE(secondaryActiveState->StateProcess(event));
    ASSERT_TRUE(secondaryActiveState->ProcessCallEnded(event));
    ASSERT_TRUE(secondaryActiveState->ProcessSettingsOff(event));
    activatedSecondaryState->StateEnd();
    secondaryActiveState->StateEnd();
}

/**
 * @tc.number   DeactivatingSecondaryState_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, DeactivatingSecondaryState_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<IncallStateMachineTest> incallStateMachineTest = std::make_shared<IncallStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->deactivatingSecondaryState_);
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->deactivatingSecondaryState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    deactivatingSecondaryState->StateBegin();
    ASSERT_FALSE(deactivatingSecondaryState->StateProcess(event));
    deactivatingSecondaryState->StateEnd();
}

/**
 * @tc.number   GetIpType_Test_01
 * @tc.name     TestDump
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, GetIpType_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    std::string result = "";
    std::string address = "";
    std::vector<AddressInfo> ipInfoArray = CellularDataUtils::ParseIpAddr(address);
    result = cellularMachine->GetIpType(ipInfoArray);
    ASSERT_TRUE(result == "");
}

/**
 * @tc.number   DataSwitchSettings_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, DataSwitchSettings_Test_01, Function | MediumTest | Level3)
{
    std::unique_ptr<DataSwitchSettings> dataSwitchSettings = std::make_unique<DataSwitchSettings>(0);
    dataSwitchSettings->LoadSwitchValue();
    bool status = true;
    dataSwitchSettings->QueryUserDataStatus(status);
    dataSwitchSettings->QueryUserDataRoamingStatus(status);
    dataSwitchSettings->SetPolicyDataOn(true);
    ASSERT_TRUE(dataSwitchSettings->IsPolicyDataOn());
    dataSwitchSettings->IsAllowActiveData();
    dataSwitchSettings->SetUserDataOn(true);
    dataSwitchSettings->IsUserDataOn();
    dataSwitchSettings->SetCarrierDataOn(true);
    ASSERT_TRUE(dataSwitchSettings->IsCarrierDataOn());
    dataSwitchSettings->SetUserDataRoamingOn(true);
    dataSwitchSettings->IsUserDataRoamingOn();
    dataSwitchSettings->SetInternalDataOn(true);
    ASSERT_TRUE(dataSwitchSettings->IsInternalDataOn());
}

/**
 * @tc.number   CellularDataStateMachine_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, CellularDataStateMachine_Test_01, Function | MediumTest | Level3)
{
    std::shared_ptr<StateMachineTest> machine = std::make_shared<StateMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->IsInactiveState();
    uint64_t capability = 1;
    cellularMachine->SetCapability(capability);
    cellularMachine->GetCapability();
    const int32_t cid = 1;
    cellularMachine->SetCid(cid);
    cellularMachine->GetSlotId();
    cellularMachine->GetApnItem();
    cellularMachine->GetCurrentState();
    const uint32_t upBandwidth = 0;
    const uint32_t downBandwidth = 0;
    cellularMachine->SetConnectionBandwidth(upBandwidth, downBandwidth);
    const std::string tcpBuffer = "";
    cellularMachine->SetConnectionTcpBuffer(tcpBuffer);
    std::string proxyIpAddress = "1234:567";
    std::string host = "";
    uint16_t port = 0;
    cellularMachine->SplitProxyIpAddress(proxyIpAddress, host, port);
    EXPECT_EQ(host, "1234");
    EXPECT_EQ(port, 567);
    EXPECT_TRUE(cellularMachine != nullptr);
    CellularDataDumpHelper dumpHelper;
    std::string result = "";
    dumpHelper.ShowHelp(result);
    EXPECT_GE(result.size(), 0);
}

/**
 * @tc.number   CellularDataUtils_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, CellularDataUtils_Test_01, Function | MediumTest | Level3)
{
    CellularDataUtils::ParseNormalIpAddr(ADDRESS);
    CellularDataUtils::ParseRoute(ADDRESS);
    CellularDataUtils::GetPrefixLen(ADDRESS, FLAG);
    auto cellularDataHiSysEvent = DelayedSingleton<CellularDataHiSysEvent>::GetInstance();
    cellularDataHiSysEvent->WriteRoamingConnectStateBehaviorEvent(1);
    cellularDataHiSysEvent->SetCellularDataActivateStartTime();
    cellularDataHiSysEvent->JudgingDataActivateTimeOut(0, 1);
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    dataConnectionMonitor->HandleRecovery();
    dataConnectionMonitor->GetPdpContextList();
    dataConnectionMonitor->SetRadioState(0, RadioEvent::RADIO_ON);
    dataConnectionMonitor->GetPreferredNetworkPara();
    dataConnectionMonitor->GetDataFlowType();
    auto cellularDataService = DelayedSingleton<CellularDataService>::GetInstance();
    MessageParcel data;
    MessageParcel reply;
    EXPECT_GE(cellularDataService->OnIsCellularDataEnabled(data, reply), 0);
    EXPECT_GE(cellularDataService->OnEnableCellularData(data, reply), 0);
    EXPECT_GE(cellularDataService->OnGetCellularDataState(data, reply), 0);
    EXPECT_GE(cellularDataService->OnIsCellularDataRoamingEnabled(data, reply), 0);
    EXPECT_GE(cellularDataService->OnEnableCellularDataRoaming(data, reply), 0);
    EXPECT_GE(cellularDataService->OnHandleApnChanged(data, reply), 0);
    cellularDataService->OnGetDefaultCellularDataSlotId(data, reply);
    EXPECT_GE(cellularDataService->OnGetDefaultCellularDataSimId(data, reply), 0);
    EXPECT_GE(cellularDataService->OnSetDefaultCellularDataSlotId(data, reply), 0);
    EXPECT_GE(cellularDataService->OnGetCellularDataFlowType(data, reply), 0);
    EXPECT_GE(cellularDataService->OnHasInternetCapability(data, reply), 0);
    EXPECT_GE(cellularDataService->OnClearCellularDataConnections(data, reply), 0);
    EXPECT_GE(cellularDataService->OnRegisterSimAccountCallback(data, reply), 0);
    EXPECT_GE(cellularDataService->OnUnregisterSimAccountCallback(data, reply), 0);
}

/**
 * @tc.number   CellularDataSettingsRdbHelper_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, CellularDataSettingsRdbHelper_Test_01, Function | MediumTest | Level3)
{
    auto settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("settingHelper is null");
        return;
    }
    Uri dataEnableUri(CELLULAR_DATA_SETTING_DATA_ROAMING_URI);
    settingHelper->RegisterSettingsObserver(dataEnableUri, nullptr);
    settingHelper->UnRegisterSettingsObserver(dataEnableUri, nullptr);
    EXPECT_TRUE(settingHelper != nullptr);
}

/**
 * @tc.number   FindBestCapability_Test_01
 * @tc.name     test branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, FindBestCapability_Test_01, Function | MediumTest | Level3)
{
    auto apnManager = std::make_shared<ApnManager>();

    uint64_t capabilities = 1L << NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET;
    auto ret = apnManager->FindBestCapability(capabilities);
    EXPECT_EQ(ret, NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);

    capabilities |= 1L << NetManagerStandard::NetCap::NET_CAPABILITY_INTERNAL_DEFAULT;
    ret = apnManager->FindBestCapability(capabilities);
    EXPECT_EQ(ret, NetManagerStandard::NetCap::NET_CAPABILITY_INTERNAL_DEFAULT);

    capabilities |= 1L << NetManagerStandard::NetCap::NET_CAPABILITY_MMS;
    ret = apnManager->FindBestCapability(capabilities);
    EXPECT_EQ(ret, NetManagerStandard::NetCap::NET_CAPABILITY_MMS);
}

/**
 * @tc.number   GetOverallDefaultApnState_Test_01
 * @tc.name     test branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, GetOverallDefaultApnState_Test_01, Function | MediumTest | Level3)
{
    auto apnManager = std::make_shared<ApnManager>();
    apnManager->InitApnHolders();
    auto &apnHolders = apnManager->apnHolders_;
    for (auto &apnHolder : apnHolders) {
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_DEFAULT) {
            apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_IDLE);
        }
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
            apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_IDLE);
        }
    }
    auto ret = apnManager->GetOverallDefaultApnState();
    EXPECT_EQ(ret, ApnProfileState::PROFILE_STATE_IDLE);

    for (auto &apnHolder : apnHolders) {
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_DEFAULT) {
            apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_CONNECTING);
        }
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
            apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_IDLE);
        }
    }
    ret = apnManager->GetOverallDefaultApnState();
    EXPECT_EQ(ret, ApnProfileState::PROFILE_STATE_CONNECTING);

    for (auto &apnHolder : apnHolders) {
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_DEFAULT) {
            apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_IDLE);
        }
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
            apnHolder->SetApnState(ApnProfileState::PROFILE_STATE_CONNECTING);
        }
    }
    ret = apnManager->GetOverallDefaultApnState();
    EXPECT_EQ(ret, ApnProfileState::PROFILE_STATE_CONNECTING);
}
} // namespace Telephony
} // namespace OHOS
