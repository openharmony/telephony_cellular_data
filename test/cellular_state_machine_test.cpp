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
#include "core_manager_inner.h"
#include "data_connection_manager.h"
#include "data_connection_params.h"
#include "data_disconnect_params.h"
#include "default.h"
#include "disconnecting.h"
#include "gtest/gtest.h"
#include "inactive.h"
#include "incall_data_state_machine.h"
#include "mock/mock_sim_manager.h"
#include "mock/mock_network_search.h"
#include "tel_event_handler.h"
#include "telephony_types.h"
#include "tel_ril_data_parcel.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Mock;
using ::testing::Return;
using ::testing::SetArgReferee;

class CellularStateMachineTest : public testing::Test {
public:
    CellularStateMachineTest()
    {
        mockSimManager = new MockSimManager();
        std::shared_ptr<MockSimManager> mockSimManagerPtr(mockSimManager);
        CoreManagerInner::GetInstance().simManager_ = mockSimManagerPtr;

        mockNetworkSearchManager = new MockNetworkSearchManager();
        std::shared_ptr<MockNetworkSearchManager> mockNetworkSearchManagerPtr(mockNetworkSearchManager);
        CoreManagerInner::GetInstance().networkSearchManager_ = mockNetworkSearchManagerPtr;
    }
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<CellularDataStateMachine> cellularMachine;
    MockSimManager *mockSimManager;
    MockNetworkSearchManager *mockNetworkSearchManager;
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
 * @tc.number   IdleState_StateBegin_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IdleState_StateBegin_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->idleState_);
    auto idleState = static_cast<IdleState *>(incallStateMachine->idleState_.GetRefPtr());
    incallStateMachine = nullptr;
    idleState->stateMachine_ = incallStateMachine;
    idleState->StateBegin();
    ASSERT_EQ(idleState->isActive_, false);
}

