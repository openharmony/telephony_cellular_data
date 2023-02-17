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
#include "apn_holder.h"
#include "apn_item.h"
#include "cellular_data_controller.h"
#include "cellular_data_dump_helper.h"
#include "cellular_data_handler.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_rdb_observer.h"
#include "cellular_data_roaming_observer.h"
#include "cellular_data_service.h"
#include "cellular_data_setting_observer.h"
#include "cellular_data_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "data_connection_manager.h"
#include "data_connection_monitor.h"
#include "gtest/gtest.h"
#include "net_manager_call_back.h"
#include "net_manager_tactics_call_back.h"
#include "network_search_callback.h"
#include "telephony_errors.h"
#include "telephony_hisysevent.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

namespace {
const int32_t INVALID_SLOTID = -1;
const int32_t INVALID_SLOTID_TWO = 5;
const int32_t INVALID_CID = -1;
const int32_t INVALID_FD = -1;
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

/**
 * @tc.number   Telephony_CellularDataHandler_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_001, Function | MediumTest | Level1)
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("test");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    CellularDataHandler cellularDataHandler { runner, subscriberInfo, 0 };
    NetRequest request;
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
}

/**
 * @tc.number   Telephony_CellularDataHandler_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_002, Function | MediumTest | Level1)
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("test");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    CellularDataHandler cellularDataHandler { runner, subscriberInfo, 0 };
    NetRequest request;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    sptr<ApnHolder> apnHolder;
    cellularDataHandler.RadioPsConnectionAttached(event);
    cellularDataHandler.RadioPsConnectionDetached(event);
    cellularDataHandler.RoamingStateOn(event);
    cellularDataHandler.RoamingStateOff(event);
    cellularDataHandler.EstablishDataConnectionComplete(event);
    cellularDataHandler.DisconnectDataComplete(event);
    cellularDataHandler.MsgEstablishDataConnection(event);
    cellularDataHandler.MsgRequestNetwork(event);
    cellularDataHandler.HandleSettingSwitchChanged(event);
    cellularDataHandler.HandleVoiceCallChanged(0);
    cellularDataHandler.HandleSimStateOrRecordsChanged(event);
    cellularDataHandler.HandleSimAccountLoaded(event);
    cellularDataHandler.HandleRadioStateChanged(event);
    cellularDataHandler.SetRilAttachApnResponse(event);
    cellularDataHandler.GetDefaultConfiguration();
    cellularDataHandler.HandleRadioNrStateChanged(event);
    cellularDataHandler.HandleRadioNrFrequencyChanged(event);
    cellularDataHandler.HandleDBSettingEnableChanged(event);
    cellularDataHandler.HandleDBSettingRoamingChanged(event);
    cellularDataHandler.SetDataPermittedResponse(event);
    cellularDataHandler.RegisterDataSettingObserver();
    cellularDataHandler.UnRegisterDataSettingObserver();
    ASSERT_FALSE(cellularDataHandler.HasAnyHigherPriorityConnection(apnHolder));
}

/**
 * @tc.number   Telephony_CellularDataHandler_003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataHandler_003, Function | MediumTest | Level3)
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("test");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    CellularDataHandler cellularDataHandler { runner, subscriberInfo, INVALID_SLOTID };
    cellularDataHandler.apnManager_ = std::make_unique<ApnManager>().release();
    NetRequest request;
    ASSERT_FALSE(cellularDataHandler.ReleaseNet(request));
    sptr<ApnHolder> apnHolder = new ApnHolder("", 0);
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    cellularDataHandler.ClearConnection(apnHolder, reason);
    cellularDataHandler.EstablishAllApnsIfConnectable();
    cellularDataHandler.ClearAllConnections(reason);
    cellularDataHandler.connectionManager_ = std::make_unique<DataConnectionManager>(runner, INVALID_SLOTID).release();
    cellularDataHandler.ClearAllConnections(reason);
    cellularDataHandler.EstablishAllApnsIfConnectable();
    ASSERT_FALSE(cellularDataHandler.CheckApnState(apnHolder));
    cellularDataHandler.AttemptEstablishDataConnection(apnHolder);
    cellularDataHandler.connectionManager_ = nullptr;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    cellularDataHandler.MsgEstablishDataConnection(event);
    ASSERT_FALSE(cellularDataHandler.HasAnyHigherPriorityConnection(apnHolder));
    cellularDataHandler.connectionManager_ = std::make_unique<DataConnectionManager>(runner, INVALID_SLOTID).release();
    ASSERT_FALSE(cellularDataHandler.HasInternetCapability(INVALID_CID));
}

/**
 * @tc.number   Telephony_CellularDataService_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataService_001, Function | MediumTest | Level3)
{
    CellularDataService service;
    std::vector<std::u16string> strV;
    ASSERT_EQ(TELEPHONY_ERR_FAIL, service.Dump(INVALID_FD, strV));
    service.state_ = ServiceRunningState::STATE_RUNNING;
    service.OnStart();
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
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.StrategySwitch(INVALID_SLOTID, false));
    NetRequest request;
    request.ident = "simId12";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.RequestNet(request));
    request.ident = "simId2";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service.ReleaseNet(request));
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
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("test");
    CellularDataController controller { runner, 0 };
    controller.RegisterEvents();
    controller.Init();
    controller.cellularDataHandler_ = nullptr;
    controller.cellularDataRdbObserver_ = nullptr;
    NetRequest request;
    ASSERT_FALSE(controller.ReleaseNet(request));
    ASSERT_FALSE(controller.RequestNet(request));
    ASSERT_NE(controller.SetCellularDataEnable(true), TELEPHONY_ERR_SUCCESS);
    bool dataEnabled = false;
    controller.IsCellularDataEnabled(dataEnabled);
    ASSERT_FALSE(dataEnabled);
    ASSERT_NE(controller.SetCellularDataRoamingEnabled(true), TELEPHONY_ERR_SUCCESS);
    EXPECT_EQ(ApnProfileState::PROFILE_STATE_FAILED, controller.GetCellularDataState());
    EXPECT_EQ(ApnProfileState::PROFILE_STATE_FAILED, controller.GetCellularDataState(""));
    bool dataRoamingEnabled = false;
    controller.IsCellularDataRoamingEnabled(dataRoamingEnabled);
    ASSERT_FALSE(dataRoamingEnabled);
    controller.SetDataPermitted(true);
    ASSERT_FALSE(controller.HandleApnChanged());
    controller.EstablishDataConnection();
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
    controller.UnRegisterEvents();
    ASSERT_FALSE(controller.HandleApnChanged());
    ASSERT_FALSE(controller.GetCellularDataFlowType());
}

/**
 * @tc.number  CellularDataConnectionManager_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_CellularDataConnectionManager_001, Function | MediumTest | Level3)
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("test");
    DataConnectionManager con { runner, 0 };
    con.connectionMonitor_ = nullptr;
    con.ccmDefaultState_ = nullptr;
    std::shared_ptr<CellularDataStateMachine> stateMachine = nullptr;
    con.RemoveConnectionStateMachine(stateMachine);
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
    event = AppExecFwk::InnerEvent::Get(0);
    ASSERT_FALSE(ccmDefaultState.StateProcess(event));
    ccmDefaultState.RadioDataCallListChanged(event);
    ccmDefaultState.UpdateNetworkInfo(event);
    con.GetDataFlowType();
    con.SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_NONE);
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
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("test");
    DataConnectionMonitor mon { runner, 0 };
    mon.trafficManager_ = nullptr;
    mon.stallDetectionTrafficManager_ = nullptr;
    mon.UpdateFlowInfo();
    auto event = AppExecFwk::InnerEvent::Get(0);
    mon.SetPreferredNetworkPara(event);
    mon.UpdateDataFlowType();
    mon.ProcessEvent(event);
    event = nullptr;
    mon.ProcessEvent(event);
    ASSERT_EQ(CellDataFlowType::DATA_FLOW_TYPE_NONE, mon.GetDataFlowType());
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
}

} // namespace Telephony
} // namespace OHOS
