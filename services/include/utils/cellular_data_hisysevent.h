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

#ifndef CELLULAR_DATA_HISYSEVENT_H
#define CELLULAR_DATA_HISYSEVENT_H

#include <stdint.h>

#include "iosfwd"
#include "apn_item.h"
#include "telephony_hisysevent.h"

namespace OHOS {
namespace Telephony {
static const int32_t DATA_ACTIVATE_TIME = 1000;
static const int32_t SWITCH_ON = 1;
static const int32_t SWITCH_OFF = 0;

enum class DataDisconnectCause {
    ON_THE_NETWORK_SIDE = 0,
    BY_USER,
    HIGN_PRIORITY_NETWORK,
    LOST_CONNECTION,
};

class CellularDataHiSysEvent : public TelephonyHiSysEvent {
public:
    static void WriteDataDeactiveBehaviorEvent(const int32_t slotId, const DataDisconnectCause type,
        const std::string &apnType = "default");
    static void WriteDataConnectStateBehaviorEvent(const int32_t slotId, const std::string &apnType,
        const uint64_t capability, const int32_t state);
    static void WriteRoamingConnectStateBehaviorEvent(const int32_t state);
    static void WriteCellularRequestBehaviorEvent(
        const uint32_t uid, const std::string name, const uint64_t type, const int32_t state);
    static void WriteDataActivateFaultEvent(const int32_t slotId, const int32_t switchState,
        const CellularDataErrorCode errorType, const std::string &errorMsg);
    static void WriteApnInfoBehaviorEvent(const int32_t slotId, struct PdpProfile &apnData);
    void SetCellularDataActivateStartTime();
    void JudgingDataActivateTimeOut(const int32_t slotId, const int32_t switchState);

private:
    int64_t dataActivateStartTime_ = 0L;
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_DATA_HISYSEVENT_H