/**
 * @tc.number   IdleState_StateBegin_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IdleState_StateBegin_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->idleState_);
    auto idleState = static_cast<IdleState *>(incallStateMachine->idleState_.GetRefPtr());
    incallStateMachine->UpdateCallState(static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE));
    idleState->stateMachine_ = incallStateMachine;
    idleState->StateBegin();
    ASSERT_EQ(idleState->isActive_, true);
}

/**
 * @tc.number   IdleState_StateBegin_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IdleState_StateBegin_003, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->idleState_);
    auto idleState = static_cast<IdleState *>(incallStateMachine->idleState_.GetRefPtr());
    incallStateMachine->UpdateCallState(static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED));
    idleState->stateMachine_ = incallStateMachine;
    idleState->StateBegin();
    ASSERT_EQ(idleState->isActive_, true);
}

/**
 * @tc.number   IdleState_StateProcess_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IdleState_StateProcess_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->idleState_);
    auto idleState = static_cast<IdleState *>(incallStateMachine->idleState_.GetRefPtr());
    incallStateMachine = nullptr;
    idleState->stateMachine_ = incallStateMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    idleState->StateProcess(event);
    ASSERT_EQ(idleState->isActive_, NOT_PROCESSED);
}

/**
 * @tc.number   IdleState_StateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IdleState_StateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->idleState_);
    auto idleState = static_cast<IdleState *>(incallStateMachine->idleState_.GetRefPtr());
    idleState->stateMachine_ = incallStateMachine;
    idleState->eventIdFunMap_.clear();
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_STARTED);
    idleState->StateProcess(event);
    ASSERT_EQ(idleState->isActive_, NOT_PROCESSED);
}

/**
 * @tc.number   IdleState_IncallStateMachine_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IdleState_IncallStateMachine_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->idleState_);
    auto idleState = static_cast<IdleState *>(incallStateMachine->idleState_.GetRefPtr());
    incallStateMachine = nullptr;
    idleState->stateMachine_ = incallStateMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    bool result = idleState->ProcessCallStarted(event);
    ASSERT_EQ(result, NOT_PROCESSED);
    result = idleState->ProcessCallEnded(event);
    ASSERT_EQ(result, NOT_PROCESSED);
    result = idleState->ProcessSettingsOn(event);
    ASSERT_EQ(result, NOT_PROCESSED);
    result = idleState->ProcessDsdsChanged(event);
    ASSERT_EQ(result, NOT_PROCESSED);
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
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->activatedSecondaryState_);
    incallStateMachine->TransitionTo(incallStateMachine->deactivatingSecondaryState_);
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(incallStateMachine->deactivatingSecondaryState_.GetRefPtr());
    incallStateMachine = nullptr;
    deactivatingSecondaryState->stateMachine_ = incallStateMachine;
    bool result = deactivatingSecondaryState->StateProcess(event);
    deactivatingSecondaryState->StateBegin();
    EXPECT_EQ(result, NOT_PROCESSED);
}

/**
 * @tc.number   StateProcess_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, StateProcess_004, Function | MediumTest | Level1)
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
 * @tc.number   ActivatingSecondaryState_IncallDataStateMachine_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, ActivatingSecondaryState_IncallDataStateMachine_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(0);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->activatingSecondaryState_);
    auto activatingSecondaryState =
        static_cast<ActivatingSecondaryState *>(incallStateMachine->activatingSecondaryState_.GetRefPtr());
    incallStateMachine = nullptr;
    activatingSecondaryState->stateMachine_ = incallStateMachine;
    bool result = activatingSecondaryState->StateProcess(event);
    activatingSecondaryState->StateBegin();
    EXPECT_EQ(result, NOT_PROCESSED);
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
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    bool result = secondaryActiveState->StateProcess(event);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number   SecondaryActiveStateProcess_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, SecondaryActiveStateProcess_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->secondaryActiveState_);
    auto secondaryActiveState =
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    incallStateMachine = nullptr;
    secondaryActiveState->stateMachine_ = incallStateMachine;
    bool result = secondaryActiveState->StateProcess(event);
    EXPECT_EQ(result, NOT_PROCESSED);
}

/**
 * @tc.number   SecondaryActiveStateProcess_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, SecondaryActiveStateProcess_003, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->secondaryActiveState_);
    auto secondaryActiveState =
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    secondaryActiveState->stateMachine_ = incallStateMachine;
    secondaryActiveState->eventIdFunMap_.clear();
    bool result = secondaryActiveState->StateProcess(event);
    EXPECT_EQ(result, NOT_PROCESSED);
}

/**
 * @tc.number   SecondaryActiveState_ProcessCallEnded_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, SecondaryActiveState_ProcessCallEnded_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->secondaryActiveState_);
    auto secondaryActiveState =
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    incallStateMachine = nullptr;
    secondaryActiveState->stateMachine_ = incallStateMachine;
    bool result = secondaryActiveState->ProcessCallEnded(event);
    EXPECT_EQ(result, NOT_PROCESSED);
}

/**
 * @tc.number   SecondaryActiveState_ProcessCallEnded_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, SecondaryActiveState_ProcessCallEnded_002, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->secondaryActiveState_);
    incallStateMachine->UpdateCallState(static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE));
    auto secondaryActiveState =
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    secondaryActiveState->stateMachine_ = incallStateMachine;
    bool result = secondaryActiveState->ProcessCallEnded(event);
    EXPECT_EQ(result, PROCESSED);
    incallStateMachine->UpdateCallState(static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED));
    result = secondaryActiveState->ProcessCallEnded(event);
    EXPECT_EQ(result, PROCESSED);
}

/**
 * @tc.number   SecondaryActiveState_IncallDataStateMachine_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, SecondaryActiveState_IncallDataStateMachine_001, Function | MediumTest | Level1)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
    incallStateMachine->Init(TelCallStatus::CALL_STATUS_DIALING);
    incallStateMachine->TransitionTo(incallStateMachine->secondaryActiveState_);
    auto secondaryActiveState =
        static_cast<SecondaryActiveState *>(incallStateMachine->secondaryActiveState_.GetRefPtr());
    incallStateMachine = nullptr;
    secondaryActiveState->stateMachine_ = incallStateMachine;
    bool result = secondaryActiveState->ProcessCallEnded(event);
    EXPECT_EQ(result, NOT_PROCESSED);
    result = secondaryActiveState->ProcessSettingsOff(event);
    EXPECT_EQ(result, NOT_PROCESSED);
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
    inactive->SetDataCallResultInfoToRetry();
    EXPECT_EQ(inactive->resultInfo_->reason, static_cast<int32_t>(PdpErrorReason::PDP_ERR_RETRY));
    inactive->SetDataCallResultInfoToClear();
    EXPECT_EQ(inactive->resultInfo_->reason,
        static_cast<int32_t>(PdpErrorReason::PDP_ERR_UNKNOWN_TO_CLEAR_CONNECTION));
}

/**
 * @tc.number   InactiveStateProcess_002
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
 * @tc.number   InactiveStateProcess_003
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
 * @tc.number   Inactive_CellularDataStateMachine_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Inactive_CellularDataStateMachine_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
    }
    auto inactive = static_cast<Inactive *>(cellularMachine->inActiveState_.GetRefPtr());
    cellularMachine = nullptr;
    inactive->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT_ALL);
    bool result = inactive->StateProcess(event);
    inactive->StateBegin();
    EXPECT_EQ(result, false);
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
    bool result = mDefault->ProcessDisconnectAllDone(event);
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
    bool result = mDefault->ProcessDisconnectAllDone(event);
    EXPECT_EQ(result, false);
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

/**
 * @tc.number   CellularDataStateMachine_DoConnect_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_SplitProxyIpAddress_001, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    std::string proxyIpAddress = "";
    std::string host;
    uint16_t port;
    cellularMachine->SplitProxyIpAddress(proxyIpAddress, host, port);
    ASSERT_EQ(host, "");
    ASSERT_EQ(port, 0);
}

/**
 * @tc.number   CellularDataStateMachine_DoConnect_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_SplitProxyIpAddress_002, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    std::string proxyIpAddress = "192.168.1.1";
    std::string host;
    uint16_t port;
    cellularMachine->SplitProxyIpAddress(proxyIpAddress, host, port);
    ASSERT_EQ(host, "192.168.1.1");
    ASSERT_EQ(port, 0);
}

/**
 * @tc.number   CellularDataStateMachine_DoConnect_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_SplitProxyIpAddress_003, TestSize.Level0)
{
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    std::string proxyIpAddress = "192.168.1.1:8080";
    std::string host;
    uint16_t port;
    cellularMachine->SplitProxyIpAddress(proxyIpAddress, host, port);
    ASSERT_EQ(host, "192.168.1.1");
    ASSERT_EQ(port, 8080);
}

/**
 * @tc.number   IncallDataStateMachine_IsSecondaryCanActiveData_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IncallDataStateMachine_IsSecondaryCanActiveData_001, TestSize.Level0)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &dsdsMode) {
            dsdsMode = static_cast<int32_t>(DsdsMode::DSDS_MODE_V3);
            return 0;
        });
    auto result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   IncallDataStateMachine_IsSecondaryCanActiveData_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IncallDataStateMachine_IsSecondaryCanActiveData_002, TestSize.Level0)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &dsdsMode) {
            dsdsMode = static_cast<int32_t>(DsdsMode::DSDS_MODE_V2);
            return 0;
        });
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &slotId) {
            slotId = INVALID_SLOT_ID;
            return 0;
        });
    auto result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &slotId) {
            slotId = 0;
            return 0;
        });
    result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   IncallDataStateMachine_IsSecondaryCanActiveData_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IncallDataStateMachine_IsSecondaryCanActiveData_003, TestSize.Level0)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &dsdsMode) {
            dsdsMode = static_cast<int32_t>(DsdsMode::DSDS_MODE_V2);
            return 0;
        });
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &slotId) {
            slotId = 1;
            return 0;
        });
    auto result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   IncallDataStateMachine_IsSecondaryCanActiveData_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IncallDataStateMachine_IsSecondaryCanActiveData_004, TestSize.Level0)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &dsdsMode) {
            dsdsMode = static_cast<int32_t>(DsdsMode::DSDS_MODE_V2);
            return 0;
        });
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &slotId) {
            slotId = 1;
            return 0;
        });
    EXPECT_CALL(*mockSimManager, HasSimCard(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, bool &hasSimCard) {
            hasSimCard = true;
            return 0;
        });
    EXPECT_CALL(*mockNetworkSearchManager, GetImsRegStatus(_, _, _)).Times(AtLeast(2))
        .WillOnce([](int32_t slotId, ImsServiceType imsSrvType, ImsRegInfo &info) {
            switch (imsSrvType) {
                case ImsServiceType::TYPE_VOICE:
                    info.imsRegState = ImsRegState::IMS_UNREGISTERED;
                    break;
                case ImsServiceType::TYPE_VIDEO:
                    info.imsRegState = ImsRegState::IMS_UNREGISTERED;
                    break;
                default:
                    break;
            }
            return 0;
        });
    auto result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   IncallDataStateMachine_IsSecondaryCanActiveData_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IncallDataStateMachine_IsSecondaryCanActiveData_005, TestSize.Level0)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &dsdsMode) {
            dsdsMode = static_cast<int32_t>(DsdsMode::DSDS_MODE_V2);
            return 0;
        });
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &slotId) {
            slotId = 1;
            return 0;
        });
    EXPECT_CALL(*mockSimManager, HasSimCard(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, bool &hasSimCard) {
            hasSimCard = true;
            return 0;
        });
    EXPECT_CALL(*mockNetworkSearchManager, GetImsRegStatus(_, _, _)).Times(AtLeast(2))
        .WillOnce([](int32_t slotId, ImsServiceType imsSrvType, ImsRegInfo &info) {
            switch (imsSrvType) {
                case ImsServiceType::TYPE_VOICE:
                    info.imsRegState = ImsRegState::IMS_REGISTERED ;
                    break;
                case ImsServiceType::TYPE_VIDEO:
                    info.imsRegState = ImsRegState::IMS_UNREGISTERED;
                    break;
                default:
                    break;
            }
            return 0;
        });
    auto result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   IncallDataStateMachine_IsSecondaryCanActiveData_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IncallDataStateMachine_IsSecondaryCanActiveData_006, TestSize.Level0)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &dsdsMode) {
            dsdsMode = static_cast<int32_t>(DsdsMode::DSDS_MODE_V2);
            return 0;
        });
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &slotId) {
            slotId = 1;
            return 0;
        });
    EXPECT_CALL(*mockSimManager, HasSimCard(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, bool &hasSimCard) {
            hasSimCard = true;
            return 0;
        });
    EXPECT_CALL(*mockNetworkSearchManager, GetImsRegStatus(_, _, _)).Times(AtLeast(2))
        .WillOnce([](int32_t slotId, ImsServiceType imsSrvType, ImsRegInfo &info) {
            switch (imsSrvType) {
                case ImsServiceType::TYPE_VOICE:
                    info.imsRegState = ImsRegState::IMS_REGISTERED ;
                    break;
                case ImsServiceType::TYPE_VIDEO:
                    info.imsRegState = ImsRegState::IMS_REGISTERED;
                    break;
                default:
                    break;
            }
            return 0;
        });
    incallStateMachine->callState_ = static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE);
    auto result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
    incallStateMachine->callState_ = static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED);
    result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
    incallStateMachine->callState_ = static_cast<int32_t>(TelCallStatus::CALL_STATUS_DIALING);
    result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, false);
}

/**
 * @tc.number   IncallDataStateMachine_IsSecondaryCanActiveData_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, IncallDataStateMachine_IsSecondaryCanActiveData_007, TestSize.Level0)
{
    std::shared_ptr<IncallDataStateMachineTest> incallStateMachineTest = std::make_shared<IncallDataStateMachineTest>();
    std::shared_ptr<IncallDataStateMachine> incallStateMachine =
        incallStateMachineTest->CreateIncallDataStateMachine(0);
    EXPECT_CALL(*mockSimManager, GetDsdsMode(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &dsdsMode) {
            dsdsMode = static_cast<int32_t>(DsdsMode::DSDS_MODE_V2);
            return 0;
        });
    EXPECT_CALL(*mockSimManager, GetPrimarySlotId(_)).Times(AtLeast(1))
        .WillOnce([](int32_t &slotId) {
            slotId = 1;
            return 0;
        });
    EXPECT_CALL(*mockSimManager, HasSimCard(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, bool &hasSimCard) {
            hasSimCard = true;
            return 0;
        });
    EXPECT_CALL(*mockNetworkSearchManager, GetImsRegStatus(_, _, _)).Times(AtLeast(2))
        .WillOnce([](int32_t slotId, ImsServiceType imsSrvType, ImsRegInfo &info) {
            switch (imsSrvType) {
                case ImsServiceType::TYPE_VOICE:
                    info.imsRegState = ImsRegState::IMS_REGISTERED ;
                    break;
                case ImsServiceType::TYPE_VIDEO:
                    info.imsRegState = ImsRegState::IMS_REGISTERED;
                    break;
                default:
                    break;
            }
            return 0;
        });
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, int32_t &psRadioTech) {
            psRadioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE);
            return 0;
        });
    incallStateMachine->callState_ = static_cast<int32_t>(TelCallStatus::CALL_STATUS_DIALING);
    auto result = incallStateMachine->IsSecondaryCanActiveData();
    ASSERT_EQ(result, true);
}

/**
 * @tc.number   CellularDataStateMachine_GetMtuSizeFromOpCfg_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_GetMtuSizeFromOpCfg_001, TestSize.Level0)
{
    int32_t mtuSize = 0;
    int32_t slotId = 0;
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    EXPECT_CALL(*mockSimManager, GetOperatorConfigs(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, OperatorConfig &poc) {
            poc.stringValue[KEY_MTU_SIZE_STRING] = "ipv4:1500;ipv6:1400";
            return 0;
        });
    cellularMachine->ipType_ = "ipv4";
    cellularMachine->GetMtuSizeFromOpCfg(mtuSize, slotId);
    ASSERT_EQ(mtuSize, 1500);
}

/**
 * @tc.number   CellularDataStateMachine_GetMtuSizeFromOpCfg_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_GetMtuSizeFromOpCfg_002, TestSize.Level0)
{
    int32_t mtuSize = 0;
    int32_t slotId = 0;
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    EXPECT_CALL(*mockSimManager, GetOperatorConfigs(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, OperatorConfig &poc) {
            poc.stringValue[KEY_MTU_SIZE_STRING] = "ipv4:1500;ipv6:1400";
            return 0;
        });
    cellularMachine->ipType_ = "ipv6";
    cellularMachine->GetMtuSizeFromOpCfg(mtuSize, slotId);
    ASSERT_EQ(mtuSize, 1400);
}

/**
 * @tc.number   CellularDataStateMachine_GetMtuSizeFromOpCfg_003
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_GetMtuSizeFromOpCfg_003, TestSize.Level0)
{
    int32_t mtuSize = 0;
    int32_t slotId = 0;
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    EXPECT_CALL(*mockSimManager, GetOperatorConfigs(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, OperatorConfig &poc) {
            poc.stringValue[KEY_MTU_SIZE_STRING] = "ipv4:1500;ipv6:1400";
            return 0;
        });
    cellularMachine->ipType_ = "ipv5";
    cellularMachine->GetMtuSizeFromOpCfg(mtuSize, slotId);
    ASSERT_EQ(mtuSize, 0);
}

/**
 * @tc.number   CellularDataStateMachine_GetMtuSizeFromOpCfg_004
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_GetMtuSizeFromOpCfg_004, TestSize.Level0)
{
    int32_t mtuSize = 0;
    int32_t slotId = 0;
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    EXPECT_CALL(*mockSimManager, GetOperatorConfigs(_, _)).Times(AtLeast(1))
        .WillOnce([](int32_t slotId, OperatorConfig &poc) {
            poc.stringValue[KEY_MTU_SIZE_STRING] = "ipv4:abc;ipv6:1400";
            return 0;
        });
    cellularMachine->ipType_ = "ipv4";
    cellularMachine->GetMtuSizeFromOpCfg(mtuSize, slotId);
    ASSERT_EQ(mtuSize, 0);
}

/**
 * @tc.number   CellularDataStateMachine_GetNetScoreBySlotId_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_GetNetScoreBySlotId_001, TestSize.Level0)
{
    int32_t score;
    int32_t slotId = 0;
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).Times(AtLeast(1))
        .WillOnce([]() {
            return 0;
        });
    score = cellularMachine->GetNetScoreBySlotId(slotId);
    ASSERT_EQ(score, DEFAULT_INTERNET_CONNECTION_SCORE);
}

/**
 * @tc.number   CellularDataStateMachine_GetNetScoreBySlotId_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, CellularDataStateMachine_GetNetScoreBySlotId_002, TestSize.Level0)
{
    int32_t score;
    int32_t slotId = 0;
    std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(0);
    EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).Times(AtLeast(1))
        .WillOnce([]() {
            return 1;
        });
    score = cellularMachine->GetNetScoreBySlotId(slotId);
    ASSERT_EQ(score, OTHER_CONNECTION_SCORE);
}

/**
 * @tc.number   Default_ProcessUpdateNetworkInfo_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularStateMachineTest, Default_ProcessUpdateNetworkInfo_001, Function | MediumTest | Level1)
{
    if (cellularMachine == nullptr) {
        std::shared_ptr<CellularMachineTest> machine = std::make_shared<CellularMachineTest>();
        cellularMachine = machine->CreateCellularDataConnect(0);
        cellularMachine->Init();
        EXPECT_CALL(*mockSimManager, GetDefaultCellularDataSlotId()).Times(AtLeast(1))
        .WillOnce([]() {
            return 0;
        });
    }
    auto defaultState = static_cast<Default *>(cellularMachine->defaultState_.GetRefPtr());
    defaultState->stateMachine_ = cellularMachine;
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_UPDATE_NETWORK_INFO);
    bool result = defaultState->ProcessUpdateNetworkInfo(event);
    EXPECT_EQ(result, true);
}
} // namespace Telephony
} // namespace OHOS