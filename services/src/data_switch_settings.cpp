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

#include "data_switch_settings.h"

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "core_manager_inner.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
DataSwitchSettings::DataSwitchSettings(int32_t slotId) : slotId_(slotId) {}

void DataSwitchSettings::LoadSwitchValue()
{
    policyDataOn_ = true;
    std::shared_ptr<CellularDataSettingsRdbHelper> settingsRdbHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingsRdbHelper == nullptr) {
        TELEPHONY_LOGE("LoadSwitchValue settingsRdbHelper == nullptr!");
        return;
    }
    Uri userDataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    int userDataOnValue = settingsRdbHelper->GetValue(userDataEnableUri, CELLULAR_DATA_COLUMN_ENABLE);
    userDataOn_ = (userDataOnValue == static_cast<int>(DataSwitchCode::CELLULAR_DATA_ENABLED) ? true : false);

    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return;
    }
    Uri userDataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    int userDataRoamingValue = settingsRdbHelper->GetValue(
        userDataRoamingUri, std::string(CELLULAR_DATA_COLUMN_ROAMING) + std::to_string(simId));
    userDataRoaming_ =
        (userDataRoamingValue == static_cast<int>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED) ? true : false);
    TELEPHONY_LOGI("LoadSwitchValue userDataOn_:%{public}d userDataRoaming_:%{public}d policyDataOn_:%{public}d",
        userDataOn_, userDataRoaming_, policyDataOn_);
}

bool DataSwitchSettings::IsInternalDataOn() const
{
    return internalDataOn_;
}

void DataSwitchSettings::SetInternalDataOn(bool internalDataOn)
{
    internalDataOn_ = internalDataOn;
}

void DataSwitchSettings::SetUserDataOn(bool userDataOn)
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingsRdbHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingsRdbHelper == nullptr) {
        TELEPHONY_LOGE("SetUserDataOn settingsRdbHelper == nullptr!");
        return;
    }
    int value = (userDataOn ? static_cast<int>(DataSwitchCode::CELLULAR_DATA_ENABLED)
                            : static_cast<int>(DataSwitchCode::CELLULAR_DATA_DISABLED));
    TELEPHONY_LOGI("SetUserDataOn value:%{public}d", value);
    Uri userDataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    settingsRdbHelper->PutValue(userDataEnableUri, CELLULAR_DATA_COLUMN_ENABLE, value);
    userDataOn_ = userDataOn;
}

bool DataSwitchSettings::GetUserDataOn() const
{
    return userDataOn_;
}

void DataSwitchSettings::SetUserDataRoamingOn(bool dataRoamingEnabled)
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingsRdbHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingsRdbHelper == nullptr) {
        TELEPHONY_LOGE("SetUserDataRoamingOn settingsRdbHelper == nullptr!");
        return;
    }
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return;
    }
    int value = (dataRoamingEnabled ? static_cast<int>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED)
                                    : static_cast<int>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED));
    TELEPHONY_LOGI("SetUserDataRoamingOn value:%{public}d", value);
    Uri userDataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    settingsRdbHelper->PutValue(
        userDataRoamingUri, std::string(CELLULAR_DATA_COLUMN_ROAMING) + std::to_string(simId), value);
    userDataRoaming_ = dataRoamingEnabled;
}

bool DataSwitchSettings::IsUserDataRoamingOn() const
{
    return userDataRoaming_;
}

bool DataSwitchSettings::IsPolicyDataOn() const
{
    return policyDataOn_;
}

void DataSwitchSettings::SetPolicyDataOn(bool policyDataOn)
{
    policyDataOn_ = policyDataOn;
}

bool DataSwitchSettings::IsCarrierDataOn() const
{
    return carrierDataOn_;
}

void DataSwitchSettings::SetCarrierDataOn(bool carrierDataOn)
{
    carrierDataOn_ = carrierDataOn;
}

bool DataSwitchSettings::IsAllowActiveData() const
{
    if (userDataOn_ && policyDataOn_) {
        return true;
    } else {
        TELEPHONY_LOGE("Activation not allowed[user:%{public}d policy:%{public}d]", userDataOn_, policyDataOn_);
        return false;
    }
}
} // namespace Telephony
} // namespace OHOS