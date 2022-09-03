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

#include "cellular_data_roaming_observer.h"

#include "cellular_data_constant.h"
#include "cellular_data_event_code.h"
#include "cellular_data_settings_rdb_helper.h"
#include "core_manager_inner.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CellularDataRoamingObserver::CellularDataRoamingObserver(
    std::shared_ptr<AppExecFwk::EventHandler> &&cellularDataHandler, int32_t slotId)
    : cellularDataHandler_(std::move(cellularDataHandler)), slotId_(slotId)
{}

CellularDataRoamingObserver::~CellularDataRoamingObserver() = default;

void CellularDataRoamingObserver::OnChange()
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        return;
    }
    Uri uri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(slotId_));
    int value = settingHelper->GetValue(uri, std::string(CELLULAR_DATA_COLUMN_ROAMING) + std::to_string(slotId_));
    TELEPHONY_LOGI("cellular data roaming switch is %{public}d", value);
    if (cellularDataHandler_ != nullptr) {
        cellularDataHandler_->SendEvent(CellularDataEventCode::MSG_DB_SETTING_ROAMING_CHANGED, value, 0);
    }
}
} // namespace Telephony
} // namespace OHOS
