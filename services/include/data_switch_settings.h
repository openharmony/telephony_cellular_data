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

#ifndef DATA_SWITCH_SETTINGS_H
#define DATA_SWITCH_SETTINGS_H

#include <stdint.h>

namespace OHOS {
namespace Telephony {
class DataSwitchSettings {
public:
    explicit DataSwitchSettings(int32_t slotId);
    ~DataSwitchSettings() = default;
    void LoadSwitchValue();
    bool IsInternalDataOn() const;
    void SetInternalDataOn(bool internalDataOn);
    bool IsPolicyDataOn() const;
    void SetPolicyDataOn(bool policyDataOn);
    bool IsCarrierDataOn() const;
    void SetCarrierDataOn(bool carrierDataOn);
    bool IsAllowActiveData() const;
    bool IsUserDataOn();
    bool IsUserDataRoamingOn();
    void UpdateUserDataRoamingOn(bool dataRoaming);
    int32_t SetUserDataOn(bool userDataOn);
    int32_t SetAnySimDetected(int32_t simDetected);
    int32_t SetIntelliSwitchOn(bool userSwitchOn);
    int32_t SetUserDataRoamingOn(bool dataRoamingEnabled);
    int32_t QueryIntelligenceSwitchStatus(bool &switchEnabled);
    int32_t QueryUserDataStatus(bool &dataEnabled);
    int32_t QueryAnySimDetectedStatus(int32_t simDetected);
    int32_t QueryUserDataRoamingStatus(bool &dataRoamingEnabled);
    int32_t GetLastQryRet();

private:
    bool internalDataOn_ = true;
    bool userDataOn_ = true;
    bool userDataRoaming_ = false;
    bool policyDataOn_ = true;
    bool carrierDataOn_ = false;
    bool intelliSwitchOn_ = false;
    const int32_t slotId_;
    int32_t lastQryRet_ = -1;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_SWITCH_SETTINGS_H
