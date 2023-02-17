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
#include "cellular_data_error.h"
#include "cellular_data_settings_rdb_helper.h"
#include "core_manager_inner.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
DataSwitchSettings::DataSwitchSettings(int32_t slotId) : slotId_(slotId) {}

void DataSwitchSettings::LoadSwitchValue()
{
    bool dataEnabled = false;
    bool dataRoamingEnabled = false;
    IsUserDataOn(dataEnabled);
    IsUserDataRoamingOn(dataRoamingEnabled);
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

int32_t DataSwitchSettings::SetUserDataOn(bool userDataOn)
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingsRdbHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingsRdbHelper == nullptr) {
        TELEPHONY_LOGE("SetUserDataOn settingsRdbHelper == nullptr!");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    int value = (userDataOn ? static_cast<int>(DataSwitchCode::CELLULAR_DATA_ENABLED)
                            : static_cast<int>(DataSwitchCode::CELLULAR_DATA_DISABLED));
    TELEPHONY_LOGI("SetUserDataOn value:%{public}d", value);
    Uri userDataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    int32_t result = settingsRdbHelper->PutValue(userDataEnableUri, CELLULAR_DATA_COLUMN_ENABLE, value);
    if (result == TELEPHONY_ERR_SUCCESS) {
        userDataOn_ = userDataOn;
    }
    return result;
}

int32_t DataSwitchSettings::IsUserDataOn(bool &dataEnabled)
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingsRdbHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingsRdbHelper == nullptr) {
        TELEPHONY_LOGE("IsUserDataOn settingsRdbHelper == nullptr!");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    Uri userDataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    int32_t userDataEnable = static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
    int32_t ret = settingsRdbHelper->GetValue(userDataEnableUri, CELLULAR_DATA_COLUMN_ENABLE, userDataEnable);
    if (ret != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("GetValue failed!");
        return ret;
    }
    userDataOn_ = (userDataEnable == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED));
    dataEnabled = userDataOn_;
    return TELEPHONY_ERR_SUCCESS;
}

int32_t DataSwitchSettings::SetUserDataRoamingOn(bool dataRoamingEnabled)
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingsRdbHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingsRdbHelper == nullptr) {
        TELEPHONY_LOGE("SetUserDataRoamingOn settingsRdbHelper is nullptr!");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return TELEPHONY_ERR_SLOTID_INVALID;
    }
    int value = (dataRoamingEnabled ? static_cast<int>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED)
                                    : static_cast<int>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED));
    TELEPHONY_LOGI("SetUserDataRoamingOn value:%{public}d", value);
    Uri userDataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    int32_t result = settingsRdbHelper->PutValue(
        userDataRoamingUri, std::string(CELLULAR_DATA_COLUMN_ROAMING) + std::to_string(simId), value);
    if (result == TELEPHONY_ERR_SUCCESS) {
        userDataRoaming_ = dataRoamingEnabled;
    }
    return result;
}

int32_t DataSwitchSettings::IsUserDataRoamingOn(bool &dataRoamingEnabled)
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingsRdbHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingsRdbHelper == nullptr) {
        TELEPHONY_LOGE("IsUserDataRoamingOn settingsRdbHelper == nullptr!");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }

    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: IsUserDataRoamingOn invalid sim id %{public}d", slotId_, simId);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    Uri userDataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    int32_t userDataRoamingValue = static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED);
    int32_t ret = settingsRdbHelper->GetValue(
        userDataRoamingUri, std::string(CELLULAR_DATA_COLUMN_ROAMING) + std::to_string(simId), userDataRoamingValue);
    if (ret != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("GetValue failed!");
        return ret;
    }
    userDataRoaming_ = (userDataRoamingValue == static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED));
    dataRoamingEnabled = userDataRoaming_;
    return TELEPHONY_ERR_SUCCESS;
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