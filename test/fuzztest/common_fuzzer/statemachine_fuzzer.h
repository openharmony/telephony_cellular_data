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

#ifndef STATE_MACHINE_FUZZER_H
#define STATE_MACHINE_FUZZER_H

#include "cellular_data_state_machine.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;

class StateMachineFuzzer : public AppExecFwk::EventHandler {
public:
    StateMachineFuzzer() = default;
    ~StateMachineFuzzer() = default;
    std::shared_ptr<CellularDataStateMachine> CreateCellularDataConnect(int32_t slotId);

public:
    std::shared_ptr<AppExecFwk::EventRunner> stateMachineEventLoop_ = nullptr;
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine_ = nullptr;
};
} // namespace Telephony
} // namespace OHOS
#endif // STATE_MACHINE_FUZZER_H