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

class CellularStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void CellularStateMachineTest::SetUpTestCase() {}

void CellularStateMachineTest::TearDownTestCase() {}

void CellularStateMachineTest::SetUp() {}

void CellularStateMachineTest::TearDown() {}

class IncallDataStateMachineTest : public TelEventHandler {
public:
    IncallDataStateMachineTest() : TelEventHandler("IncallDataStateMachineTest") {}
    ~IncallDataStateMachineTest() = default;
    std::shared_ptr<IncallDataStateMachine> CreateIncallDataStateMachine(int32_t slotId);

public:
    std::shared_ptr<IncallDataStateMachine> incallStateMachine_ = nullptr;
};

std::shared_ptr<IncallDataStateMachine> IncallDataStateMachineTest::CreateIncallDataStateMachine(int32_t slotId)
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
 * @tc.number   HasAnyConnectedState_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, HasAnyConnectedState_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->apnManager_ = nullptr;
    ASSERT_EQ(incallStateMachine->HasAnyConnectedState(), false);
}

/**
 * @tc.number   StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, StateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->deactivatingSecondaryState_);
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->deactivatingSecondaryState_.GetRefPtr());
    bool result = deactivatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   StateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, StateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->deactivatingSecondaryState_);
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->deactivatingSecondaryState_.GetRefPtr());
    bool result = deactivatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   StateProcess_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, StateProcess_003, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_ON);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->deactivatingSecondaryState_);
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->deactivatingSecondaryState_.GetRefPtr());
    bool result = deactivatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   ActivatingStateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, ActivatingStateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    auto activatingSecondaryState =
        static_cast<ActivatingSecondaryState *>(incallStateMachine->activatingSecondaryState_.GetRefPtr());
    bool result = activatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   ActivatingStateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, ActivatingStateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(-1);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    auto activatingSecondaryState =
        static_cast<ActivatingSecondaryState *>(incallStateMachine->activatingSecondaryState_.GetRefPtr());
    bool result = activatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   SecondaryActiveStateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, SecondaryActiveStateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->secondaryActiveState_);
    auto secondaryActiveState =
        static_cast<ActivatingSecondaryState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    bool result = secondaryActiveState->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   InactiveStateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, InactiveStateBegin_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto inactive = static_cast<Inactive *>(cellularMachine->inActiveState_.GetRefPtr());
    inactive->deActiveApnTypeId_ = 0;
    inactive->SetStateMachine(cellularMachine);
    EXPECT_EQ(inactive->reason_, DisConnectionReason::REASON_RETRY_CONNECTION);
}

/**
 * @tc.number   InactiveStateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, InactiveStateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto inactive = static_cast<Inactive *>(cellularMachine->inActiveState_.GetRefPtr());
    inactive->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT);
    bool result = inactive->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   InactiveStateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, InactiveStateProcess_003, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto inactive = static_cast<Inactive *>(cellularMachine->inActiveState_.GetRefPtr());
    inactive->SetStateMachine(cellularMachine);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT_ALL);
    bool result = inactive->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   DefaultStateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultStateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    mDefault->stateMachine_ = cellularMachine;
    mDefault->eventIdFunMap_.clear();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDisconnectDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDisconnectDone_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    mDefault->stateMachine_ = cellularMachine;
    mDefault->eventIdFunMap_.clear();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDisconnectDone(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   DefaultProcessDataConnectionDrsOrRatChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataConnectionDrsOrRatChanged_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->activeState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataConnectionDrsOrRatChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDataConnectionDrsOrRatChanged_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataConnectionDrsOrRatChanged_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->activatingState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataConnectionDrsOrRatChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDataConnectionDrsOrRatChanged_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataConnectionDrsOrRatChanged_003, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->disconnectingState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataConnectionDrsOrRatChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDataCallListChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataCallListChanged_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->disconnectingState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataCallListChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_StateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_StateBegin_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    active->StateBegin();
    EXPECT_EQ(active->isActive_, true);
}

/**
 * @tc.number   Active_StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_StateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_StateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_StateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDisconnectDone_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    bool result = active->ProcessDisconnectDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectAllDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLostConnection_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->inActiveState_);
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->ProcessLostConnection(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessLostConnection_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLostConnection_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine->inActiveState_ = nullptr;
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->ProcessLostConnection(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDataConnectionRoamOn_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDataConnectionRoamOn_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessDataConnectionRoamOn(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessDataConnectionRoamOff_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDataConnectionRoamOff_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessDataConnectionRoamOff(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessDataConnectionVoiceCallStartedOrEnded_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, ProcessDataConnectionVoiceCallStartedOrEnded_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessDataConnectionVoiceCallStartedOrEnded(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessNrStateChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessNrStateChanged_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessNrStateChanged(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_RefreshTcpBufferSizes_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_RefreshTcpBufferSizes_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    active->RefreshTcpBufferSizes();
    ASSERT_TRUE(cellularMachine != nullptr);
}

/**
 * @tc.number   Active_RefreshTcpBufferSizes_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_RefreshTcpBufferSizes_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine->cdConnectionManager_ = nullptr;
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
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    activating->stateMachine_ = cellularMachine;
    activating->StateBegin();
    EXPECT_EQ(activating->isActive_, true);
}

/**
 * @tc.number   Activating_DataCallPdpError_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_DataCallPdpError_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    auto result = activating->DataCallPdpError(96);
    EXPECT_EQ(result, DisConnectionReason::REASON_CLEAR_CONNECTION);
    result = activating->DataCallPdpError(114);
    EXPECT_EQ(result, DisConnectionReason::REASON_CLEAR_CONNECTION);
}

/**
 * @tc.number   Activating_StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_StateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
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
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(1);
    activating->ProcessConnectTimeout(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Activating_ProcessConnectTimeout_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Activating_ProcessConnectTimeout_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto activating = static_cast<Activating *>(cellularMachine->activatingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    activating->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    activating->ProcessConnectTimeout(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Disconnecting_StateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_StateBegin_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
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
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(1);
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
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->connectId_ = 1;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
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
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
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
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    disconnecting->ProcessRilDeactivateDataCall(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Disconnecting_ProcessRilDeactivateDataCall_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_ProcessRilDeactivateDataCall_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    auto disconnecting = static_cast<Disconnecting *>(cellularMachine->disconnectingState_.GetRefPtr());
    cellularMachine->stateMachineEventHandler_ = nullptr;
    disconnecting->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    disconnecting->ProcessRilDeactivateDataCall(event);
    EXPECT_EQ(cellularMachine->IsInactiveState(), false);
}

/**
 * @tc.number   Disconnecting_StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Disconnecting_StateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
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
 * @tc.number   CellularDataStateMachine_GetSlotId_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_GetSlotId_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    int result = cellularMachine->GetSlotId();
    ASSERT_EQ(result, DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   CellularDataStateMachine_HasMatchedIpTypeAddrs_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_HasMatchedIpTypeAddrs_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    uint8_t ipType = 1;
    uint8_t ipInfoArraySize = 2;
    std::vector<AddressInfo> ipInfoArray;
    AddressInfo info1;
    info1.type = 1;
    AddressInfo info2;
    info2.type = 3;
    ipInfoArray.push_back(info1);
    ipInfoArray.push_back(info2);
    bool result = cellularMachine->HasMatchedIpTypeAddrs(ipType, ipInfoArraySize, ipInfoArray);
    ASSERT_TRUE(result);
}

/**
 * @tc.number   CellularDataStateMachine_HasMatchedIpTypeAddrs_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_HasMatchedIpTypeAddrs_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    uint8_t ipType = 5;
    uint8_t ipInfoArraySize = 2;
    std::vector<AddressInfo> ipInfoArray;
    AddressInfo info1;
    info1.type = 1;
    AddressInfo info2;
    info2.type = 3;
    ipInfoArray.push_back(info1);
    ipInfoArray.push_back(info2);
    bool result = cellularMachine->HasMatchedIpTypeAddrs(ipType, ipInfoArraySize, ipInfoArray);
    ASSERT_FALSE(result);
}

/**
 * @tc.number   GetIpType_ShouldReturnIPV4V6_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, GetIpType_ShouldReturnIPV4V6_001, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    std::vector<AddressInfo> ipInfoArray;
    AddressInfo info1;
    info1.type = INetAddr::IpType::IPV4;
    AddressInfo info2;
    info2.type = INetAddr::IpType::IPV6;
    ipInfoArray.push_back(info1);
    ipInfoArray.push_back(info2);
    std::string result = cellularMachine->GetIpType(ipInfoArray);
    ASSERT_EQ(result, "IPV4V6");
}

/**
 * @tc.number   GetIpType_ShouldReturnIPV4V6_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, GetIpType_ShouldReturnIPV4V6_002, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    std::vector<AddressInfo> ipInfoArray;
    AddressInfo info1;
    info1.type = INetAddr::IpType::IPV4;
    ipInfoArray.push_back(info1);
    std::string result = cellularMachine->GetIpType(ipInfoArray);
    ASSERT_EQ(result, "IPV4");
}

/**
 * @tc.number   GetIpType_ShouldReturnIPV6_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, GetIpType_ShouldReturnIPV6_003, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    std::vector<AddressInfo> ipInfoArray;
    AddressInfo info2;
    info2.type = INetAddr::IpType::IPV6;
    ipInfoArray.push_back(info2);
    std::string result = cellularMachine->GetIpType(ipInfoArray);
    ASSERT_EQ(result, "IPV6");
}

/**
 * @tc.number   GetIpType_ShouldReturnEmpty_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, GetIpType_ShouldReturnEmpty_004,  TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    std::vector<AddressInfo> ipInfoArray = {};
    std::string result = cellularMachine->GetIpType(ipInfoArray);
    ASSERT_EQ(result, "");
}

/**
 * @tc.number   GetIpType_ShouldReturnIPV4V6_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, GetIpType_ShouldReturnIPV4V6_005, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    std::vector<AddressInfo> ipInfoArray;
    AddressInfo info1;
    info1.type = 5;
    AddressInfo info2;
    info2.type = 6;
    ipInfoArray.push_back(info1);
    ipInfoArray.push_back(info2);
    std::string result = cellularMachine->GetIpType(ipInfoArray);
    ASSERT_EQ(result, "");
}

HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfo_001, TestSize.Level0) {
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    SetupDataCallResultInfo dataCallInfo;
    dataCallInfo.address = "";
    dataCallInfo.dns = "";
    dataCallInfo.dnsSec = "";
    dataCallInfo.gateway = "";
    cellularMachine->UpdateNetworkInfo(dataCallInfo);
    ASSERT_EQ(cellularMachine->cause_, 0);
}

HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfo_002, TestSize.Level0) {
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    cellularMachine->Init();
    SetupDataCallResultInfo dataCallInfo;
    dataCallInfo.address = "192.168.1.1";
    dataCallInfo.dns = "192.168.1.1";
    dataCallInfo.dnsSec = "192.168.1.1";
    dataCallInfo.gateway = "192.168.1.1";
    dataCallInfo.reason = 1;
    cellularMachine->UpdateNetworkInfo(dataCallInfo);
    ASSERT_EQ(cellularMachine->cause_, 1);
}

HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfo_003, TestSize.Level0) {
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    SetupDataCallResultInfo dataCallInfo;
    dataCallInfo.address = "192.168.1.1";
    dataCallInfo.dns = "192.168.1.1";
    dataCallInfo.dnsSec = "192.168.1.1";
    dataCallInfo.gateway = "192.168.1.1";
    dataCallInfo.reason = 1;
    cellularMachine->UpdateNetworkInfo(dataCallInfo);
    ASSERT_EQ(cellularMachine->cause_, 0);
}
} // namespace Telephony
} // namespace OHOS