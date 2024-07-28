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

#include "apn_manager.h"
#include "cellular_data_state_machine.h"
#include "data_connection_manager.h"
#include "default.h"
#include "gtest/gtest.h"
#include "inactive.h"
#include "incall_data_state_machine.h"
#include "tel_event_handler.h"

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
    IncallStateMachineTest() : TelEventHandler("IncallDataStateMachineTest") {}
    ~IncallStateMachineTest() = default;
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
    CellularMachineTest() : TelEventHandler("CellularMachineTest") {}
    ~CellularMachineTest() = default;
    std::shared_ptr<CellularDataStateMachine> CreateCellularDataStateMachine(int32_t slotId);

public:
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine_ = nullptr;
};

std::shared_ptr<CellularDataStateMachine> CellularMachineTest::CreateCellularDataStateMachine(int32_t slotId)
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
HWTEST_F(ApnManagerTest, HasAnyConnectedState_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateCellularDataStateMachine(0);
    incallStateMachine->apnManager_ = nullptr;
    ASSERT_EQ(incallStateMachine->HasAnyConnectedState(), false);
}

/**
 * @tc.number   StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, StateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateCellularDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->deactivatingSecondaryState_);
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->deactivatingSecondaryState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = deactivatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   StateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, StateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateCellularDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->deactivatingSecondaryState_);
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->deactivatingSecondaryState_.GetRefPtr());
    auto event = AppExecFwk::InnerEvent::Get(0);
    bool result = deactivatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   StateProcess_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, StateProcess_003, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateCellularDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(MSG_SM_INCALL_DATA_SETTINGS_ON);
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
HWTEST_F(ApnManagerTest, ActivatingStateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateCellularDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    auto activatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->activatingSecondaryState_.GetRefPtr());
    bool result = activatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   ActivatingStateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, ActivatingStateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateCellularDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(-1);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    auto activatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->activatingSecondaryState_.GetRefPtr());
    bool result = activatingSecondaryState->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   SecondaryActivatingStateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, SecondaryActivatingStateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateCellularDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->secondaryActiveState_);
    auto secondaryActiveState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    bool result = secondaryActiveState->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   InactiveStateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, InactiveStateBegin_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
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
HWTEST_F(ApnManagerTest, InactiveStateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
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
HWTEST_F(ApnManagerTest, InactiveStateProcess_003, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
    cellularMachine->Init();
    auto inactive = static_cast<Inactive *>(cellularMachine->inActiveState_.GetRefPtr());
    inactive->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT_ALL);
    bool result = inactive->StateProcess(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   DefaultStateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, DefaultStateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
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
HWTEST_F(ApnManagerTest, DefaultProcessDisconnectDone_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    mDefault->stateMachine_ = cellularMachine;
    mDefault->eventIdFunMap_.clear();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDisconnectDone(event);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number   DefaulProcessDataConnectionDrsOrRatChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, DefaulProcessDataConnectionDrsOrRatChanged_001, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->activeState);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataConnectionDrsOrRatChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaulProcessDataConnectionDrsOrRatChanged_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, DefaulProcessDataConnectionDrsOrRatChanged_002, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->activatingState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataConnectionDrsOrRatChanged(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   DefaulProcessDataConnectionDrsOrRatChanged_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(ApnManagerTest, DefaulProcessDataConnectionDrsOrRatChanged_003, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataStateMachine(0);
    cellularMachine->Init();
    auto mDefault = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    cellularMachine->TransitionTo(cellularMachine->disconnectingState_);
    mDefault->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT);
    bool result = mDefault->ProcessDataConnectionDrsOrRatChanged(event);
    EXPECT_EQ(result, false);
}
} // namespace Telephony
} // namespace OHOS