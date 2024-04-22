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

#include "cellular_data_state_machine.h"

#include <cinttypes>
#include <string_ex.h>

#include "activating.h"
#include "active.h"
#include "apn_manager.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "default.h"
#include "disconnecting.h"
#include "inactive.h"
#include "radio_event.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
using namespace NetManagerStandard;
namespace Telephony {
static const int32_t INVALID_MTU_VALUE = -1;
bool CellularDataStateMachine::IsInactiveState() const
{
    return currentState_ == inActiveState_;
}

bool CellularDataStateMachine::IsActivatingState() const
{
    return currentState_ == activatingState_;
}

bool CellularDataStateMachine::IsDisconnectingState() const
{
    return currentState_ == disconnectingState_;
}

bool CellularDataStateMachine::IsActiveState() const
{
    return currentState_ == activeState_;
}

bool CellularDataStateMachine::IsDefaultState() const
{
    return currentState_ == defaultState_;
}

void CellularDataStateMachine::SetCapability(uint64_t capability)
{
    capability_ = capability;
}

uint64_t CellularDataStateMachine::GetCapability() const
{
    return capability_;
}

int32_t CellularDataStateMachine::GetCid() const
{
    return cid_;
}

void CellularDataStateMachine::SetCid(const int32_t cid)
{
    cid_ = cid;
}

int32_t CellularDataStateMachine::GetSlotId() const
{
    if (cdConnectionManager_ == nullptr) {
        TELEPHONY_LOGE("cdConnectionManager_ is null");
        return DEFAULT_SIM_SLOT_ID;
    }
    return cdConnectionManager_->GetSlotId();
}

sptr<ApnItem> CellularDataStateMachine::GetApnItem() const
{
    return apnItem_;
}

void CellularDataStateMachine::DoConnect(const DataConnectionParams &connectionParams)
{
    if (connectionParams.GetApnHolder() == nullptr) {
        TELEPHONY_LOGE("apnHolder is null");
        return;
    }
    apnId_ = ApnManager::FindApnIdByApnName(connectionParams.GetApnHolder()->GetApnType());
    sptr<ApnItem> apn = connectionParams.GetApnHolder()->GetCurrentApn();
    apnItem_ = apn;
    if (apnItem_ == nullptr) {
        TELEPHONY_LOGE("apnItem is null");
        return;
    }
    const int32_t slotId = GetSlotId();
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId, radioTech);
    ActivateDataParam activeDataParam;
    activeDataParam.param = connectId_;
    activeDataParam.radioTechnology = radioTech;
    activeDataParam.allowRoaming = connectionParams.GetRoamingState();
    activeDataParam.isRoaming = connectionParams.GetUserDataRoaming();
    activeDataParam.dataProfile.profileId = apn->attr_.profileId_;
    activeDataParam.dataProfile.apn = apn->attr_.apn_;
    activeDataParam.dataProfile.protocol = apn->attr_.protocol_;
    activeDataParam.dataProfile.verType = apn->attr_.authType_;
    activeDataParam.dataProfile.userName = apn->attr_.user_;
    activeDataParam.dataProfile.password = apn->attr_.password_;
    activeDataParam.dataProfile.roamingProtocol = apn->attr_.roamingProtocol_;
    TELEPHONY_LOGI("Slot%{public}d: Activate PDP context (%{public}d, %{public}s, %{public}s, %{public}s)", slotId,
        apn->attr_.profileId_, apn->attr_.apn_, apn->attr_.protocol_, apn->attr_.types_);
    int32_t result = CoreManagerInner::GetInstance().ActivatePdpContext(slotId, RadioEvent::RADIO_RIL_SETUP_DATA_CALL,
        activeDataParam, stateMachineEventHandler_);
    if (result != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: Activate PDP context failed", slotId);
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(
            slotId, SWITCH_ON, CellularDataErrorCode::DATA_ERROR_PDP_ACTIVATE_FAIL, "Activate PDP context failed");
    }
    if (stateMachineEventHandler_ == nullptr) {
        TELEPHONY_LOGE("stateMachineEventHandler_ is nullptr");
        return;
    }
    stateMachineEventHandler_->SendEvent(
        CellularDataEventCode::MSG_CONNECT_TIMEOUT_CHECK, connectId_, CONNECTION_DISCONNECTION_TIMEOUT);
}

