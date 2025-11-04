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

#include "cellular_data_error.h"
#include "cellular_data_service.h"
#include "core_manager_inner.h"
#include "telephony_common_utils.h"
#include <charconv>

namespace OHOS {
namespace Telephony {
int32_t NetManagerTacticsCallBack::NetStrategySwitch(const std::string &simId, bool enable)
{
    if (simId.length() == 0) {
        TELEPHONY_LOGI("StrategySwitch[The simd length is 0]");
        return CELLULAR_DATA_INVALID_PARAM;
    }
    if (!IsValidDecValue(simId)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t tempNetSimId = 0;
    ConvertStrToInt(simId, tempNetSimId);
    const int32_t netSimId = tempNetSimId;
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    for (int32_t  i = 0; i < SIM_SLOT_COUNT; ++i) {
        IccAccountInfo accountInfo;
        if (coreInner.GetSimAccountInfo(i, accountInfo) != TELEPHONY_ERR_SUCCESS) {
            continue;
        }
        if (netSimId == accountInfo.simId) {
            int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().StrategySwitch(i, enable);
            TELEPHONY_LOGI("StrategySwitch[%{public}s, %{public}d, %{public}d] result %{public}d", simId.c_str(),
                i, enable, result);
            return result;
        }
    }
    TELEPHONY_LOGI("StrategySwitch fail[%{public}s, %{public}d]", simId.c_str(), enable);
    return CELLULAR_DATA_INVALID_PARAM;
}

bool NetManagerTacticsCallBack::ConvertStrToInt(const std::string& str, int32_t& value)
{
    if (str.empty()) {
        return false;
    }
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, 10);  // 10: 十进制
    bool succ = ec == std::errc{} && ptr == str.data() + str.size();
    if (!succ) {
        TELEPHONY_LOGE("ConvertStrToInt failed: str: %{public}s", str.c_str());
    }
    return succ;
}
} // Telephony
} // OHOS
