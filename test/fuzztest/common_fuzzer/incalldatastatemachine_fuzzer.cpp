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

#include "incalldatastatemachine_fuzzer.h"

namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;

std::shared_ptr<IncallDataStateMachine> IncallDataStateMachineFuzzer::CreateIncallDataStateMachine(int32_t slotId)
{
    if (incallDataStateMachine_ != nullptr) {
        return incallDataStateMachine_;
    }
    sptr<ApnManager> apnManager = std::make_unique<ApnManager>().release();
    if (apnManager == nullptr) {
        return nullptr;
    }
    incallDataStateMachine_ = std::make_shared<IncallDataStateMachine>(slotId,
        std::weak_ptr<TelEventHandler>(std::static_pointer_cast<TelEventHandler>(shared_from_this())), apnManager);
    return incallDataStateMachine_;
}
} // namespace Telephony
} // namespace OHOS