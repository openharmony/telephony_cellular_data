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

#include "cellular_data_setting_observer.h"

#include "cellular_data_constant.h"
#include "cellular_data_event_code.h"
#include "cellular_data_settings_rdb_helper.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CellularDataSettingObserver::CellularDataSettingObserver(
    std::shared_ptr<AppExecFwk::EventHandler> &&cellularDataHandler)
    : cellularDataHandler_(std::move(cellularDataHandler))
{}

CellularDataSettingObserver::~CellularDataSettingObserver() = default;

void CellularDataSettingObserver::OnChange()
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        return;
    }
    Uri uri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    int value = static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED);
    if (settingHelper->GetValue(uri, CELLULAR_DATA_COLUMN_ENABLE, value) != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("OnChange GetValue failed!");
        return;
    }
    TELEPHONY_LOGI("cellular data switch is %{public}d", value);
    if (cellularDataHandler_ != nullptr) {
        cellularDataHandler_->SendEvent(CellularDataEventCode::MSG_DB_SETTING_ENABLE_CHANGED, value, 0);
    }
}
} // namespace Telephony
} // namespace OHOS