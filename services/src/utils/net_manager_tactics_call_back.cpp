/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "net_manager_tactics_call_back.h"

#include "cellular_data_service.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
int32_t NetManagerTacticsCallBack::NetStrategySwitch(const std::string &slotId, bool enable)
{
    const int32_t netSimId = std::stoi(slotId);
    int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().StrategySwitch(netSimId, enable);
    TELEPHONY_LOGI("StrategySwitch[%{public}d, %{public}d] result %{public}d", netSimId, enable, result);
    return result;
}
} // Telephony
} // OHOS
