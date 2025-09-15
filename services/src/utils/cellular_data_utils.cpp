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

#include "cellular_data_utils.h"

#include "telephony_common_utils.h"
#include "telephony_log_wrapper.h"
#include <charconv>

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;
std::vector<AddressInfo> CellularDataUtils::ParseIpAddr(const std::string &address)
{
    std::vector<AddressInfo> ipInfoArray;
    std::vector<std::string> ipArray = Split(address, " ");
    for (std::string &ipItem: ipArray) {
        AddressInfo ipInfo;
        std::string flag = (ipItem.find('.') == std::string::npos) ? ":" : ".";
        std::vector<std::string> ipData = Split(ipItem, "/");
        if (ipData.size() == 0) {
            TELEPHONY_LOGE("ParseIpAddr ipData is empty");
            continue;
        }
        ipInfo.ip = ipData[0];
        if (flag == ".") {
            std::vector<std::string> ipSubData = Split(ipInfo.ip, flag);
            ipInfo.type = (ipSubData.size() > MIN_IPV4_ITEM) ? INetAddr::IpType::IPV6 : INetAddr::IpType::IPV4;
            ipInfo.prefixLen = (ipSubData.size() > MIN_IPV4_ITEM) ? IPV6_BIT : IPV4_BIT;
        } else {
            ipInfo.type = INetAddr::IpType::IPV6;
            ipInfo.prefixLen = IPV6_BIT;
        }
        if ((ipData.size() >= VALID_IP_SIZE) && IsValidDecValue(ipData[1].c_str())) {
            ConvertStrToUint(ipData[1].c_str(), ipInfo.prefixLen);
        }
        ipInfoArray.push_back(ipInfo);
    }
    return ipInfoArray;
}

std::vector<AddressInfo> CellularDataUtils::ParseNormalIpAddr(const std::string &address)
{
    std::vector<AddressInfo> ipInfoArray;
    std::vector<std::string> ipArray = Split(address, " ");
    for (size_t i = 0; i < ipArray.size(); ++i) {
        AddressInfo ipInfo;
        if (ipArray[i].find(':') == std::string::npos) {
            ipInfo.prefixLen = IPV4_BIT;
            ipInfo.type = INetAddr::IpType::IPV4;
        } else {
            ipInfo.prefixLen = IPV6_BIT;
            ipInfo.type = INetAddr::IpType::IPV6;
        }
        ipInfo.ip = ipArray[i];
        ipInfoArray.push_back(ipInfo);
    }
    return ipInfoArray;
}

std::vector<RouteInfo> CellularDataUtils::ParseRoute(const std::string &address)
{
    std::vector<RouteInfo> routeInfoArray;
    std::vector<std::string> routeArray = Split(address, " ");
    for (size_t i = 0; i < routeArray.size(); ++i) {
        RouteInfo route;
        if (routeArray[i].find(':') == std::string::npos) {
            route.type = INetAddr::IpType::IPV4;
            route.destination = ROUTED_IPV4;
        } else {
            route.type = INetAddr::IpType::IPV6;
            route.destination = ROUTED_IPV6;
        }
        route.ip = routeArray[i];
        routeInfoArray.push_back(route);
    }
    return routeInfoArray;
}

std::vector<std::string> CellularDataUtils::Split(const std::string &input, const std::string &flag)
{
    std::vector<std::string> vec;
    if (input.empty()) {
        TELEPHONY_LOGD("input is null");
        return vec;
    }
    std::string::size_type start = 0;
    std::string::size_type pos = 0;
    while ((pos = input.find(flag, start)) != std::string::npos) {
        vec.push_back(input.substr(start, pos - start));
        start = pos + flag.size();
    }
    if (start != input.size()) {
        vec.push_back(input.substr(start, input.size() - start));
    }
    return vec;
}

int32_t CellularDataUtils::GetPrefixLen(const std::string &netmask, const std::string& flag)
{
    std::vector<std::string> mask = Split(netmask, flag);
    return GetPrefixLen(mask, 0);
}

int32_t CellularDataUtils::GetPrefixLen(const std::vector<std::string> &netmask, const size_t start)
{
    int32_t prefixLen = 0;
    for (size_t i = start; i < netmask.size(); ++i) {
        if (!IsValidDecValue(netmask[i].c_str())) {
            break;
        }
        int32_t value = 0;
        ConvertStrToInt(netmask[i].c_str(), value);
        int32_t maskValue = ((static_cast<uint32_t>(value)) & 0x00FF);
        if (maskValue == 0) {
            break;
        }
        while ((maskValue & 0x80) != 0) {
            prefixLen++;
            maskValue = (maskValue << 1);
        }
        if ((prefixLen % MASK_BYTE_BIT) != 0) {
            break;
        }
    }
    return prefixLen;
}

int CellularDataUtils::GetDefaultMobileMtuConfig()
{
    char mobile_mtu[MIN_BUFFER_SIZE] = {0};
    GetParameter(CONFIG_MOBILE_MTU, DEFAULT_MOBILE_MTU, mobile_mtu, MIN_BUFFER_SIZE);
    return std::atoi(mobile_mtu);
}

bool CellularDataUtils::GetDefaultPreferApnConfig()
{
    char preferApn[MIN_BUFFER_SIZE] = {0};
    GetParameter(CONFIG_PREFERAPN, DEFAULT_PREFER_APN, preferApn, MIN_BUFFER_SIZE);
    return std::atoi(preferApn);
}

bool CellularDataUtils::GetDefaultMultipleConnectionsConfig()
{
    char connections[MIN_BUFFER_SIZE] = {0};
    GetParameter(CONFIG_MULTIPLE_CONNECTIONS, DEFAULT_MULTIPLE_CONNECTIONS, connections, MIN_BUFFER_SIZE);
    return std::atoi(connections) > 0 ? true : false;
}

std::string CellularDataUtils::ConvertRadioTechToRadioName(const int32_t radioTech)
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

bool CellularDataUtils::ConvertStrToInt(const std::string& str, int32_t& value)
{
    if (str.empty()) {
        return false;
    }
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, 10);  // 10: 十进制
    bool succ = ec == std::errc{} && ptr == str.data() + str.size();
    if (!succ) {
        TELEPHONY_LOGE("ConvertStrToInt failed: str: %{public}s", str.c_str());
    }
    return succ;
}
			 
bool CellularDataUtils::ConvertStrToUint(const std::string& str, uint8_t& value)
{
    if (str.empty()) {
        return false;
    }
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, 10);  // 10: 十进制
    bool succ = ec == std::errc{} && ptr == str.data() + str.size();
    if (!succ) {
        TELEPHONY_LOGE("ConvertStrToInt failed: str: %{public}s", str.c_str());
    }
    return succ;
}
} // namespace Telephony
} // namespace OHOS