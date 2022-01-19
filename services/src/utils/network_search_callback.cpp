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

#include "network_search_callback.h"

#include "telephony_log_wrapper.h"

#include "cellular_data_service.h"

namespace OHOS {
namespace Telephony {
bool NetworkSearchCallback::HasInternetCapability(int32_t slotId, int32_t cId)
{
    int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().HasInternetCapability(slotId, cId);
    if (result == static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)) {
        return true;
    }
    TELEPHONY_LOGE("HasInternetCapability slot:%{public}d cid:%{public}d failed %{public}d", slotId, cId, result);
    return false;
}

void NetworkSearchCallback::ClearCellularDataConnections(int32_t slotId)
{
    int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().ClearCellularDataConnections(slotId);
    if (result != static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)) {
        TELEPHONY_LOGE("ClearCellularDataConnections slot:%{public}d failed %{public}d", slotId, result);
    }
}
} // namespace Telephony
} // namespace OHOS
