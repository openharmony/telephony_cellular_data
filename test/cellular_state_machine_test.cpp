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
#include "data_disconnect_params.h"
#include "default.h"
#include "disconnecting.h"
#include "gtest/gtest.h"
#include "inactive.h"
#include "incall_data_state_machine.h"
#include "tel_event_handler.h"
#include "telephony_types.h"
#include "tel_ril_data_parcel.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class CellularStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<CellularDataStateMachine> cellularMachine;
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    mDefault->stateMachine_ = cellularMachine;
    mDefault->eventIdFunMap_.clear();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultStateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultStateProcess_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine = nullptr;
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    mDefault->stateMachine_ = cellularMachine;
    mDefault->eventIdFunMap_.clear();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDisconnectDone(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   DefaultProcessDisconnectDone_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDisconnectDone_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine = nullptr;
    mDefault->stateMachine_ = cellularMachine;
    mDefault->eventIdFunMap_.clear();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDisconnectDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDisconnectAllDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDisconnectAllDone_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDisconnectDone(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   DefaultProcessDisconnectAllDone_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDisconnectAllDone_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine = nullptr;
    mDefault->stateMachine_ = cellularMachine;
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->disconnectingState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataConnectionDrsOrRatChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDataConnectionDrsOrRatChanged_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataConnectionDrsOrRatChanged_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine = nullptr;
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->disconnectingState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataCallListChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDataCallListChanged_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataCallListChanged_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine = nullptr;
    mDefault->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> dataCallInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, dataCallInfo);
    bool result = mDefault->ProcessDataCallListChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaultProcessDataCallListChanged_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataCallListChanged_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->activatingState_);
    mDefault->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> dataCallInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, dataCallInfo);
    bool result = mDefault->ProcessDataCallListChanged(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   DefaultProcessDataCallListChanged_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, DefaultProcessDataCallListChanged_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->inActiveState_);
    mDefault->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> dataCallInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, dataCallInfo);
    bool result = mDefault->ProcessDataCallListChanged(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_StateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_StateBegin_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_StateProcess_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_StateProcess_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine = nullptr;
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessDisconnectDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDisconnectDone_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    bool result = active->ProcessDisconnectDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectDone_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDisconnectDone_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->ProcessDisconnectDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectAllDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDisconnectAllDone_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    bool result = active->ProcessDisconnectAllDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectAllDone_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDisconnectAllDone_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->ProcessDisconnectAllDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectAllDone_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDisconnectAllDone_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->inActiveState_);
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<DataDisconnectParams> dataDisconnectParams =
        std::make_shared<DataDisconnectParams>("", DisConnectionReason::REASON_NORMAL);
    auto event = AppExecFwk::InnerEvent::Get(0, dataDisconnectParams);
    bool result = active->ProcessDisconnectAllDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectAllDone_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDisconnectAllDone_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine->inActiveState_ = nullptr;
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<DataDisconnectParams> dataDisconnectParams =
        std::make_shared<DataDisconnectParams>("", DisConnectionReason::REASON_NORMAL);
    auto event = AppExecFwk::InnerEvent::Get(0, dataDisconnectParams);
    bool result = active->ProcessDisconnectAllDone(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessLinkCapabilityChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLinkCapabilityChanged_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->ProcessLinkCapabilityChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessLinkCapabilityChanged_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLinkCapabilityChanged_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine = nullptr;
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<DataLinkCapability> linkCapability = std::make_shared<DataLinkCapability>();
    auto event = AppExecFwk::InnerEvent::Get(0, linkCapability);
    bool result = active->ProcessLinkCapabilityChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessLinkCapabilityChanged_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLinkCapabilityChanged_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<DataLinkCapability> linkCapability = std::make_shared<DataLinkCapability>();
    linkCapability->primaryUplinkKbps = 0;
    auto event = AppExecFwk::InnerEvent::Get(0, linkCapability);
    bool result = active->ProcessLinkCapabilityChanged(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessLinkCapabilityChanged_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLinkCapabilityChanged_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<DataLinkCapability> linkCapability = std::make_shared<DataLinkCapability>();
    linkCapability->primaryUplinkKbps = 1;
    linkCapability->primaryDownlinkKbps = 0;
    auto event = AppExecFwk::InnerEvent::Get(0, linkCapability);
    bool result = active->ProcessLinkCapabilityChanged(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessLinkCapabilityChanged_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLinkCapabilityChanged_005, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<DataLinkCapability> linkCapability = std::make_shared<DataLinkCapability>();
    linkCapability->primaryUplinkKbps = 1;
    linkCapability->primaryDownlinkKbps = 1;
    auto event = AppExecFwk::InnerEvent::Get(0, linkCapability);
    bool result = active->ProcessLinkCapabilityChanged(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessDataConnectionComplete_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDataConnectionComplete_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = active->ProcessDataConnectionComplete(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDataConnectionComplete_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDataConnectionComplete_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine = nullptr;
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(0, setupDataCallResultInfo);
    bool result = active->ProcessDataConnectionComplete(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDataConnectionComplete_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDataConnectionComplete_003, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(0, setupDataCallResultInfo);
    bool result = active->ProcessDataConnectionComplete(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessDataConnectionComplete_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessDataConnectionComplete_004, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine->stateMachineEventHandler_ = nullptr;
    active->stateMachine_ = cellularMachine;
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    auto event = AppExecFwk::InnerEvent::Get(0, setupDataCallResultInfo);
    bool result = active->ProcessDataConnectionComplete(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessNrStateChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessNrStateChanged_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessNrStateChanged(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   Active_ProcessNrStateChanged_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessNrStateChanged_002, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine = nullptr;
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessNrStateChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessNrFrequencyChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessNrFrequencyChanged_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    cellularMachine = nullptr;
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessNrFrequencyChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   Active_ProcessDisconnectAllDone_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Active_ProcessLostConnection_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto active = static_cast<Active *>(cellularMachine->activeState_.GetRefPtr());
    active->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = active->ProcessDataConnectionVoiceCallStartedOrEnded(event);
    EXPECT_EQ(result, true);
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
HWTEST_F(CellularStateMachineTest, GetIpType_ShouldReturnEmpty_004, TestSize.Level0)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
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

/**
 * @tc.number   CellularDataStateMachine_UpdateNetworkInfo_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfo_001, TestSize.Level0) {
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    SetupDataCallResultInfo dataCallInfo;
    dataCallInfo.address = "";
    dataCallInfo.dns = "";
    dataCallInfo.dnsSec = "";
    dataCallInfo.gateway = "";
    cellularMachine->UpdateNetworkInfo(dataCallInfo);
    ASSERT_EQ(cellularMachine->cause_, 0);
}

/**
 * @tc.number   CellularDataStateMachine_UpdateNetworkInfo_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfo_002, TestSize.Level0) {
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    SetupDataCallResultInfo dataCallInfo;
    dataCallInfo.address = "192.168.1.1";
    dataCallInfo.dns = "192.168.1.1";
    dataCallInfo.dnsSec = "192.168.1.1";
    dataCallInfo.gateway = "192.168.1.1";
    dataCallInfo.reason = 1;
    cellularMachine->UpdateNetworkInfo(dataCallInfo);
    ASSERT_EQ(cellularMachine->cause_, 1);
}

/**
 * @tc.number   CellularDataStateMachine_UpdateNetworkInfo_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
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

/**
 * @tc.number   CellularDataStateMachine_UpdateNetworkInfo_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfo_004, TestSize.Level0) {
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    cellularMachine->netSupplierInfo_->isAvailable_ = true;
    cellularMachine->UpdateNetworkInfo();
    ASSERT_NE(cellularMachine->netSupplierInfo_, nullptr);
}

/**
 * @tc.number   CellularDataStateMachine_UpdateNetworkInfo_005
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfo_005, TestSize.Level0) {
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    cellularMachine->netSupplierInfo_->isAvailable_ = false;
    cellularMachine->UpdateNetworkInfo();
    ASSERT_NE(cellularMachine->netSupplierInfo_, nullptr);
}

/**
 * @tc.number   CellularDataStateMachine_ResolveRoute_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
 HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_ResolveRoute_001, TestSize.Level0)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    std::vector<AddressInfo> routeInfoArray;
    cellularMachine->ResolveRoute(routeInfoArray, "eth0");
    EXPECT_TRUE(cellularMachine->netLinkInfo_->routeList_.empty());
}

/**
 * @tc.number   CellularDataStateMachine_ResolveRoute_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_ResolveRoute_002, TestSize.Level0)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    std::vector<AddressInfo> routeInfoArray;
    AddressInfo routeInfo;
    routeInfo.ip = "192.168.1.1";
    routeInfo.type = INetAddr::IpType::IPV4;
    routeInfo.prefixLen = 24;
    routeInfoArray.push_back(routeInfo);
    cellularMachine->ResolveRoute(routeInfoArray, "eth0");
    EXPECT_FALSE(cellularMachine->netLinkInfo_->routeList_.empty());
}

/**
 * @tc.number   CellularDataStateMachine_ResolveRoute_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_ResolveRoute_003, TestSize.Level0)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    std::vector<AddressInfo> routeInfoArray;
    AddressInfo routeInfo1;
    routeInfo1.ip = "192.168.1.1";
    routeInfo1.type = INetAddr::IpType::IPV4;
    routeInfo1.prefixLen = 24;
    routeInfoArray.push_back(routeInfo1);
    AddressInfo routeInfo2;
    routeInfo2.ip = "2001:db8::1";
    routeInfo2.type = INetAddr::IpType::IPV6;
    routeInfo2.prefixLen = 64;
    routeInfoArray.push_back(routeInfo2);
    cellularMachine->ResolveRoute(routeInfoArray, "eth0");
    EXPECT_EQ(cellularMachine->netLinkInfo_->routeList_.size(), 2);
}

/**
 * @tc.number   CellularDataStateMachine_UpdateNetworkInfoIfInActive_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfoIfInActive_001, TestSize.Level0)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    SetupDataCallResultInfo dataCallInfo;
    cellularMachine->UpdateNetworkInfoIfInActive(dataCallInfo);
    EXPECT_NE(cellularMachine->stateMachineEventHandler_, nullptr);
}

/**
 * @tc.number   CellularDataStateMachine_UpdateNetworkInfoIfInActive_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_UpdateNetworkInfoIfInActive_002, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    SetupDataCallResultInfo dataCallInfo;
    cellularMachine->stateMachineEventHandler_ = nullptr;
    cellularMachine->UpdateNetworkInfoIfInActive(dataCallInfo);
    EXPECT_EQ(cellularMachine->stateMachineEventHandler_, nullptr);
}
} // namespace Telephony
} // namespace OHOS