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

#include "active.h"

#include "cellular_data_hisysevent.h"
#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "inactive.h"
#include "telephony_log_wrapper.h"
#include "telephony_ext_wrapper.h"
#include "apn_manager.h"

namespace OHOS {
namespace Telephony {
void Active::StateBegin()
{
    TELEPHONY_LOGI("Enter active state");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    RefreshTcpBufferSizes();
    RefreshConnectionBandwidths();
    stateMachine->SetCurrentState(sptr<State>(this));
}

void Active::StateEnd()
{
    TELEPHONY_LOGI("Exit active state");
}

bool Active::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    bool retVal = false;
    uint32_t eventCode = event->GetInnerEventId();
    std::map<uint32_t, Fun>::iterator it = eventIdFunMap_.find(eventCode);
    if (it != eventIdFunMap_.end()) {
        if (it->second != nullptr) {
            return it->second(event);
        }
    }
    return retVal;
}

bool Active::ProcessConnectDone(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::MSG_SM_CONNECT");
    return PROCESSED;
}

bool Active::ProcessDisconnectDone(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::MSG_SM_DISCONNECT");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }

    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::unique_ptr<DataDisconnectParams> object = event->GetUniqueObject<DataDisconnectParams>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is null");
        return false;
    }
    if (stateMachine->GetReuseApnCap() != NetManagerStandard::NetCap::NET_CAPABILITY_END) {
        stateMachine->SetIfReuseSupplierId(false);
        stateMachine->SetReuseApnCap(NetManagerStandard::NetCap::NET_CAPABILITY_END);
    }
    DisConnectionReason reason = object->GetReason();
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    if (inActive == nullptr) {
        TELEPHONY_LOGE("inActive is null");
        return false;
    }
    inActive->SetPdpErrorReason(disconnReasonPdpErrorMap_[reason]);
    stateMachine->FreeConnection(*object);
    stateMachine->TransitionTo(stateMachine->disconnectingState_);
    return PROCESSED;
}

bool Active::ProcessDisconnectAllDone(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::MSG_SM_DISCONNECT_ALL");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::unique_ptr<DataDisconnectParams> object = event->GetUniqueObject<DataDisconnectParams>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is null");
        return false;
    }
    if (stateMachine->GetReuseApnCap() != NetManagerStandard::NetCap::NET_CAPABILITY_END) {
        stateMachine->SetIfReuseSupplierId(false);
        stateMachine->SetReuseApnCap(NetManagerStandard::NetCap::NET_CAPABILITY_END);
    }
    DisConnectionReason reason = object->GetReason();
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    if (inActive == nullptr) {
        TELEPHONY_LOGE("inActive is null");
        return false;
    }
    inActive->SetPdpErrorReason(disconnReasonPdpErrorMap_[reason]);
    stateMachine->FreeConnection(*object);
    stateMachine->TransitionTo(stateMachine->disconnectingState_);
    return PROCESSED;
}

bool Active::ProcessLostConnection(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::EVENT_LOST_CONNECTION");
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    auto resultInfo = event->GetSharedObject<SetupDataCallResultInfo>();
    if (resultInfo == nullptr) {
        TELEPHONY_LOGE("resultInfo null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    if (stateMachine->GetReuseApnCap() != NetManagerStandard::NetCap::NET_CAPABILITY_END) {
        stateMachine->SetIfReuseSupplierId(false);
        stateMachine->SetReuseApnCap(NetManagerStandard::NetCap::NET_CAPABILITY_END);
    }
    CellularDataHiSysEvent::WriteDataDeactiveBehaviorEvent(stateMachine->GetSlotId(),
        DataDisconnectCause::LOST_CONNECTION, ApnManager::FindApnNameByApnId(stateMachine->apnId_));
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    if (inActive == nullptr) {
        TELEPHONY_LOGE("inActive is null");
        return false;
    }
    inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
    inActive->SetDataCallResultInfo(resultInfo);

#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (TELEPHONY_EXT_WRAPPER.dataEndSelfCure_) {
        int32_t cause = static_cast<int32_t>(stateMachine->cause_);
        int32_t slotId = stateMachine->GetSlotId();
        TELEPHONY_LOGI("cause%{private}d:, slotId%{public}d", cause, slotId);
        TELEPHONY_EXT_WRAPPER.dataEndSelfCure_(cause, slotId);
    }
#endif
    stateMachine->TransitionTo(stateMachine->inActiveState_);
    return PROCESSED;
}

bool Active::ProcessRilAdapterHostDied(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::EVENT_RIL_ADAPTER_HOST_DIED");
    return ProcessLostConnection(event);
}

bool Active::ProcessLinkCapabilityChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::MSG_SM_LINK_CAPABILITY_CHANGED");
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::shared_ptr<DataLinkCapability> dataLinkCapability = event->GetSharedObject<DataLinkCapability>();
    if (dataLinkCapability == nullptr) {
        TELEPHONY_LOGE("result info is null");
        return false;
    }
    uint32_t upBandwidth = static_cast<uint32_t>(dataLinkCapability->primaryUplinkKbps);
    uint32_t downBandwidth = static_cast<uint32_t>(dataLinkCapability->primaryDownlinkKbps);
    std::shared_ptr<CellularDataStateMachine> shareStateMachine = stateMachine_.lock();
    if (shareStateMachine == nullptr) {
        TELEPHONY_LOGE("shareStateMachine is null");
        return false;
    }
    if (upBandwidth == 0 || downBandwidth == 0) {
        RefreshConnectionBandwidths();
    } else {
        shareStateMachine->SetConnectionBandwidth(upBandwidth, downBandwidth);
    }
    shareStateMachine->UpdateNetworkInfo();
    return true;
}

