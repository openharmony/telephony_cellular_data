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
static const std::string DOMAIN_CELLULAR_DATA = "CELLULAR_DATA";
// EVENT
static const std::string DATA_CONNECTION_STATE_EVENT = "DATA_CONNECTION_STATE";

// KEY
static const std::string DATA_CONNECTION_KEY = "STATE";

class CellularDataHisysevent {
public:
    template<typename... Types>
    static void WriteHiSysEvent(const std::string &eventName, Types... args);
    static void HiSysEventWriteString(const std::string &eventName, const std::string eventKey, std::string eventValue);
    static void HiSysEventWriteInt(const std::string &eventName, const std::string eventKey, int32_t eventValue);
    static void DataConnectStateEventWrite(int32_t eventValue);
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_DATA_HISYSEVENT_H