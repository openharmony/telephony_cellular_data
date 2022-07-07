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

#include "cellular_data_hisysevent.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void DataSwitchSettings::LoadSwitchValue()
{
    userDataOn_ = false;
    userDataRoaming_ = false;
    policyDataOn_ = true;
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
    userDataOn_ = userDataOn;
    CellularDataHisysevent::DataConnectStateEventWrite(userDataOn);
}

bool DataSwitchSettings::GetUserDataOn() const
{
    return userDataOn_;
}

void DataSwitchSettings::SetUserDataRoamingOn(bool dataRoamingEnabled)
{
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