void CellularDataStateMachine::FreeConnection(const DataDisconnectParams &params)
{
    const int32_t slotId = GetSlotId();
    int32_t apnId = ApnManager::FindApnIdByApnName(params.GetApnType());
    TELEPHONY_LOGI("Slot%{public}d: Deactivate PDP context cid:%{public}d type:%{public}s id:%{public}d",
        slotId, cid_, params.GetApnType().c_str(), apnId);
    DeactivateDataParam deactivateDataParam;
    deactivateDataParam.param = connectId_;
    deactivateDataParam.cid = cid_;
    deactivateDataParam.reason = static_cast<int32_t>(params.GetReason());
    int32_t result = CoreManagerInner::GetInstance().DeactivatePdpContext(slotId,
        RadioEvent::RADIO_RIL_DEACTIVATE_DATA_CALL, deactivateDataParam, stateMachineEventHandler_);
    if (result != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: Deactivate PDP context failed", slotId);
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(
            slotId, SWITCH_OFF, CellularDataErrorCode::DATA_ERROR_PDP_DEACTIVATE_FAIL, "Deactivate PDP context failed");
    }
    if (stateMachineEventHandler_ == nullptr) {
        TELEPHONY_LOGE("stateMachineEventHandler_ is nullptr");
        return;
    }
    stateMachineEventHandler_->SendEvent(
        CellularDataEventCode::MSG_DISCONNECT_TIMEOUT_CHECK, connectId_, CONNECTION_DISCONNECTION_TIMEOUT);
}

bool CellularDataStateMachine::operator==(const CellularDataStateMachine &stateMachine) const
{
    return this->GetCid() == stateMachine.GetCid();
}

void CellularDataStateMachine::Init()
{
    activeState_ = std::make_unique<Active>(
        std::weak_ptr<CellularDataStateMachine>(shared_from_this()), "Active").release();
    inActiveState_ = std::make_unique<Inactive>(
        std::weak_ptr<CellularDataStateMachine>(shared_from_this()), "Inactive").release();
    activatingState_ = std::make_unique<Activating>(
        std::weak_ptr<CellularDataStateMachine>(shared_from_this()), "Activating").release();
    disconnectingState_ = std::make_unique<Disconnecting>(
        std::weak_ptr<CellularDataStateMachine>(shared_from_this()), "Disconnecting").release();
    defaultState_ = std::make_unique<Default>(
        std::weak_ptr<CellularDataStateMachine>(shared_from_this()), "Default").release();
    netSupplierInfo_ = std::make_unique<NetSupplierInfo>().release();
    netLinkInfo_ = std::make_unique<NetLinkInfo>().release();
    if (activeState_ == nullptr || inActiveState_ == nullptr || activatingState_ == nullptr ||
        disconnectingState_ == nullptr || defaultState_ == nullptr || netSupplierInfo_ == nullptr ||
        netLinkInfo_ == nullptr) {
        TELEPHONY_LOGE("memory allocation failed");
        return;
    }
    activeState_->SetParentState(defaultState_);
    inActiveState_->SetParentState(defaultState_);
    activatingState_->SetParentState(defaultState_);
    disconnectingState_->SetParentState(defaultState_);
    StateMachine::SetOriginalState(inActiveState_);
    StateMachine::Start();
}

void CellularDataStateMachine::SetCurrentState(const sptr<State> &&state)
{
    currentState_ = std::move(state);
}

sptr<State> CellularDataStateMachine::GetCurrentState() const
{
    return currentState_;
}

bool CellularDataStateMachine::HasMatchedIpTypeAddrs(uint8_t ipType, uint8_t ipInfoArraySize,
    std::vector<AddressInfo> ipInfoArray)
{
    for (int i = 0; i < ipInfoArraySize; i++) {
        if (ipInfoArray[i].type == ipType) {
            return true;
        }
    }
    return false;
}

std::string CellularDataStateMachine::GetIpType(std::vector<AddressInfo> ipInfoArray)
{
    uint8_t ipInfoArraySize = ipInfoArray.size();
    uint8_t ipv4Type = INetAddr::IpType::IPV4;
    uint8_t ipv6Type = INetAddr::IpType::IPV6;
    std::string result;
    if (HasMatchedIpTypeAddrs(ipv4Type, ipInfoArraySize, ipInfoArray) &&
        HasMatchedIpTypeAddrs(ipv6Type, ipInfoArraySize, ipInfoArray)) {
        result = "IPV4V6";
    } else if (HasMatchedIpTypeAddrs(ipv4Type, ipInfoArraySize, ipInfoArray)) {
        result = "IPV4";
    } else if (HasMatchedIpTypeAddrs(ipv6Type, ipInfoArraySize, ipInfoArray)) {
        result = "IPV6";
    } else {
        TELEPHONY_LOGE("Ip type not match");
    }
    return result;
}

std::string CellularDataStateMachine::GetIpType()
{
    return ipType_;
}

