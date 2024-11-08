/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "updateinactivemachine_fuzzer.h"

#define private public
#define protected public

#include "activating.h"
#include "adddatatoken_fuzzer.h"
#include "inactive.h"
#include "statemachine_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
using namespace OHOS::Telephony;
using namespace AppExecFwk;
using namespace OHOS::EventFwk;
constexpr int32_t SLOT_NUM_MAX = 3;
constexpr int32_t EVENT_ID_MAX = 255;
bool g_flag = false;

void UpdateInActiveMachineFuzz(const uint8_t *data, size_t size)
{
    std::shared_ptr<StateMachineFuzzer> machine = std::make_shared<StateMachineFuzzer>();
    if (machine == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    std::shared_ptr<CellularDataStateMachine> cellularMachine = machine->CreateCellularDataConnect(slotId);
    if (cellularMachine == nullptr) {
        return;
    }
    cellularMachine->Init();

    sptr<Inactive> inactive =
        std::make_unique<Inactive>(std::weak_ptr<CellularDataStateMachine>(cellularMachine), "Inactive").release();
    sptr<Activating> activating =
        std::make_unique<Activating>(std::weak_ptr<CellularDataStateMachine>(cellularMachine), "Activating").release();

    if (inactive == nullptr || activating == nullptr) {
        TELEPHONY_LOGE("memory allocation failed");
        return;
    }

    std::int32_t intValue = fdp.ConsumeIntegralInRange<uint32_t>(0, EVENT_ID_MAX);
    std::unique_ptr<uint8_t> object = std::make_unique<uint8_t>(*data);
    if (object == nullptr) {
        return;
    }

    InnerEvent::Pointer event = InnerEvent::Get(intValue, object);
    inactive->SetDeActiveApnTypeId(intValue);
    inactive->StateProcess(event);

    activating->StateProcess(event);
    activating->RilErrorResponse(event);
    activating->ProcessConnectTimeout(event);
    activating->StateProcess(event);
}

void UpdateActiveMachineWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    if (!g_flag) {
        UpdateInActiveMachineFuzz(data, size);
        g_flag = true;
    }
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AddDataTokenFuzzer token;
    /* Run your code on data */
    OHOS::UpdateActiveMachineWithMyAPI(data, size);
    return 0;
}