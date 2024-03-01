/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "updateincalldatamachine_fuzzer.h"

#define private public
#define protected public

#include "adddatatoken_fuzzer.h"
#include "incalldatastatemachine_fuzzer.h"

namespace OHOS {
using namespace OHOS::Telephony;
using namespace AppExecFwk;
using namespace OHOS::EventFwk;
static int32_t SIM_COUNT = 2;
bool g_flag = false;

void IdleStateMachineFuzz(const uint8_t *data, size_t size)
{
    std::shared_ptr<IncallDataStateMachineFuzzer> fuzzer = std::make_shared<IncallDataStateMachineFuzzer>();
    if (fuzzer == nullptr) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SIM_COUNT);
    std::shared_ptr<IncallDataStateMachine> machine = fuzzer->CreateIncallDataStateMachine(slotId);
    if (machine == nullptr) {
        return;
    }
    std::int32_t intValue = static_cast<int32_t>(size);
    machine->Init(intValue);
    if (machine->idleState_ == nullptr) {
        return;
    }
    auto idleState = static_cast<IdleState *>(machine->idleState_.GetRefPtr());
    machine->GetCurrentState();
    machine->CanActiveDataByRadioTech();
    machine->IsSecondaryCanActiveData();
    machine->IsIncallDataSwitchOn();
    machine->GetSlotId();
    machine->HasAnyConnectedState();
    machine->GetCallState();
    machine->UpdateCallState(intValue);
    std::shared_ptr<uint8_t> object = std::make_shared<uint8_t>(*data);
    if (object == nullptr) {
        return;
    }
    InnerEvent::Pointer event = InnerEvent::Get(intValue, object);
    idleState->StateProcess(event);
    idleState->ProcessCallStarted(event);
    idleState->ProcessCallEnded(event);
    idleState->ProcessSettingsOn(event);
    idleState->ProcessDsdsChanged(event);
}

void ActivatingSecondaryStateMachineFuzz(const uint8_t *data, size_t size)
{
    std::shared_ptr<IncallDataStateMachineFuzzer> fuzzer = std::make_shared<IncallDataStateMachineFuzzer>();
    if (fuzzer == nullptr) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SIM_COUNT);
    std::shared_ptr<IncallDataStateMachine> machine = fuzzer->CreateIncallDataStateMachine(slotId);
    if (machine == nullptr) {
        return;
    }
    std::int32_t intValue = static_cast<int32_t>(size);
    machine->Init(intValue);
    if (machine->activatingSecondaryState_ == nullptr || machine->secondaryActiveState_ == nullptr) {
        return;
    }
    auto activatingSecondaryState =
        static_cast<ActivatingSecondaryState *>(machine->activatingSecondaryState_.GetRefPtr());
    auto secondaryActiveState = static_cast<SecondaryActiveState *>(machine->secondaryActiveState_.GetRefPtr());
    machine->TransitionTo(machine->activatingSecondaryState_);
    machine->GetCurrentState();
    machine->CanActiveDataByRadioTech();
    machine->IsSecondaryCanActiveData();
    machine->IsIncallDataSwitchOn();
    machine->GetSlotId();
    machine->HasAnyConnectedState();
    machine->GetCallState();
    machine->UpdateCallState(intValue);
    std::shared_ptr<uint8_t> object = std::make_shared<uint8_t>(*data);
    if (object == nullptr) {
        return;
    }
    InnerEvent::Pointer event = InnerEvent::Get(intValue, object);
    activatingSecondaryState->StateProcess(event);
    secondaryActiveState->StateProcess(event);
    secondaryActiveState->ProcessSettingsOn(event);
    secondaryActiveState->ProcessCallEnded(event);
    secondaryActiveState->ProcessSettingsOff(event);
    secondaryActiveState->ProcessDsdsChanged(event);
}