void CellularDataStateMachine::GetMtuSizeFromOpCfg(int32_t &mtuSize, int32_t slotId)
{
    std::string mtuString = "";
    int32_t mtuValue = INVALID_MTU_VALUE;
    OperatorConfig configsForMtuSize;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId, configsForMtuSize);
    if (configsForMtuSize.stringValue.find(KEY_MTU_SIZE_STRING) != configsForMtuSize.stringValue.end()) {
        mtuString = configsForMtuSize.stringValue[KEY_MTU_SIZE_STRING];
    }
    std::vector<std::string> mtuArray = CellularDataUtils::Split(mtuString, ";");
    for (std::string &ipTypeArray : mtuArray) {
        std::vector<std::string> mtuIpTypeArray = CellularDataUtils::Split(ipTypeArray, ":");
        if (mtuIpTypeArray.size() != VALID_VECTOR_SIZE || mtuIpTypeArray[0].empty() || mtuIpTypeArray[1].empty()) {
            TELEPHONY_LOGE("mtu size string is invalid");
            break;
        }
        std::string ipTypeString = mtuIpTypeArray[0];
        StrToInt(mtuIpTypeArray[1], mtuValue);
        if (mtuValue == INVALID_MTU_VALUE) {
            TELEPHONY_LOGE("mtu values is invalid");
            break;
        }
        if (!ipTypeString.empty() && ipTypeString == ipType_) {
            mtuSize = mtuValue;
        }
    }
    return;
}

void CellularDataStateMachine::SplitProxyIpAddress(const std::string &proxyIpAddress, std::string &host, uint16_t &port)
{
    std::vector<std::string> address;
    size_t pos = 0;
    size_t found = 0;
    while ((found = proxyIpAddress.find(':', pos)) != std::string::npos) {
        address.push_back(proxyIpAddress.substr(pos, found - pos));
        pos = found + 1;
    }
    address.push_back(proxyIpAddress.substr(pos));
    if (address.size() == HOST_SIZE) {
        host = address[0];
    }
    if (address.size() == HOST_PORT_SIZE) {
        host = address[0];
        port = static_cast<uint16_t>(std::stoi(address[1]));
    }
}

void CellularDataStateMachine::UpdateHttpProxy(const std::string &proxyIpAddress)
{
    std::string host = "";
    uint16_t port = DEFAULT_PORT;
    SplitProxyIpAddress(proxyIpAddress, host, port);
    HttpProxy httpProxy = { host, port, {} };
    netLinkInfo_->httpProxy_ = httpProxy;
}

void CellularDataStateMachine::UpdateNetworkInfo(const SetupDataCallResultInfo &dataCallInfo)
{
    std::lock_guard<std::mutex> guard(mtx_);
    int32_t slotId = GetSlotId();
    TELEPHONY_LOGD("Slot%{private}d: dataCall, capability:%{private}" PRIu64", state:%{private}d, addr:%{private}s, "
        "dns: %{private}s, gw: %{private}s", slotId, capability_, dataCallInfo.reason,
        dataCallInfo.address.c_str(), dataCallInfo.dns.c_str(), dataCallInfo.gateway.c_str());
    std::vector<AddressInfo> ipInfoArray = CellularDataUtils::ParseIpAddr(dataCallInfo.address);
    std::vector<AddressInfo> dnsInfoArray = CellularDataUtils::ParseNormalIpAddr(dataCallInfo.dns);
    std::vector<AddressInfo> dnsSecArray = CellularDataUtils::ParseNormalIpAddr(dataCallInfo.dnsSec);
    dnsInfoArray.insert(dnsInfoArray.end(), dnsSecArray.begin(), dnsSecArray.end());
    std::vector<AddressInfo> routeInfoArray = CellularDataUtils::ParseNormalIpAddr(dataCallInfo.gateway);
    if (ipInfoArray.empty() || dnsInfoArray.empty() || routeInfoArray.empty()) {
        TELEPHONY_LOGE("Verifying network Information(ipInfoArray or dnsInfoArray or routeInfoArray empty)");
    }
    if (netLinkInfo_ == nullptr || netSupplierInfo_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: start update net info,but netLinkInfo or netSupplierInfo is null!", slotId);
        return;
    }
    bool roamingState = false;
    if (CoreManagerInner::GetInstance().GetPsRoamingState(slotId) > 0) {
        roamingState = true;
    }
    int32_t mtuSize = (dataCallInfo.maxTransferUnit == 0) ? DEFAULT_MTU : dataCallInfo.maxTransferUnit;
    ipType_ = GetIpType(ipInfoArray);
    GetMtuSizeFromOpCfg(mtuSize, slotId);
    netLinkInfo_->ifaceName_ = dataCallInfo.netPortName;
    netLinkInfo_->mtu_ = mtuSize;
    netLinkInfo_->tcpBufferSizes_ = tcpBuffer_;
    ResolveIp(ipInfoArray);
    ResolveDns(dnsInfoArray);
    ResolveRoute(routeInfoArray, dataCallInfo.netPortName);
    netSupplierInfo_->isAvailable_ = (dataCallInfo.active > 0);
    netSupplierInfo_->isRoaming_ = roamingState;
    netSupplierInfo_->linkUpBandwidthKbps_ = upBandwidth_;
    netSupplierInfo_->linkDownBandwidthKbps_ = downBandwidth_;
    cause_ = dataCallInfo.reason;
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    int32_t supplierId = netAgent.GetSupplierId(slotId, capability_);
    netAgent.UpdateNetSupplierInfo(supplierId, netSupplierInfo_);
    if (netSupplierInfo_->isAvailable_) {
        netAgent.UpdateNetLinkInfo(supplierId, netLinkInfo_);
    }
}

