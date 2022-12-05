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

#include "statemachine_fuzzer.h"

namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;

std::shared_ptr<CellularDataStateMachine> StateMachineFuzzer::CreateCellularDataConnect(int32_t slotId)
{
    if (cellularDataStateMachine_ != nullptr) {
        return cellularDataStateMachine_;
    }
    stateMachineEventLoop_ = AppExecFwk::EventRunner::Create("CellularDataStateMachine");
    if (stateMachineEventLoop_ == nullptr) {
        return nullptr;
    }
    stateMachineEventLoop_->Run();

    sptr<DataConnectionManager> connectionManager =
        std::make_unique<DataConnectionManager>(GetEventRunner(), slotId).release();
    if (connectionManager == nullptr) {
        return nullptr;
    }
    cellularDataStateMachine_ =
        std::make_shared<CellularDataStateMachine>(connectionManager, shared_from_this(), stateMachineEventLoop_);
    return cellularDataStateMachine_;
}
} // namespace Telephony
} // namespace OHOS