bool Active::ProcessDataConnectionRoamOn(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::EVENT_DATA_CONNECTION_ROAM_ON");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    int32_t supplierId = netAgent.GetSupplierId(stateMachine->GetSlotId(), stateMachine->GetCapability());
    netAgent.UpdateNetSupplierInfo(supplierId, stateMachine->netSupplierInfo_);
    return PROCESSED;
}

bool Active::ProcessDataConnectionRoamOff(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Active::EVENT_DATA_CONNECTION_ROAM_OFF");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    int32_t supplierId = netAgent.GetSupplierId(stateMachine->GetSlotId(), stateMachine->GetCapability());
    netAgent.UpdateNetSupplierInfo(supplierId, stateMachine->netSupplierInfo_);
    return PROCESSED;
}

bool Active::ProcessDataConnectionVoiceCallStartedOrEnded(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    int32_t supplierId = netAgent.GetSupplierId(stateMachine->GetSlotId(), stateMachine->GetCapability());
    netAgent.UpdateNetSupplierInfo(supplierId, stateMachine->netSupplierInfo_);
    return PROCESSED;
}

bool Active::ProcessDataConnectionComplete(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::shared_ptr<SetupDataCallResultInfo> resultInfo = event->GetSharedObject<SetupDataCallResultInfo>();
    if (resultInfo == nullptr) {
        TELEPHONY_LOGE("result info is null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> shareStateMachine = stateMachine_.lock();
    if (shareStateMachine == nullptr) {
        TELEPHONY_LOGE("shareStateMachine is null");
        return false;
    }
    resultInfo->flag = shareStateMachine->apnId_;

    if (shareStateMachine->stateMachineEventHandler_ == nullptr) {
        TELEPHONY_LOGE("stateMachineEventHandler_ is null");
        return false;
    }
    shareStateMachine->stateMachineEventHandler_->RemoveEvent(CellularDataEventCode::MSG_CONNECT_TIMEOUT_CHECK);
    if (shareStateMachine->cellularDataHandler_ != nullptr) {
        shareStateMachine->cellularDataHandler_->SendEvent(
            CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION_COMPLETE, resultInfo);
    } else {
        TELEPHONY_LOGE("cellularDataHandler is null");
        return false;
    }
    return true;
}

bool Active::ProcessNrStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<CellularDataStateMachine> shareStateMachine = stateMachine_.lock();
    if (shareStateMachine == nullptr) {
        TELEPHONY_LOGE("shareStateMachine is null");
        return false;
    }
    TELEPHONY_LOGI("ProcessNrStateChanged event");
    RefreshTcpBufferSizes();
    RefreshConnectionBandwidths();
    shareStateMachine->UpdateNetworkInfo();
    return true;
}

bool Active::ProcessNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<CellularDataStateMachine> shareStateMachine = stateMachine_.lock();
    if (shareStateMachine == nullptr) {
        TELEPHONY_LOGE("shareStateMachine is null");
        return false;
    }
    TELEPHONY_LOGI("ProcessNrFrequencyChanged event");
    RefreshConnectionBandwidths();
    shareStateMachine->UpdateNetworkInfo();
    return true;
}

void Active::RefreshTcpBufferSizes()
{
    std::shared_ptr<CellularDataStateMachine> shareStateMachine = stateMachine_.lock();
    if (shareStateMachine == nullptr) {
        TELEPHONY_LOGE("shareStateMachine is null");
        return;
    }
    int32_t slotId = shareStateMachine->GetSlotId();
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId, radioTech);
    if (shareStateMachine->cdConnectionManager_ == nullptr) {
        TELEPHONY_LOGE("cdConnectionManager_ is null");
        return;
    }
    std::string tcpBuffer = shareStateMachine->cdConnectionManager_->GetTcpBufferByRadioTech(radioTech);
    TELEPHONY_LOGI("tcpBuffer is %{public}s", tcpBuffer.c_str());
    shareStateMachine->SetConnectionTcpBuffer(tcpBuffer);
}

void Active::RefreshConnectionBandwidths()
{
    std::shared_ptr<CellularDataStateMachine> shareStateMachine = stateMachine_.lock();
    if (shareStateMachine == nullptr) {
        TELEPHONY_LOGE("shareStateMachine is null");
        return;
    }
    int32_t slotId = shareStateMachine->GetSlotId();
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId, radioTech);
    if (shareStateMachine->cdConnectionManager_ == nullptr) {
        TELEPHONY_LOGE("cdConnectionManager_ is null");
        return;
    }
    LinkBandwidthInfo linkBandwidthInfo = shareStateMachine->cdConnectionManager_->GetBandwidthsByRadioTech(radioTech);
    TELEPHONY_LOGI("upBandwidth is %{public}u, downBandwidth is %{public}u", linkBandwidthInfo.upBandwidth,
        linkBandwidthInfo.downBandwidth);
    shareStateMachine->SetConnectionBandwidth(linkBandwidthInfo.upBandwidth, linkBandwidthInfo.downBandwidth);
}
} // namespace Telephony
} // namespace OHOS
