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

#ifndef CELLULAR_DATA_UTILS_H
#define CELLULAR_DATA_UTILS_H

#include <vector>
#include <utility>

#include "parameter.h"

#include "cellular_data_constant.h"
#include "cellular_data_state_machine.h"

namespace OHOS {
namespace Telephony {
class CellularDataUtils {
public:
    static std::vector<AddressInfo> ParseIpAddr(const std::string &address);
    static std::vector<AddressInfo> ParseNormalIpAddr(const std::string &address);
    static std::vector<RouteInfo> ParseRoute(const std::string &address);
    static bool ParseDotIpData(const std::string &address, AddressInfo &ipInfo);
    static std::vector<std::string> Split(const std::string &input, const std::string &flag);
    static bool IsDigit(const std::string &data);
    static int32_t GetPrefixLen(const std::string &netmask, const std::string& flag);
    static int32_t GetPrefixLen(const std::vector<std::string> &netmask, const size_t start);
    static int GetDefaultMobileMtuConfig();
    static bool GetDefaultPreferApnConfig();
    static bool GetDefaultMultipleConnectionsConfig();
    static std::string ConvertRadioTechToRadioName(const int32_t radioTech);

private:
    CellularDataUtils() = default;
    ~CellularDataUtils() = default;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_UTILS_H
