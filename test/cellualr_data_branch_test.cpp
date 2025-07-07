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

#include "activating.h"
#include "active.h"
#include "apn_manager.h"
#include "cellular_data_state_machine.h"
#include "data_connection_manager.h"
#include "default.h"
#include "disconnecting.h"
#include "gtest/gtest.h"
#include "inactive.h"
#include "incall_data_state_machine.h"
#include "tel_event_handler.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

static const int32_t SLEEP_TIME = 3;

class CellularStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<CellularDataStateMachine> cellularMachine;
};
void CellularStateMachineTest::SetUpTestCase() {}

void CellularStateMachineTest::TearDownTestCase()
{
    sleep(SLEEP_TIME);
}

void CellularStateMachineTest::SetUp() {}

void CellularStateMachineTest::TearDown() {}

class CellularMachineTest : public TelEventHandler {
public:
    CellularMachineTest() : TelEventHandler("CellularDataStateMachineTest") {}
    ~CellularMachineTest() = default;
    std::shared_ptr<CellularDataStateMachine> CreateCellularDataConnect(int32_t slotId);

public:
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine_ = nullptr;
};

std::shared_ptr<CellularDataStateMachine> CellularMachineTest::CreateCellularDataConnect(int32_t slotId)
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
 * @tc.number   Active_RefreshTcpBufferSizes_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_RefreshTcpBufferSizes_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    active->RefreshTcpBufferSizes();
    ASSERT_TRUE(cellularMachine != nullptr);
}

