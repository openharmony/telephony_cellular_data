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

namespace OHOS {
namespace Telephony {
// EVENT
static constexpr const char *DATA_CONNECTION_STATE_EVENT = "DATA_CONNECTION_STATE";
static constexpr const char *ROAMING_DATA_CONNECTION_STATE_EVENT = "ROAMING_DATA_CONNECTION_STATE";
static constexpr const char *DATA_ACTIVATE_FAILED_EVENT = "DATA_ACTIVATE_FAILED";
static constexpr const char *DATA_DEACTIVED_EVENT = "DATA_DEACTIVED";

// KEY
static constexpr const char *MODULE_NAME_KEY = "MODULE";
static constexpr const char *SLOT_ID_KEY = "SLOT_ID";
static constexpr const char *STATE_KEY = "STATE";
static constexpr const char *DATA_SWITCH_KEY = "DATA_SWITCH";
static constexpr const char *UPLINK_DATA_KEY = "UPLINK_DATA";
static constexpr const char *DOWNLINK_DATA_KEY = "DOWNLINK_DATA";
static constexpr const char *DATASTATE_KEY = "DATASTATE";
static constexpr const char *ERROR_TYPE_KEY = "ERROR_TYPE";
static constexpr const char *ERROR_MSG_KEY = "ERROR_MSG";
static constexpr const char *TYPE_KEY = "TYPE";

// VALUE
static constexpr const char *CELLULAR_DATA_MODULE = "CELLULAR_DATA";

void CellularDataHiSysEvent::WriteDataDeactiveBehaviorEvent(const int32_t slotId, const DataDisconnectCause type)
{
    HiWriteBehaviorEvent(DATA_DEACTIVED_EVENT, SLOT_ID_KEY, slotId, TYPE_KEY, static_cast<int32_t>(type));
}

void CellularDataHiSysEvent::WriteDataConnectStateBehaviorEvent(const int32_t state)
{
    HiWriteBehaviorEvent(DATA_CONNECTION_STATE_EVENT, STATE_KEY, state);
}

void CellularDataHiSysEvent::WriteRoamingConnectStateBehaviorEvent(const int32_t state)
{
    HiWriteBehaviorEvent(ROAMING_DATA_CONNECTION_STATE_EVENT, STATE_KEY, state);
}

void CellularDataHiSysEvent::WriteDataActivateFaultEvent(
    const int32_t slotId, const int32_t switchState, const CellularDataErrorCode errorType, const std::string &errorMsg)
{
    HiWriteFaultEvent(DATA_ACTIVATE_FAILED_EVENT, MODULE_NAME_KEY, CELLULAR_DATA_MODULE, SLOT_ID_KEY, slotId,
        DATA_SWITCH_KEY, switchState, UPLINK_DATA_KEY, INVALID_PARAMETER, DOWNLINK_DATA_KEY, INVALID_PARAMETER,
        DATASTATE_KEY, INVALID_PARAMETER, ERROR_TYPE_KEY, static_cast<int32_t>(errorType), ERROR_MSG_KEY, errorMsg);
}

void CellularDataHiSysEvent::SetCellularDataActivateStartTime()
{
    dataActivateStartTime_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
}

void CellularDataHiSysEvent::JudgingDataActivateTimeOut(const int32_t slotId, const int32_t switchState)
{
    int64_t dataActivateEndTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    if (dataActivateEndTime - dataActivateStartTime_ > DATA_ACTIVATE_TIME) {
        WriteDataActivateFaultEvent(slotId, switchState, CellularDataErrorCode::DATA_ERROR_DATA_ACTIVATE_TIME_OUT,
            "data activate time out " + std::to_string(dataActivateEndTime - dataActivateStartTime_));
    }
}
} // namespace Telephony
} // namespace OHOS
