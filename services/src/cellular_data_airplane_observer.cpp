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

#include "cellular_data_airplane_observer.h"

#include "cellular_data_constant.h"
#include "cellular_data_event_code.h"
#include "cellular_data_settings_rdb_helper.h"
#include "core_manager_inner.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {

void CellularDataAirplaneObserver::OnChange()
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        return;
    }
    Uri uri(CELLULAR_DATA_AIRPLANE_MODE_URI);
    int value = 0;
    if (settingHelper->GetValue(uri, CELLULAR_DATA_COLUMN_AIRPLANE, value) != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("GetValue failed!");
        return;
    }
    isAirplaneModeOn_ = value;
    TELEPHONY_LOGI("airplane switch is %{public}d", value);
}

bool CellularDataAirplaneObserver::IsAirplaneModeOn()
{
    return isAirplaneModeOn_;
}
} // namespace Telephony
} // namespace OHOS