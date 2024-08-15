/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_NET_CONN_SERVICE_H
#define MOCK_NET_CONN_SERVICE_H

#include <gmock/gmock.h>
#include "i_net_conn_service.h"
#include "net_all_capabilities.h"

namespace OHOS {
namespace NetManagerStandard {

class MockINetConnService : public INetConnService {
public:
    MockINetConnService() = default;
    ~MockINetConnService() = default;

    MOCK_METHOD(sptr<IRemoteObject>, AsObject, ());
    MOCK_METHOD(int32_t, SystemReady, ());
    MOCK_METHOD(int32_t, SetInternetPermission, (uint32_t uid, uint8_t allow));
    MOCK_METHOD(int32_t, RegisterNetSupplier,
        (NetBearType bearerType, const std::string &ident, const std::set<NetCap> &netCaps, uint32_t &supplierId));
    MOCK_METHOD(int32_t, UnregisterNetSupplier, (uint32_t supplierId));
    MOCK_METHOD(
        int32_t, RegisterNetSupplierCallback, (uint32_t supplierId, const sptr<INetSupplierCallback> &callback));
    MOCK_METHOD(int32_t, RegisterNetConnCallback, (const sptr<INetConnCallback> callback));
    MOCK_METHOD(int32_t, RegisterNetConnCallback,
        (const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> callback, const uint32_t &timeoutMS));
    MOCK_METHOD(int32_t, RequestNetConnection,
        (const sptr<NetSpecifier> netSpecifier, const sptr<INetConnCallback> callback, const uint32_t timeoutMS));
    MOCK_METHOD(int32_t, UnregisterNetConnCallback, (const sptr<INetConnCallback> &callback));
    MOCK_METHOD(int32_t, UpdateNetStateForTest, (const sptr<NetSpecifier> &netSpecifier, int32_t netState));
    MOCK_METHOD(int32_t, UpdateNetSupplierInfo, (uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo));
    MOCK_METHOD(int32_t, UpdateNetLinkInfo, (uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo));
    MOCK_METHOD(int32_t, GetIfaceNames, (NetBearType bearerType, std::list<std::string> &ifaceNames));
    MOCK_METHOD(
        int32_t, GetIfaceNameByType, (NetBearType bearerType, const std::string &ident, std::string &ifaceName));
    MOCK_METHOD(int32_t, GetIfaceNameIdentMaps,
        (NetBearType bearerType, (SafeMap<std::string, std::string> & ifaceNameIdentMaps)));
    MOCK_METHOD(int32_t, RegisterNetDetectionCallback, (int32_t netId, const sptr<INetDetectionCallback> &callback));
    MOCK_METHOD(int32_t, UnRegisterNetDetectionCallback, (int32_t netId, const sptr<INetDetectionCallback> &callback));
    MOCK_METHOD(int32_t, NetDetection, (int32_t netId));
    MOCK_METHOD(int32_t, GetDefaultNet, (int32_t & netId));
    MOCK_METHOD(int32_t, HasDefaultNet, (bool &flag));
    MOCK_METHOD(int32_t, GetAddressesByName, (const std::string &host, int32_t netId, std::vector<INetAddr> &addrList));
    MOCK_METHOD(int32_t, GetAddressByName, (const std::string &host, int32_t netId, INetAddr &addr));
    MOCK_METHOD(int32_t, GetSpecificNet, (NetBearType bearerType, std::list<int32_t> &netIdList));
    MOCK_METHOD(int32_t, GetAllNets, (std::list<int32_t> & netIdList));
    MOCK_METHOD(int32_t, GetSpecificUidNet, (int32_t uid, int32_t &netId));
    MOCK_METHOD(int32_t, GetConnectionProperties, (int32_t netId, NetLinkInfo &info));
    MOCK_METHOD(int32_t, GetNetCapabilities, (int32_t netId, NetManagerStandard::NetAllCapabilities &netAllCap));
    MOCK_METHOD(int32_t, BindSocket, (int32_t socketFd, int32_t netId));
    MOCK_METHOD(int32_t, SetAirplaneMode, (bool state));
    MOCK_METHOD(int32_t, IsDefaultNetMetered, (bool &isMetered));
    MOCK_METHOD(int32_t, SetGlobalHttpProxy, (const HttpProxy &httpProxy));
    MOCK_METHOD(int32_t, GetGlobalHttpProxy, (HttpProxy & httpProxy));
    MOCK_METHOD(int32_t, GetDefaultHttpProxy, (int32_t bindNetId, HttpProxy &httpProxy));
    MOCK_METHOD(int32_t, GetNetIdByIdentifier, (const std::string &ident, std::list<int32_t> &netIdList));
    MOCK_METHOD(int32_t, SetAppNet, (int32_t netId));
    MOCK_METHOD(int32_t, RegisterNetInterfaceCallback, (const sptr<INetInterfaceStateCallback> &callback));
    MOCK_METHOD(int32_t, GetNetInterfaceConfiguration,
        (const std::string &iface, NetManagerStandard::NetInterfaceConfiguration &config));
    MOCK_METHOD(int32_t, AddNetworkRoute,
        (int32_t netId, const std::string &ifName, const std::string &destination, const std::string &nextHop));
    MOCK_METHOD(int32_t, RemoveNetworkRoute,
        (int32_t netId, const std::string &ifName, const std::string &destination, const std::string &nextHop));
    MOCK_METHOD(
        int32_t, AddInterfaceAddress, (const std::string &ifName, const std::string &ipAddr, int32_t prefixLength));
    MOCK_METHOD(
        int32_t, DelInterfaceAddress, (const std::string &ifName, const std::string &ipAddr, int32_t prefixLength));
    MOCK_METHOD(
        int32_t, AddStaticArp, (const std::string &ipAddr, const std::string &macAddr, const std::string &ifName));
    MOCK_METHOD(
        int32_t, DelStaticArp, (const std::string &ipAddr, const std::string &macAddr, const std::string &ifName));
    MOCK_METHOD(int32_t, RegisterSlotType, (uint32_t supplierId, int32_t type));
    MOCK_METHOD(int32_t, GetSlotType, (std::string & type));
    MOCK_METHOD(int32_t, FactoryResetNetwork, ());
    MOCK_METHOD(int32_t, RegisterNetFactoryResetCallback, (const sptr<INetFactoryResetCallback> &callback));
    MOCK_METHOD(int32_t, IsPreferCellularUrl, (const std::string &url, bool &preferCellular));
    MOCK_METHOD(int32_t, RegisterPreAirplaneCallback, (const sptr<IPreAirplaneCallback> callback));
    MOCK_METHOD(int32_t, UnregisterPreAirplaneCallback, (const sptr<IPreAirplaneCallback> callback));
    MOCK_METHOD(int32_t, UpdateSupplierScore, (NetBearType bearerType, bool isBetter, uint32_t &supplierId));
    MOCK_METHOD(int32_t, EnableVnicNetwork, (const sptr<NetLinkInfo> &netLinkInfo, const std::set<int32_t> &uids));
    MOCK_METHOD(int32_t, DisableVnicNetwork, ());
    MOCK_METHOD(int32_t, EnableDistributedClientNet, (const std::string &virnicAddr, const std::string &iif));
    MOCK_METHOD(int32_t, EnableDistributedServerNet,
        (const std::string &iif, const std::string &devIface, const std::string &dstAddr));
    MOCK_METHOD(int32_t, DisableDistributedNet, (bool isServer));
};

}  // namespace NetManagerStandard
}  // namespace OHOS
#endif  // MOCK_NET_CONN_SERVICE_H