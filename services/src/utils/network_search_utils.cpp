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

#include "network_search_utils.h"

#include "string_ex.h"

#include "telephony_log_wrapper.h"

#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
bool NetworkSearchUtils::GetRoamingState(int32_t slotId)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        int32_t result = core->GetPsRoamingState(slotId);
        if (result > 0) {
            return true;
        }
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
    return false;
}

bool NetworkSearchUtils::GetAttachedState(int32_t slotId)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core == nullptr) {
        TELEPHONY_LOGE("network search core is null");
        return false;
    }
    int32_t result = core->GetPsRegState(slotId);
    if (result == (int32_t)RegServiceState::REG_STATE_IN_SERVICE) {
        return true;
    }
    return false;
}

int32_t NetworkSearchUtils::GetPsRadioTech(int32_t slotId)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        return core->GetPsRadioTech(slotId);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
    return (int32_t)RadioTech::RADIO_TECHNOLOGY_UNKNOWN;
}

void NetworkSearchUtils::SetRadioState(
    int32_t slotId, int fun, int rst, const AppExecFwk::InnerEvent::Pointer &response)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->SetRadioState(fun, rst, response);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

std::string NetworkSearchUtils::GetOperatorNumeric(int32_t slotId)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        std::u16string numeric = core->GetSimOperatorNumeric(slotId);
        return Str16ToStr8(numeric);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
        return "";
    }
}

void NetworkSearchUtils::DcPhysicalLinkActiveUpdate(int32_t slotId, bool isActive)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->DcPhysicalLinkActiveUpdate(slotId, isActive);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

std::string NetworkSearchUtils::ConvertRadioTechToRadioName(const int32_t radioTech)
{
    std::string radioName = "unknown";
    switch (radioTech) {
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_UNKNOWN):
            radioName = "unknown";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_GSM):
            radioName = "EDGE";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_1XRTT):
            radioName = "1xRTT";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_WCDMA):
            radioName = "UMTS";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_HSPA):
            radioName = "HSPA";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_HSPAP):
            radioName = "HSPAP";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_TD_SCDMA):
            radioName = "TD-SCDMA";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_EVDO):
            radioName = "EVDO";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_EHRPD):
            radioName = "eHRPD";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE):
            radioName = "LTE";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_LTE_CA):
            radioName = "LTE_CA";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_IWLAN):
            radioName = "IWAN";
            break;
        case static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_NR):
            radioName = "NR";
            break;
        default:
            break;
    }
    return radioName;
}
} // namespace Telephony
} // namespace OHOS