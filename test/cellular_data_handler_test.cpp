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
    cellularDataHandler->Init();
    cellularDataHandler->SetCellularDataRoamingEnabled(false);
    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler->RoamingStateOff(event);
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
    cellularDataHandler->Init();
    DataProfile dataProfile;
    cellularDataHandler->ClearConnectionsOnUpdateApns(dataProfile, DisConnectionReason::REASON_RETRY_CONNECTION);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
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
    DataProfile dataProfile;
    dataProfile.profileId = 1;
    cellularDataHandler->ClearConnectionsOnUpdateApns(dataProfile, DisConnectionReason::REASON_RETRY_CONNECTION);
    EXPECT_NE(cellularDataHandler->connectionManager_, nullptr);
    EXPECT_NE(cellularDataHandler->apnManager_, nullptr);
}
} // namespace Telephony
} // namespace OHOS