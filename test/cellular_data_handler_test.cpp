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

#include "gtest/gtest.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "cellular_data_handler.h"
#include "cellular_data_controller.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class CellularDataHandlerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number   HandleUpdateNetInfo_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleUpdateNetInfo_001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    auto netInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DATA_CALL_LIST_CHANGED, netInfo);
    cellularDataHandler->HandleUpdateNetInfo(event);
    EXPECT_EQ(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_EQ(cellularDataHandler->apnManager_, nullptr);
}

/**
 * @tc.number   HandleUpdateNetInfo_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleUpdateNetInfo_002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto netInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DATA_CALL_LIST_CHANGED);
    cellularDataHandler->HandleUpdateNetInfo(event);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_EQ(event->GetSharedObject<SetupDataCallResultInfo>(), nullptr);
}

/**
 * @tc.number   HandleUpdateNetInfo_003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleUpdateNetInfo_003, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto netInfo = std::make_shared<SetupDataCallResultInfo>();
    netInfo->flag = DATA_CONTEXT_ROLE_DEFAULT_ID;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DATA_CALL_LIST_CHANGED, netInfo);
    cellularDataHandler->HandleUpdateNetInfo(event);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_NE(event->GetSharedObject<SetupDataCallResultInfo>(), nullptr);
    sptr<ApnHolder> apnHolder = cellularDataHandler->apnManager_->GetApnHolder(
        cellularDataHandler->apnManager_->FindApnNameByApnId(netInfo->flag));
    EXPECT_NE(apnHolder, nullptr);
    EXPECT_EQ(apnHolder->GetApnState(), PROFILE_STATE_IDLE);
}

/**
 * @tc.number   HandleUpdateNetInfo_004
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleUpdateNetInfo_004, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto netInfo = std::make_shared<SetupDataCallResultInfo>();
    netInfo->flag = DATA_CONTEXT_ROLE_DEFAULT_ID;
    sptr<ApnHolder> apnHolder = cellularDataHandler->apnManager_->GetApnHolder(
        cellularDataHandler->apnManager_->FindApnNameByApnId(netInfo->flag));
    EXPECT_NE(apnHolder, nullptr);
    apnHolder->SetApnState(PROFILE_STATE_CONNECTED);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DATA_CALL_LIST_CHANGED, netInfo);
    cellularDataHandler->HandleUpdateNetInfo(event);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_NE(event->GetSharedObject<SetupDataCallResultInfo>(), nullptr);
    EXPECT_EQ(apnHolder->GetApnState(), PROFILE_STATE_CONNECTED);
    auto stateMachine = cellularDataHandler->connectionManager_->GetActiveConnectionByCid(netInfo->cid);
    EXPECT_EQ(stateMachine, nullptr);
}

/**
 * @tc.number   HandleUpdateNetInfo_005
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleUpdateNetInfo_005, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto netInfo = std::make_shared<SetupDataCallResultInfo>();
    netInfo->flag = DATA_CONTEXT_ROLE_DEFAULT_ID;
    netInfo->cid = 100;
    sptr<ApnHolder> apnHolder = cellularDataHandler->apnManager_->GetApnHolder(
        cellularDataHandler->apnManager_->FindApnNameByApnId(netInfo->flag));
    EXPECT_NE(apnHolder, nullptr);
    apnHolder->SetApnState(PROFILE_STATE_CONNECTED);
    sptr<DataConnectionManager> connectionManager = std::make_unique<DataConnectionManager>(0).release();
    EXPECT_NE(connectionManager, nullptr);
    connectionManager->Init();
    auto sm = std::make_shared<CellularDataStateMachine>(
        connectionManager, std::make_shared<TelEventHandler>("CellularDataHandlerTest"));
    sm->cid_ = 100;
    cellularDataHandler->connectionManager_->AddActiveConnectionByCid(sm);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DATA_CALL_LIST_CHANGED, netInfo);
    cellularDataHandler->HandleUpdateNetInfo(event);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_NE(event->GetSharedObject<SetupDataCallResultInfo>(), nullptr);
    EXPECT_EQ(apnHolder->GetApnState(), PROFILE_STATE_CONNECTED);
    auto stateMachine = cellularDataHandler->connectionManager_->GetActiveConnectionByCid(netInfo->cid);
    EXPECT_NE(stateMachine, nullptr);
}

/**
 * @tc.number   HandleUpdateNetInfo_006
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleUpdateNetInfo_006, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto netInfo = std::make_shared<SetupDataCallResultInfo>();
    netInfo->flag = DATA_CONTEXT_ROLE_DEFAULT_ID;
    netInfo->cid = 100;
    sptr<ApnHolder> apnHolder = cellularDataHandler->apnManager_->GetApnHolder(
        cellularDataHandler->apnManager_->FindApnNameByApnId(netInfo->flag));
    EXPECT_NE(apnHolder, nullptr);
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    cellularDataHandler->ClearConnection(apnHolder, DisConnectionReason::REASON_CLEAR_CONNECTION);
    EXPECT_NE(apnHolder->GetApnState(), PROFILE_STATE_DISCONNECTING);
}

/**
 * @tc.number   HandleRoamingOff_001
 * @tc.name     test roaming off
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleRoamingOff_001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    EXPECT_NE(cellularDataHandler, nullptr);
    cellularDataHandler->Init();
    cellularDataHandler->SetCellularDataRoamingEnabled(true);
    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler->RoamingStateOff(event);
}

/**
 * @tc.number   HandleRoamingOff_002
 * @tc.name     test roaming off
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleRoamingOff_002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    EXPECT_NE(cellularDataHandler, nullptr);
    cellularDataHandler->Init();
    cellularDataHandler->SetCellularDataRoamingEnabled(false);
    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler->RoamingStateOff(event);
}

/**
 * @tc.number   HandleEstablishAllApnsIfConnectable_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleEstablishAllApnsIfConnectable_001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_ESTABLISH_ALL_APNS_IF_CONNECTABLE);
    cellularDataHandler->HandleEstablishAllApnsIfConnectable(msgEvent);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
}

/**
 * @tc.number   HandleSimEvent_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleSimEvent_001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    AppExecFwk::InnerEvent::Pointer nullEvent(nullptr, nullptr);
    cellularDataHandler->HandleSimEvent(nullEvent);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DATA_CALL_LIST_CHANGED, 0);
    cellularDataHandler->HandleSimEvent(event);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
}

/**
 * @tc.number   HandleSimEvent_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleSimEvent_002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_SIM_STATE_CHANGE, 1);
    cellularDataHandler->HandleSimEvent(event);
    EXPECT_NE(cellularDataHandler->slotId_, event->GetParam());
}

/**
 * @tc.number   HandleSimEvent_003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleSimEvent_003, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_SIM_STATE_CHANGE, 0);
    cellularDataHandler->HandleSimEvent(event);
    EXPECT_EQ(event->GetInnerEventId(), RadioEvent::RADIO_SIM_STATE_CHANGE);
}

/**
 * @tc.number   HandleSimEvent_004
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleSimEvent_004, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_SIM_RECORDS_LOADED, 0);
    cellularDataHandler->HandleSimEvent(event);
    EXPECT_EQ(event->GetInnerEventId(), RadioEvent::RADIO_SIM_RECORDS_LOADED);
}

/**
 * @tc.number   HandleSimEvent_005
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleSimEvent_005, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_NV_REFRESH_FINISHED, 0);
    cellularDataHandler->HandleSimEvent(event);
    EXPECT_EQ(event->GetInnerEventId(), RadioEvent::RADIO_NV_REFRESH_FINISHED);
}

/**
 * @tc.number   HandleSimEvent_006
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleSimEvent_006, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_SIM_ACCOUNT_LOADED, 0);
    cellularDataHandler->HandleSimEvent(event);
    EXPECT_EQ(event->GetInnerEventId(), RadioEvent::RADIO_SIM_ACCOUNT_LOADED);
}

/**
 * @tc.number   ClearConnectionsOnUpdateApns_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, ClearConnectionsOnUpdateApns_001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->ClearConnectionsOnUpdateApns(DisConnectionReason::REASON_RETRY_CONNECTION);
    EXPECT_EQ(cellularDataHandler->apnManager_, nullptr);
}

/**
 * @tc.number   ClearConnectionsOnUpdateApns_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, ClearConnectionsOnUpdateApns_002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    cellularDataHandler->ClearConnectionsOnUpdateApns(DisConnectionReason::REASON_RETRY_CONNECTION);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    cellularDataHandler->apnManager_->allApnItem_.clear();
    cellularDataHandler->ClearConnectionsOnUpdateApns(DisConnectionReason::REASON_RETRY_CONNECTION);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    std::vector<sptr<ApnItem>> allApnItem;
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    allApnItem.push_back(defaultApnItem);
    cellularDataHandler->apnManager_->allApnItem_ = allApnItem;
    EXPECT_NE(cellularDataHandler->apnManager_->GetRilAttachApn(), nullptr);
    cellularDataHandler->ClearConnectionsOnUpdateApns(DisConnectionReason::REASON_RETRY_CONNECTION);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
}

/**
 * @tc.number   CellularDataHandler_Uid_Test001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, CellularDataHandler_Uid_Test001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    NetRequest netRequest;
    netRequest.capability = 1;
    netRequest.ident = "ident";
    EXPECT_FALSE(cellularDataHandler->AddUid(netRequest));
    EXPECT_FALSE(cellularDataHandler->RemoveUid(netRequest));
    cellularDataHandler->Init();
    EXPECT_TRUE(cellularDataHandler->AddUid(netRequest));
    EXPECT_TRUE(cellularDataHandler->RemoveUid(netRequest));
}

/**
 * @tc.number   SetCellularDataRoamingEnabledTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, SetCellularDataRoamingEnabledTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    bool dataRoamingEnabled = true;
    EXPECT_EQ(cellularDataHandler->SetCellularDataRoamingEnabled(dataRoamingEnabled), TELEPHONY_ERR_LOCAL_PTR_NULL);
    cellularDataHandler->Init();
    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler->RadioPsConnectionAttached(event);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    cellularDataHandler->RoamingStateOn(event);
    cellularDataHandler->RoamingStateOff(event);
    EXPECT_NE(cellularDataHandler->dataSwitchSettings_, nullptr);
    cellularDataHandler->incallDataStateMachine_ = cellularDataHandler->CreateIncallDataStateMachine(0);
    EXPECT_FALSE(cellularDataHandler->SetDataPermittedForMms(true));
}

/**
 * @tc.number   AttemptEstablishDataConnectionTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, AttemptEstablishDataConnectionTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = nullptr;
    cellularDataHandler->connectionManager_->GetAllConnectionMachine().push_back(cellularMachine);
    EXPECT_EQ(cellularDataHandler->FindIdleCellularDataConnection(), nullptr);
}

/**
 * @tc.number   AttemptEstablishDataConnectionTest002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, AttemptEstablishDataConnectionTest002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    sptr<DataConnectionManager> connectionManager = std::make_unique<DataConnectionManager>(0).release();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = std::make_shared<CellularDataStateMachine>(
        connectionManager, nullptr);
    cellularDataHandler->connectionManager_ = connectionManager;
    cellularDataHandler->connectionManager_->GetAllConnectionMachine().push_back(cellularMachine);
    EXPECT_EQ(cellularDataHandler->FindIdleCellularDataConnection(), nullptr);
    cellularDataHandler->Init();
    EXPECT_EQ(cellularDataHandler->FindIdleCellularDataConnection(), nullptr);
}

/**
 * @tc.number   EstablishDataConnectionTest002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, EstablishDataConnectionTest002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    sptr<ApnItem> apnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder->retryPolicy_.matchedApns_.push_back(apnItem);
    EXPECT_NE(apnHolder->GetNextRetryApn(), nullptr);
    EXPECT_TRUE(cellularDataHandler->EstablishDataConnection(apnHolder, 0));
    apnHolder->apnType_ = DATA_CONTEXT_ROLE_DUN;
    EXPECT_TRUE(cellularDataHandler->EstablishDataConnection(apnHolder, 0));
    apnHolder->apnType_ = DATA_CONTEXT_ROLE_MMS;
    EXPECT_TRUE(cellularDataHandler->EstablishDataConnection(apnHolder, 0));
}

/**
 * @tc.number   DisconnectDataCompleteTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, DisconnectDataCompleteTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler->DisconnectDataComplete(event);
    EXPECT_NE(event, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
}

/**
 * @tc.number   UpdatePhysicalConnectionStateTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, UpdatePhysicalConnectionStateTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->physicalConnectionActiveState_ = false;
    cellularDataHandler->UpdatePhysicalConnectionState(false);
    EXPECT_TRUE(cellularDataHandler->physicalConnectionActiveState_);

    cellularDataHandler->UpdatePhysicalConnectionState(true);
    EXPECT_FALSE(cellularDataHandler->physicalConnectionActiveState_);

    cellularDataHandler->physicalConnectionActiveState_ = true;
    cellularDataHandler->UpdatePhysicalConnectionState(false);
    EXPECT_TRUE(cellularDataHandler->physicalConnectionActiveState_);

    cellularDataHandler->UpdatePhysicalConnectionState(true);
    EXPECT_FALSE(cellularDataHandler->physicalConnectionActiveState_);
}

/**
 * @tc.number   HandleScreenStateChangedTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleScreenStateChangedTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->HandleScreenStateChanged(false);
    EXPECT_EQ(cellularDataHandler->connectionManager_, nullptr);
    cellularDataHandler->Init();
    cellularDataHandler->HandleScreenStateChanged(false);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
}

/**
 * @tc.number   UpdateCellularDataConnectStateTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, UpdateCellularDataConnectStateTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    cellularDataHandler->incallDataStateMachine_ = cellularDataHandler->CreateIncallDataStateMachine(0);
    AppExecFwk::InnerEvent::Pointer event(nullptr, nullptr);
    cellularDataHandler->IncallDataComplete(event);
    EXPECT_EQ(cellularDataHandler->incallDataStateMachine_, nullptr);
    cellularDataHandler->UpdateCellularDataConnectState("default");
    EXPECT_EQ(cellularDataHandler->apnManager_->GetOverallDefaultApnState(), 0);
    cellularDataHandler->UpdateCellularDataConnectState("internal_default");
    EXPECT_EQ(cellularDataHandler->apnManager_->GetOverallDefaultApnState(), 0);
}

/**
 * @tc.number   HandleImsCallChangedTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HandleImsCallChangedTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    cellularDataHandler->HandleImsCallChanged(1);
    EXPECT_EQ(cellularDataHandler->incallDataStateMachine_, nullptr);
    cellularDataHandler->HandleImsCallChanged(4);
    EXPECT_NE(cellularDataHandler->incallDataStateMachine_, nullptr);
    cellularDataHandler->HandleImsCallChanged(2);
    EXPECT_NE(cellularDataHandler->incallDataStateMachine_->GetCallState(), 8);
    cellularDataHandler->HandleImsCallChanged(4);
    EXPECT_EQ(cellularDataHandler->incallDataStateMachine_->GetCallState(), 4);
    cellularDataHandler->HandleImsCallChanged(6);
    EXPECT_EQ(cellularDataHandler->incallDataStateMachine_->GetCallState(), 6);
    cellularDataHandler->HandleImsCallChanged(8);
    EXPECT_EQ(cellularDataHandler->incallDataStateMachine_->GetCallState(), 8);
}

/**
 * @tc.number   PsDataRatChangedTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, PsDataRatChangedTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    AppExecFwk::InnerEvent::Pointer event(nullptr, nullptr);
    cellularDataHandler->PsDataRatChanged(event);
    cellularDataHandler->dataSwitchSettings_->userDataOn_ = false;
    EXPECT_FALSE(cellularDataHandler->dataSwitchSettings_->IsUserDataOn());
}

/**
 * @tc.number   SetPolicyDataOnTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, SetPolicyDataOnTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    cellularDataHandler->dataSwitchSettings_->policyDataOn_ = false;
    cellularDataHandler->SetPolicyDataOn(true);
    EXPECT_TRUE(cellularDataHandler->dataSwitchSettings_->IsUserDataOn());
    cellularDataHandler->dataSwitchSettings_->policyDataOn_ = true;
    cellularDataHandler->SetPolicyDataOn(false);
    EXPECT_FALSE(cellularDataHandler->dataSwitchSettings_->policyDataOn_);
}

/**
 * @tc.number   SetRilAttachApnTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, SetRilAttachApnTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    sptr<ApnItem> attachApn = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    cellularDataHandler->apnManager_->allApnItem_.push_back(attachApn);
    cellularDataHandler->SetRilAttachApn();
}

/**
 * @tc.number   HasAnyHigherPriorityConnectionTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HasAnyHigherPriorityConnectionTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    sptr<ApnHolder> sortApnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    sortApnHolder->priority_ = 1;
    sortApnHolder->dataCallEnabled_ = true;
    sortApnHolder->apnState_ = ApnProfileState::PROFILE_STATE_CONNECTED;
    cellularDataHandler->apnManager_->sortedApnHolders_.push_back(sortApnHolder);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder->priority_ = 0;
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_->GetSortApnHolder().empty(), true);
    EXPECT_TRUE(cellularDataHandler->HasAnyHigherPriorityConnection(apnHolder));
}

/**
 * @tc.number   HasAnyHigherPriorityConnectionTest002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HasAnyHigherPriorityConnectionTest002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    sptr<ApnHolder> sortApnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    sortApnHolder->priority_ = 1;
    sortApnHolder->dataCallEnabled_ = true;
    sortApnHolder->apnState_ = ApnProfileState::PROFILE_STATE_CONNECTING;
    cellularDataHandler->apnManager_->sortedApnHolders_.push_back(sortApnHolder);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder->priority_ = 0;
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_->GetSortApnHolder().empty(), true);
    EXPECT_TRUE(cellularDataHandler->HasAnyHigherPriorityConnection(apnHolder));
}

/**
 * @tc.number   HasAnyHigherPriorityConnectionTest003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HasAnyHigherPriorityConnectionTest003, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    sptr<ApnHolder> sortApnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    sortApnHolder->priority_ = 1;
    sortApnHolder->dataCallEnabled_ = true;
    sortApnHolder->apnState_ = ApnProfileState::PROFILE_STATE_DISCONNECTING;
    cellularDataHandler->apnManager_->sortedApnHolders_.push_back(sortApnHolder);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder->priority_ = 0;
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_->GetSortApnHolder().empty(), true);
    EXPECT_TRUE(cellularDataHandler->HasAnyHigherPriorityConnection(apnHolder));
}

/**
 * @tc.number   HasInternetCapabilityTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HasInternetCapabilityTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    sptr<DataConnectionManager> connectionManager = std::make_unique<DataConnectionManager>(0).release();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = std::make_shared<CellularDataStateMachine>(
        connectionManager, nullptr);
    cellularMachine->capability_ = NetCap::NET_CAPABILITY_INTERNET;
    cellularDataHandler->connectionManager_->cidActiveConnectionMap_[0] = cellularMachine;
    EXPECT_NE(cellularDataHandler->connectionManager_->GetActiveConnectionByCid(0), nullptr);
    EXPECT_TRUE(cellularDataHandler->HasInternetCapability(0));
}

/**
 * @tc.number   HasInternetCapabilityTest002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, HasInternetCapabilityTest002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    sptr<DataConnectionManager> connectionManager = std::make_unique<DataConnectionManager>(0).release();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = std::make_shared<CellularDataStateMachine>(
        connectionManager, nullptr);
    cellularMachine->capability_ = NetCap::NET_CAPABILITY_MMS;
    cellularDataHandler->connectionManager_->cidActiveConnectionMap_[0] = cellularMachine;
    EXPECT_NE(cellularDataHandler->connectionManager_->GetActiveConnectionByCid(0), nullptr);
    EXPECT_FALSE(cellularDataHandler->HasInternetCapability(0));
}

/**
 * @tc.number   ChangeConnectionForDsdsTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, ChangeConnectionForDsdsTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    cellularDataHandler->ChangeConnectionForDsds(true);
    EXPECT_TRUE(cellularDataHandler->dataSwitchSettings_->internalDataOn_);
}

/**
 * @tc.number   GetDataConnApnAttrTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, GetDataConnApnAttrTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    sptr<ApnHolder> apnHolder1 = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder1->apnItem_ = nullptr;
    apnHolder1->dataCallEnabled_ = true;
    cellularDataHandler->apnManager_->apnHolders_.push_back(apnHolder1);
    sptr<ApnHolder> apnHolder2 = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    sptr<ApnItem> apnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    apnItem->attr_.isEdited_ = true;
    apnHolder2->apnItem_ = apnItem;
    apnHolder2->dataCallEnabled_ = true;
    cellularDataHandler->apnManager_->apnHolders_.push_back(apnHolder2);
    ApnItem::Attribute apnAttr;
    apnAttr.isEdited_ = false;
    cellularDataHandler->GetDataConnApnAttr(apnAttr);
    EXPECT_TRUE(apnAttr.isEdited_);
}

/**
 * @tc.number   GetDataConnIpTypeTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, GetDataConnIpTypeTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    cellularDataHandler->Init();
    sptr<ApnHolder> apnHolder1 = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder1->cellularDataStateMachine_ = nullptr;
    apnHolder1->dataCallEnabled_ = true;
    cellularDataHandler->apnManager_->apnHolders_.push_back(apnHolder1);
    sptr<ApnHolder> apnHolder2 = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    sptr<DataConnectionManager> connectionManager = std::make_unique<DataConnectionManager>(0).release();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = std::make_shared<CellularDataStateMachine>(
        connectionManager, nullptr);
    cellularMachine->ipType_ = "IPV4";
    apnHolder2->cellularDataStateMachine_ = cellularMachine;
    apnHolder2->dataCallEnabled_ = true;
    cellularDataHandler->apnManager_->apnHolders_.push_back(apnHolder2);
    cellularDataHandler->GetDataConnIpType();
    EXPECT_EQ(cellularDataHandler->GetDataConnIpType(), "IPV4");
}

/**
 * @tc.number   CheckForCompatibleDataConnectionTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, CheckForCompatibleDataConnectionTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 1);
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder->apnType_ = DATA_CONTEXT_ROLE_DUN;
    EXPECT_EQ(cellularDataHandler->CheckForCompatibleDataConnection(apnHolder), nullptr);
    cellularDataHandler->Init();
    EXPECT_EQ(cellularDataHandler->CheckForCompatibleDataConnection(apnHolder), nullptr);
}

/**
 * @tc.number   ReleaseCellularDataConnectionTest001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, ReleaseCellularDataConnectionTest001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    std::set<uint32_t> reqUids = {1};
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>("default", static_cast<int32_t>(0)).release();
    apnHolder->reqUids_ = reqUids;
    cellularDataHandler->apnManager_->apnIdApnHolderMap_[1] = apnHolder;
    cellularDataHandler->ReleaseCellularDataConnection();
    EXPECT_NE(cellularDataHandler->apnManager_->apnIdApnHolderMap_[1]->apnState_, 3);
}

/**
 * @tc.number   UpdateNetworkInfo_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, UpdateNetworkInfo_001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    EXPECT_EQ(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_FALSE(cellularDataHandler->UpdateNetworkInfo());
}

/**
 * @tc.number   UpdateNetworkInfo_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, UpdateNetworkInfo_002, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_TRUE(cellularDataHandler->UpdateNetworkInfo());
}

/**
 * @tc.number   AddUid001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, AddUid001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    NetRequest request;
    request.uid = 1;
    EXPECT_FALSE(cellularDataHandler->AddUid(request));
    cellularDataHandler->apnManager_ = nullptr;
    EXPECT_FALSE(cellularDataHandler->AddUid(request));
}

/**
 * @tc.number   RemoveUid001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, RemoveUid001, Function | MediumTest | Level3)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    NetRequest request;
    request.uid = 1;
    EXPECT_FALSE(cellularDataHandler->RemoveUid(request));
    cellularDataHandler->apnManager_ = nullptr;
    EXPECT_FALSE(cellularDataHandler->RemoveUid(request));
}

/**
 * @tc.number   Telephony_GetCurrentApnId_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, GetCurrentApnId_Test_01, Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    int32_t profileId = cellularDataHandler->GetCurrentApnId();
    EXPECT_NE(profileId, 0);
}

/**
 * @tc.number   Telephony_FindApnHolderById_Test_01
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, FindApnHolderById_Test_01, Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    int32_t profileId = cellularDataHandler->GetCurrentApnId();
    EXPECT_NE(profileId, 0);
    EXPECT_NE(cellularDataHandler->apnManager_->FindApnHolderById(profileId), nullptr);
}

/**
 * @tc.number   Telephony_CellularDataHandler_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, Telephony_CellularDataHandler_001, Function | MediumTest | Level1)
{
    CellularDataController controller {0};
    controller.Init();
    NetRequest request;
    request.ident = "simId1";
    EXPECT_FALSE(controller.AddUid(request));
    controller.cellularDataHandler_ = nullptr;
    EXPECT_FALSE(controller.AddUid(request));
}

/**
 * @tc.number   Telephony_CellularDataHandler_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataHandlerTest, Telephony_CellularDataHandler_002, Function | MediumTest | Level1)
{
    CellularDataController controller {0};
    controller.Init();
    NetRequest request;
    request.ident = "simId1";
    EXPECT_FALSE(controller.RemoveUid(request));
    controller.cellularDataHandler_ = nullptr;
    EXPECT_FALSE(controller.RemoveUid(request));
}
} // namespace Telephony
} // namespace OHOS