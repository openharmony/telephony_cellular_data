/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "cellular_data_hisysevent.h"

#include "hisysevent.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
static constexpr const char *DOMAIN_CELLULAR_DATA = "CELLULAR_DATA";
// EVENT
static constexpr const char *DATA_CONNECTION_STATE_EVENT = "DATA_CONNECTION_STATE";
static constexpr const char *DATA_DEACTIVED_EVENT = "DATA_DEACTIVED";
static constexpr const char *ROAMING_DATA_CONNECTION_STATE_EVENT = "ROAMING_DATA_CONNECTION_STATE";
static constexpr const char *DATA_ACTIVATE_FAILED_EVENT = "DATA_ACTIVATE_FAILED";

// KEY
static constexpr const char *DATA_CONNECTION_KEY = "STATE";
static constexpr const char *SLOT_ID_KEY = "SLOT_ID";
static constexpr const char *DATA_SWITCH_KEY = "DATA_SWITCH";
static constexpr const char *UPLINK_DATA_KEY = "UPLINK_DATA";
static constexpr const char *DOWNLINK_DATA_KEY = "DOWNLINK_DATA";
static constexpr const char *DATASTATE_KEY = "DATASTATE";
static constexpr const char *ERROR_TYPE_KEY = "ERROR_TYPE";
static constexpr const char *ERROR_MSG_KEY = "ERROR_MSG";

template<typename... Types>
void CellularDataHiSysEvent::WriteHiSysEvent(const std::string &eventName, Types... args)
{
    OHOS::HiviewDFX::HiSysEvent::EventType type;
    if (eventName == DATA_CONNECTION_STATE_EVENT || eventName == DATA_DEACTIVED_EVENT ||
        eventName == ROAMING_DATA_CONNECTION_STATE_EVENT) {
        type = HiviewDFX::HiSysEvent::EventType::BEHAVIOR;
    } else if (eventName == DATA_ACTIVATE_FAILED_EVENT) {
        type = HiviewDFX::HiSysEvent::EventType::FAULT;
    } else {
        TELEPHONY_LOGE("CellularDataHiSysEvent::WriteHiSysEvent the event name is not in the processing scope!");
        return;
    }
    OHOS::HiviewDFX::HiSysEvent::Write(DOMAIN_CELLULAR_DATA, eventName, type, args...);
}

void CellularDataHiSysEvent::DataConnectStateBehaviorEvent(const int32_t state)
{
    WriteHiSysEvent(DATA_CONNECTION_STATE_EVENT, DATA_CONNECTION_KEY, state);
}

void CellularDataHiSysEvent::RoamingConnectStateBehaviorEvent(const int32_t state)
{
    WriteHiSysEvent(ROAMING_DATA_CONNECTION_STATE_EVENT, DATA_CONNECTION_KEY, state);
}

void CellularDataHiSysEvent::DataActivateFaultEvent(const CellDataActivateInfo &info, const std::string &errorMsg)
{
    WriteHiSysEvent(DATA_ACTIVATE_FAILED_EVENT, SLOT_ID_KEY, info.slotId, DATA_SWITCH_KEY, info.switchState,
        UPLINK_DATA_KEY, info.uplinkData, DOWNLINK_DATA_KEY, info.downlinkData, DATASTATE_KEY, info.dataState,
        ERROR_TYPE_KEY, info.errorType, ERROR_MSG_KEY, errorMsg);
}
} // namespace Telephony
} // namespace OHOS
