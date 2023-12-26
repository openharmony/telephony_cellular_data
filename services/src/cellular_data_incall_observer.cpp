/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "cellular_data_incall_observer.h"

#include "cellular_data_constant.h"
#include "cellular_data_event_code.h"
#include "cellular_data_settings_rdb_helper.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CellularDataIncallObserver::CellularDataIncallObserver(std::weak_ptr<TelEventHandler> &&cellularDataHandler)
    : cellularDataHandler_(std::move(cellularDataHandler))
{}

CellularDataIncallObserver::~CellularDataIncallObserver() = default;

void CellularDataIncallObserver::OnChange()
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        return;
    }
    Uri uri(CELLULAR_DATA_SETTING_DATA_INCALL_URI);
    int value = static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
    if (settingHelper->GetValue(uri, CELLULAR_DATA_COLUMN_INCALL, value) != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetValue failed!");
        return;
    }
    TELEPHONY_LOGI("cellular data incall is %{public}d", value);
    auto cellularDataHandler = cellularDataHandler_.lock();
    if (cellularDataHandler != nullptr) {
        cellularDataHandler->SendEvent(CellularDataEventCode::MSG_DB_SETTING_INCALL_CHANGED, value, 0);
    }
}
} // namespace Telephony
} // namespace OHOS