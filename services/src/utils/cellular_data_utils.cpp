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
            ipInfo.prefixLen = std::stoi(ipData[1].c_str());
        }
        ipInfoArray.push_back(ipInfo);
    }
    return ipInfoArray;
}

bool CellularDataUtils::ParseDotIpData(const std::string &address, AddressInfo &ipInfo)
{
    bool bOnlyAddress = false;
    int prefixLen = 0;
    // a1.a2.a3.a4.m1.m2.m3.m4
    // a1.a2.a3.a4.a5.a6.a7.a8.a9.a10.a11.a12.a13.a14.a15.a16.m1.m2.m3.m4.m5.m6.m7.m8.m9.m10.m11.m12.m13.m14.m15.m16
    std::vector<std::string> ipSubData = Split(ipInfo.ip, ".");
    if (ipSubData.size() > MIN_IPV6_ITEM) {
        ipInfo.type = INetAddr::IpType::IPV6;
        ipInfo.ip = ipSubData[0];
        for (int32_t i = 1; i < MIN_IPV6_ITEM; ++i) {
            ipInfo.ip += "." + ipSubData[i];
        }
        ipInfo.netMask = ipSubData[MIN_IPV6_ITEM];
        for (size_t j = MIN_IPV6_ITEM; j < ipSubData.size(); ++j) {
            ipInfo.netMask += "." + ipSubData[j];
        }
        prefixLen = GetPrefixLen(ipSubData, MIN_IPV6_ITEM);
    } else if (ipSubData.size() > MAX_IPV4_ITEM) {
        ipInfo.type = INetAddr::IpType::IPV6;
        bOnlyAddress = true;
    } else if (ipSubData.size() > MIN_IPV4_ITEM) {
        ipInfo.type = INetAddr::IpType::IPV4;
        ipInfo.ip = ipSubData[0];
        for (int32_t i = 1; i < MIN_IPV4_ITEM; ++i) {
            ipInfo.ip += "." + ipSubData[i];
        }
        ipInfo.netMask = ipSubData[MIN_IPV4_ITEM];
        for (size_t j = MIN_IPV4_ITEM; j < ipSubData.size(); ++j) {
            ipInfo.netMask += "." + ipSubData[j];
        }
        prefixLen = GetPrefixLen(ipSubData, MIN_IPV4_ITEM);
    } else {
        ipInfo.type = INetAddr::IpType::IPV4;
        bOnlyAddress = true;
    }
    ipInfo.prefixLen = prefixLen;
    return bOnlyAddress;
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
        TELEPHONY_LOGE("input is null");
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

bool CellularDataUtils::IsDigit(const std::string &data)
{
    if (data.empty()) {
        TELEPHONY_LOGE("data is null");
        return false;
    }
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[0] == '-') {
            continue;
        }
        if (!isdigit(data[i])) {
            TELEPHONY_LOGE("data %{public}s is not digit!", data.c_str());
            return false;
        }
    }
    return true;
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
        int32_t maskValue = (std::stoi(netmask[i].c_str()) & 0x00FF);
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
} // namespace Telephony
} // namespace OHOS