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
template<typename... Types>
void CellularDataHisysevent::WriteHiSysEvent(const std::string &eventName, Types... args)
{
    OHOS::HiviewDFX::HiSysEvent::EventType type;
    if (eventName == DATA_CONNECTION_STATE_EVENT) {
        type = HiviewDFX::HiSysEvent::EventType::STATISTIC;
    } else {
        TELEPHONY_LOGE("CellularDataHisysevent::WriteHiSysEvent the event name is not in the processing scope!");
        return;
    }
    OHOS::HiviewDFX::HiSysEvent::Write(DOMAIN_CELLULAR_DATA, eventName, type, args...);
}

void CellularDataHisysevent::HiSysEventWriteString(
    const std::string &eventName, const std::string eventKey, std::string eventValue)
{
    WriteHiSysEvent(eventName, eventKey, eventValue);
}

void CellularDataHisysevent::HiSysEventWriteInt(
    const std::string &eventName, const std::string eventKey, int32_t eventValue)
{
    WriteHiSysEvent(eventName, eventKey, eventValue);
}

void CellularDataHisysevent::DataConnectStateEventWrite(int32_t eventValue)
{
    HiSysEventWriteInt(DATA_CONNECTION_STATE_EVENT, DATA_CONNECTION_KEY, eventValue);
}
} // namespace Telephony
} // namespace OHOS