void ActivatedSecondaryStateMachineFuzz(const uint8_t *data, size_t size)
{
    std::shared_ptr<IncallDataStateMachineFuzzer> fuzzer = std::make_shared<IncallDataStateMachineFuzzer>();
    if (fuzzer == nullptr) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SIM_COUNT);
    std::shared_ptr<IncallDataStateMachine> machine = fuzzer->CreateIncallDataStateMachine(slotId);
    if (machine == nullptr) {
        return;
    }
    std::int32_t intValue = static_cast<int32_t>(size);
    machine->Init(intValue);
    if (machine->activatedSecondaryState_ == nullptr || machine->secondaryActiveState_ == nullptr) {
        return;
    }
    auto activatedSecondaryState =
        static_cast<ActivatedSecondaryState *>(machine->activatedSecondaryState_.GetRefPtr());
    auto secondaryActiveState = static_cast<SecondaryActiveState *>(machine->secondaryActiveState_.GetRefPtr());
    machine->TransitionTo(machine->activatingSecondaryState_);
    machine->TransitionTo(machine->activatedSecondaryState_);
    machine->GetCurrentState();
    machine->CanActiveDataByRadioTech();
    machine->IsSecondaryCanActiveData();
    machine->IsIncallDataSwitchOn();
    machine->GetSlotId();
    machine->HasAnyConnectedState();
    machine->GetCallState();
    machine->UpdateCallState(intValue);
    std::shared_ptr<uint8_t> object = std::make_shared<uint8_t>(*data);
    if (object == nullptr) {
        return;
    }
    InnerEvent::Pointer event = InnerEvent::Get(intValue, object);
    activatedSecondaryState->StateProcess(event);
    secondaryActiveState->StateProcess(event);
    secondaryActiveState->ProcessSettingsOn(event);
    secondaryActiveState->ProcessCallEnded(event);
    secondaryActiveState->ProcessSettingsOff(event);
    secondaryActiveState->ProcessDsdsChanged(event);
}

void DeactivatingSecondaryStateMachineFuzz(const uint8_t *data, size_t size)
{
    std::shared_ptr<IncallDataStateMachineFuzzer> fuzzer = std::make_shared<IncallDataStateMachineFuzzer>();
    if (fuzzer == nullptr) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SIM_COUNT);
    std::shared_ptr<IncallDataStateMachine> machine = fuzzer->CreateIncallDataStateMachine(slotId);
    if (machine == nullptr) {
        return;
    }
    std::int32_t intValue = static_cast<int32_t>(size);
    machine->Init(intValue);
    if (machine->deactivatingSecondaryState_ == nullptr || machine->idleState_ == nullptr) {
        return;
    }
    auto deactivatingSecondaryState =
        static_cast<DeactivatingSecondaryState *>(machine->deactivatingSecondaryState_.GetRefPtr());
    auto idleState = static_cast<IdleState *>(machine->idleState_.GetRefPtr());
    machine->TransitionTo(machine->activatingSecondaryState_);
    machine->TransitionTo(machine->activatedSecondaryState_);
    machine->TransitionTo(machine->deactivatingSecondaryState_);
    machine->GetCurrentState();
    machine->CanActiveDataByRadioTech();
    machine->IsSecondaryCanActiveData();
    machine->IsIncallDataSwitchOn();
    machine->GetSlotId();
    machine->HasAnyConnectedState();
    machine->GetCallState();
    machine->UpdateCallState(intValue);
    std::shared_ptr<uint8_t> object = std::make_shared<uint8_t>(*data);
    if (object == nullptr) {
        return;
    }
    InnerEvent::Pointer event = InnerEvent::Get(intValue, object);
    deactivatingSecondaryState->StateProcess(event);
    idleState->StateProcess(event);
    idleState->ProcessCallStarted(event);
    idleState->ProcessCallEnded(event);
    idleState->ProcessSettingsOn(event);
    idleState->ProcessDsdsChanged(event);
}

void UpdateIncallDataMachineWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    if (!g_flag) {
        IdleStateMachineFuzz(data, size);
        ActivatingSecondaryStateMachineFuzz(data, size);
        ActivatedSecondaryStateMachineFuzz(data, size);
        DeactivatingSecondaryStateMachineFuzz(data, size);
        g_flag = true;
    }
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AddDataTokenFuzzer token;
    /* Run your code on data */
    OHOS::UpdateIncallDataMachineWithMyAPI(data, size);
    return 0;
}