void CellularDataStateMachine::UpdateNetworkInfo()
{
    int32_t slotId = GetSlotId();
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    netLinkInfo_->tcpBufferSizes_ = tcpBuffer_;
    netSupplierInfo_->linkUpBandwidthKbps_ = upBandwidth_;
    netSupplierInfo_->linkDownBandwidthKbps_ = downBandwidth_;
    int32_t supplierId = netAgent.GetSupplierId(slotId, capability_);
    netAgent.UpdateNetSupplierInfo(supplierId, netSupplierInfo_);
    if (netSupplierInfo_->isAvailable_) {
        netAgent.UpdateNetLinkInfo(supplierId, netLinkInfo_);
    }
}

void CellularDataStateMachine::ResolveIp(std::vector<AddressInfo> &ipInfoArray)
{
    TELEPHONY_LOGD("Resolve Ip ifaceName_: %{private}s, domain_: %{private}s, mtu_: %{private}d, isAvailable_:"
        " %{private}d, isRoaming_:%{private}d", netLinkInfo_->ifaceName_.c_str(),
        netLinkInfo_->domain_.c_str(), netLinkInfo_->mtu_,
        netSupplierInfo_->isAvailable_, netSupplierInfo_->isRoaming_);
    netLinkInfo_->netAddrList_.clear();
    for (AddressInfo ipInfo : ipInfoArray) {
        INetAddr netAddr;
        netAddr.address_ = ipInfo.ip;
        netAddr.family_ = ipInfo.type;
        netAddr.type_ = ipInfo.type;
        netAddr.hostName_ = DEFAULT_HOSTNAME;
        netAddr.netMask_ = ipInfo.netMask.length() > 0 ? ipInfo.netMask : DEFAULT_MASK;
        netAddr.prefixlen_ = ipInfo.prefixLen;
        netLinkInfo_->netAddrList_.push_back(netAddr);
    }
}

void CellularDataStateMachine::ResolveDns(std::vector<AddressInfo> &dnsInfoArray)
{
    netLinkInfo_->dnsList_.clear();
    for (AddressInfo dnsInfo : dnsInfoArray) {
        INetAddr dnsAddr;
        dnsAddr.address_ = dnsInfo.ip;
        dnsAddr.family_ = dnsInfo.type;
        dnsAddr.type_ = dnsInfo.type;
        dnsAddr.hostName_ = DEFAULT_HOSTNAME;
        dnsAddr.netMask_ = dnsInfo.netMask;
        dnsAddr.prefixlen_ = dnsInfo.prefixLen;
        netLinkInfo_->dnsList_.push_back(dnsAddr);
    }
}

void CellularDataStateMachine::ResolveRoute(std::vector<AddressInfo> &routeInfoArray, const std::string &name)
{
    netLinkInfo_->routeList_.clear();
    for (AddressInfo routeInfo : routeInfoArray) {
        NetManagerStandard::Route route;
        route.iface_ = name;
        route.gateway_.address_ = routeInfo.ip;
        route.gateway_.family_ = routeInfo.type;
        route.gateway_.type_ = routeInfo.type;
        route.gateway_.hostName_ = DEFAULT_HOSTNAME;
        route.gateway_.netMask_ = DEFAULT_MASK;
        route.gateway_.prefixlen_ = routeInfo.prefixLen;
        if (routeInfo.type == INetAddr::IpType::IPV4) {
            route.destination_.address_ = ROUTED_IPV4;
        } else {
            route.destination_.address_ = ROUTED_IPV6;
        }
        route.destination_.family_ = routeInfo.type;
        route.destination_.type_ = routeInfo.type;
        route.destination_.hostName_ = DEFAULT_HOSTNAME;
        route.destination_.netMask_ = DEFAULT_MASK;
        route.destination_.prefixlen_ = 0;
        netLinkInfo_->routeList_.push_back(route);
    }
}

void CellularDataStateMachine::SetConnectionBandwidth(const uint32_t upBandwidth, const uint32_t downBandwidth)
{
    upBandwidth_ = upBandwidth;
    downBandwidth_ = downBandwidth;
}

void CellularDataStateMachine::SetConnectionTcpBuffer(const std::string &tcpBuffer)
{
    tcpBuffer_ = tcpBuffer;
}
} // namespace Telephony
} // namespace OHOS
