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

#ifndef INCALL_DATA_STATE_MACHINE_FUZZER_H
#define INCALL_DATA_STATE_MACHINE_FUZZER_H

#include "common_event_manager.h"
#include "common_event_support.h"
#include "incall_data_state_machine.h"

namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;

class IncallDataStateMachineFuzzer : public TelEventHandler {
public:
    explicit IncallDataStateMachineFuzzer() : TelEventHandler("IncallDataStateMachineFuzzer") {}
    ~IncallDataStateMachineFuzzer() = default;
    std::shared_ptr<IncallDataStateMachine> CreateIncallDataStateMachine(int32_t slotId);

public:
    std::shared_ptr<IncallDataStateMachine> incallDataStateMachine_ = nullptr;
};
} // namespace Telephony
} // namespace OHOS
#endif // INCALL_DATA_STATE_MACHINE_FUZZER_H