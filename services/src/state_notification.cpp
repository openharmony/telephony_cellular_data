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

#include "state_notification.h"

#include "telephony_log_wrapper.h"
#include "telephony_state_registry_client.h"

namespace OHOS {
namespace Telephony {
StateNotification StateNotification::stateNotification_;
StateNotification &StateNotification::GetInstance()
{
    return stateNotification_;
}

void StateNotification::UpdateCellularDataConnectState(int32_t slotId, ApnProfileState dataState, int32_t networkType)
{
    int32_t state = CellularDataStateAdapter(dataState);
    TELEPHONY_LOGI("UpdateCellularDataConnectState= %{public}d, %{public}d, %{public}d", slotId, state, networkType);
    TelephonyStateRegistryClient::GetInstance().UpdateCellularDataConnectState(slotId, state, networkType);
}

void StateNotification::OnUpDataFlowtype(int32_t slotId, CellDataFlowType flowType)
{
    TELEPHONY_LOGI("UpdateCellularDataFlow= %{public}d, %{public}d", slotId, flowType);
    TelephonyStateRegistryClient::GetInstance().UpdateCellularDataFlow(slotId, static_cast<int32_t>(flowType));
}
} // namespace Telephony
} // namespace OHOS