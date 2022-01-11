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

#include "sim_utils.h"

#include "cellular_data_constant.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
int32_t SimUtils::GetSimState(int32_t slotId)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        return core->GetSimState(slotId);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
    return SIM_STATE_UNKNOWN;
}

std::u16string SimUtils::GetSimIccId(const int32_t slotId)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        return core->GetSimIccId(slotId);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
    return u"";
}

int32_t SimUtils::GetDefaultCellularDataSlotId()
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(CoreManager::DEFAULT_SLOT_ID);
    if (core != nullptr) {
        return core->GetDefaultCellularDataSlotId();
    } else {
        TELEPHONY_LOGE("core is null");
    }
    return TELEPHONY_ERROR;
}

int32_t SimUtils::SetDefaultCellularDataSlotId(int32_t slotId)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(CoreManager::DEFAULT_SLOT_ID);
    if (core != nullptr) {
        bool result = core->SetDefaultCellularDataSlotId(slotId);
        if (result) {
            return static_cast<int32_t>(DataRespondCode::SET_SUCCESS);
        } else {
            TELEPHONY_LOGE("SetDefault failed. slotId:%{public}d", slotId);
        }
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
    return static_cast<int32_t>(DataRespondCode::SET_FAILED);
}

int32_t SimUtils::GetSimNum()
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(CoreManager::DEFAULT_SLOT_ID);
    if (core != nullptr) {
        int32_t cardNumber = core->GetMaxSimCount();
        if (cardNumber < 0) {
            return MIN_SIM;
        } else {
            return cardNumber;
        }
    } else {
        TELEPHONY_LOGE("core is null");
    }
    return TELEPHONY_ERROR;
}
} // namespace Telephony
} // namespace OHOS