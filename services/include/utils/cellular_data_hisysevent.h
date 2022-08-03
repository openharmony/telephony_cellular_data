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

#include <string>

namespace OHOS {
namespace Telephony {
// VALUE
static const int32_t INVALID_SLOT_ID = -1;
static const int32_t INVALID_PARAMETER = -1;
static const int32_t SWITCH_ON = 1;
static const int32_t SWITCH_OFF = 0;

struct CellDataActivateInfo {
    int32_t slotId;
    int32_t switchState;
    int32_t uplinkData;
    int32_t downlinkData;
    int32_t dataState;
    int32_t errorType;
};

// errorcode
enum {
    DATA_ERR_PERMISSION_ERROR = 200,
    DATA_ERR_DATABASE_WRITE_ERROR,
    DATA_ERR_PS_NOT_ATTACH,
    DATA_ERR_SIM_NOT_READY,
    DATA_ERR_CELLULAR_DATA_SLOT_ID_MISMATCH,
    DATA_ERR_ROAMING_SWITCH_OFF_AND_ROAMING,
    DATA_ERR_CALL_AND_DATA_NOT_CONCURRENCY,
    DATA_ERR_HAS_HIGHER_PRIORITY_CONNECTION,
    DATA_ERR_PDP_ACTIVATE_FAIL,
    DATA_ERR_PDP_DEACTIVATE_FAIL,
};

class CellularDataHiSysEvent {
public:
    static void DataConnectStateBehaviorEvent(const int32_t state);
    static void RoamingConnectStateBehaviorEvent(const int32_t state);
    static void DataActivateFaultEvent(const CellDataActivateInfo &info, const std::string &errorMsg);

private:
    template<typename... Types>
    static void WriteHiSysEvent(const std::string &eventName, Types... args);
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_DATA_HISYSEVENT_H