/**
 * @tc.number   Activating_StateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_StateBegin_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    activating->stateMachine_ = cellularMachine;
    activating->StateBegin();
    EXPECT_EQ(activating->isActive_, true);
}

/**
 * @tc.number   Activating_StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_StateProcess_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DRS_OR_RAT_CHANGED);
    bool result = activating->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    result = activating->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_RIL_SETUP_DATA_CALL);
    result = activating->StateProcess(event);
    EXPECT_EQ(result, false);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_GET_LAST_FAIL_DONE);
    result = activating->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_GET_RIL_BANDWIDTH);
    result = activating->StateProcess(event);
    EXPECT_EQ(result, false);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_CONNECT_TIMEOUT_CHECK);
    result = activating->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(0);
    result = activating->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Activating_ProcessConnectTimeout_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_ProcessConnectTimeout_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, 1);
    activating->ProcessConnectTimeout(event);
    EXPECT_EQ(cellularMachine->stateMachineEventHandler_->destState_, cellularMachine->inActiveState_);
}

/**
 * @tc.number   Activating_ProcessConnectTimeout_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_ProcessConnectTimeout_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->disconnectingState_);
    cellularMachine = nullptr;
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(1);
    activating->ProcessConnectTimeout(event);
    EXPECT_EQ(cellularMachine == nullptr, true);
}

/**
 * @tc.number   Activating_ProcessConnectTimeout_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_ProcessConnectTimeout_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    cellularMachine->TransitionTo(cellularMachine->disconnectingState_);
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    activating->ProcessConnectTimeout(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Activating_CellularDataStateMachine_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_CellularDataStateMachine_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine = nullptr;
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    activating->StateBegin();
    EXPECT_EQ(activating->isActive_, false);
    auto result = activating->RilErrorResponse(event);
    EXPECT_EQ(result, false);
    result = activating->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Disconnecting_StateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_StateBegin_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    disconnecting->stateMachine_ = cellularMachine;
    disconnecting->StateBegin();
    EXPECT_EQ(disconnecting->isActive_, true);
}

/**
 * @tc.number   Disconnecting_ProcessConnectTimeout_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessConnectTimeout_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT);
    disconnecting->ProcessDisconnectTimeout(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Disconnecting_ProcessConnectTimeout_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessConnectTimeout_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT);
    disconnecting->ProcessDisconnectTimeout(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Disconnecting_ProcessRilAdapterHostDied_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessRilAdapterHostDied_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT);
    disconnecting->ProcessRilAdapterHostDied(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Disconnecting_ProcessRilDeactivateDataCall_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessRilDeactivateDataCall_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT);
    disconnecting->ProcessRilDeactivateDataCall(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Disconnecting_ProcessRilDeactivateDataCall_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessRilDeactivateDataCall_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->defaultState_);
    cellularMachine->stateMachineEventHandler_ = nullptr;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT);
    disconnecting->ProcessRilDeactivateDataCall(event);
    EXPECT_FALSE(cellularMachine->IsInactiveState());
}

/**
 * @tc.number   Disconnecting_ProcessRilDeactivateDataCall_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessRilDeactivateDataCall_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->defaultState_);
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT);
    event = nullptr;
    disconnecting->ProcessRilDeactivateDataCall(event);
    EXPECT_FALSE(cellularMachine->IsInactiveState());
}

/**
 * @tc.number Disconnecting_ProcessRilDeactivateDataCall_004
 * @tc.name test function branch
 * @tc.desc Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessRilDeactivateDataCall_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    disconnecting->stateMachine_ = cellularMachine;
    cellularMachine->connectId_ = 1;
    std::shared_ptr<RadioResponseInfo> radioResponseInfo = std::make_shared<RadioResponseInfo>();
    radioResponseInfo->flag = 1;
    radioResponseInfo->error = ErrType::ERR_GENERIC_FAILURE;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, radioResponseInfo);
    disconnecting->ProcessRilDeactivateDataCall(event);
    EXPECT_FALSE(cellularMachine->IsInactiveState());
}

/**
 * @tc.number   Disconnecting_StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_StateProcess_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_RIL_DEACTIVATE_DATA_CALL);
    bool result = disconnecting->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DISCONNECT_TIMEOUT_CHECK);
    result = disconnecting->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_RIL_ADAPTER_HOST_DIED);
    result = disconnecting->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    result = disconnecting->StateProcess(event);
    EXPECT_EQ(result, true);
    event = AppExecFwk::InnerEvent::Get(0);
    result = disconnecting->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Disconnecting_StateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_StateProcess_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine = nullptr;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_RIL_DEACTIVATE_DATA_CALL);
    bool result = disconnecting->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   InactiveStateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, InactiveStateProcess_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto inactive = static_cast<Inactive *>(cellularMachine->inActiveState_.GetRefPtr());
    inactive->stateMachine_ = cellularMachine;
    sptr<ApnHolder> apnHolder;
    int32_t profileId = 0;
    int32_t radioTechnology = 0;
    bool nonTrafficUseOnly = false;
    bool roamingState = false;
    bool userDataRoaming = false;
    std::shared_ptr<DataConnectionParams> dataConnectionParams = std::make_shared<DataConnectionParams>(apnHolder,
        profileId, radioTechnology, nonTrafficUseOnly, roamingState, userDataRoaming);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, dataConnectionParams);
    bool result = inactive->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Disconnecting_ProcessDisconnectTimeout_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessDisconnectTimeout_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine = nullptr;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, 0);
    disconnecting->ProcessDisconnectTimeout(event);
    disconnecting->StateEnd();
    EXPECT_EQ(disconnecting->isActive_, false);
}

/**
 * @tc.number   Disconnecting_ProcessRilAdapterHostDied_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessRilAdapterHostDied_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->inActiveState_ = nullptr;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, 0);
    disconnecting->ProcessRilAdapterHostDied(event);
    EXPECT_EQ(cellularMachine->IsDisconnectingState(), false);
}

/**
 * @tc.number   Activating_RilActivatePdpContextDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilActivatePdpContextDone_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine = nullptr;
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = activating->RilActivatePdpContextDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Activating_RilActivatePdpContextDone_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilActivatePdpContextDone_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    setupDataCallResultInfo->flag = 2;
    auto event = AppExecFwk::InnerEvent::Get(0, setupDataCallResultInfo);
    bool result = activating->RilActivatePdpContextDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Activating_RilActivatePdpContextDone_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilActivatePdpContextDone_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    setupDataCallResultInfo->flag = 1;
    setupDataCallResultInfo->reason = 1;
    auto event = AppExecFwk::InnerEvent::Get(0, setupDataCallResultInfo);
    bool result = activating->RilActivatePdpContextDone(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Activating_RilActivatePdpContextDone_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilActivatePdpContextDone_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    setupDataCallResultInfo->flag = 1;
    setupDataCallResultInfo->reason = 0;
    setupDataCallResultInfo->active = 0;
    auto event = AppExecFwk::InnerEvent::Get(0, setupDataCallResultInfo);
    bool result = activating->RilActivatePdpContextDone(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Activating_RilActivatePdpContextDone_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilActivatePdpContextDone_005, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    setupDataCallResultInfo->flag = 1;
    setupDataCallResultInfo->reason = 0;
    setupDataCallResultInfo->active = 1;
    setupDataCallResultInfo->cid = 99;
    auto event = AppExecFwk::InnerEvent::Get(0, setupDataCallResultInfo);
    bool result = activating->RilActivatePdpContextDone(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Activating_RilErrorResponse_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilErrorResponse_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<RadioResponseInfo> radioResponseInfo = std::make_shared<RadioResponseInfo>();
    radioResponseInfo->flag = 12;
    auto event = AppExecFwk::InnerEvent::Get(0, radioResponseInfo);
    bool result = activating->RilErrorResponse(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Activating_RilErrorResponse_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilErrorResponse_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<RadioResponseInfo> radioResponseInfo = std::make_shared<RadioResponseInfo>();
    radioResponseInfo->flag = 1;
    radioResponseInfo->error = ErrType::ERR_GENERIC_FAILURE;
    auto event = AppExecFwk::InnerEvent::Get(0, radioResponseInfo);
    bool result = activating->RilErrorResponse(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Activating_RilErrorResponse_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilErrorResponse_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<RadioResponseInfo> radioResponseInfo = std::make_shared<RadioResponseInfo>();
    radioResponseInfo->flag = 1;
    radioResponseInfo->error = ErrType::ERR_INVALID_RESPONSE;
    auto event = AppExecFwk::InnerEvent::Get(0, radioResponseInfo);
    bool result = activating->RilErrorResponse(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Activating_RilErrorResponse_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilErrorResponse_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    std::shared_ptr<RadioResponseInfo> radioResponseInfo = std::make_shared<RadioResponseInfo>();
    radioResponseInfo->flag = 1;
    radioResponseInfo->error = ErrType::NONE;
    auto event = AppExecFwk::InnerEvent::Get(0, radioResponseInfo);
    bool result = activating->RilErrorResponse(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Activating_RilErrorResponse_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_RilErrorResponse_005, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine = nullptr;
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = activating->RilErrorResponse(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_CellularDataStateMachine_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_CellularDataStateMachine_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine = nullptr;
    active->stateMachine_ = cellularMachine;
    active->StateBegin();
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->ProcessDisconnectDone(event);
    EXPECT_EQ(result, false);
    result = active->ProcessDisconnectAllDone(event);
    EXPECT_EQ(result, false);
    result = active->ProcessLostConnection(event);
    EXPECT_EQ(result, false);
    result = active->ProcessLinkCapabilityChanged(event);
    EXPECT_EQ(result, false);
    result = active->ProcessDataConnectionRoamOn(event);
    EXPECT_EQ(result, false);
    active->RefreshConnectionBandwidths();
    result = active->ProcessDataConnectionRoamOff(event);
    EXPECT_EQ(result, false);
    active->RefreshTcpBufferSizes();
    result = active->ProcessDataConnectionVoiceCallStartedOrEnded(event);
    EXPECT_EQ(result, false);
}
} // namespace Telephony
} // namespace OHOS