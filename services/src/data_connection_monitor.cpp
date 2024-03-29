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

#include "data_connection_monitor.h"

#include "core_manager_inner.h"
#include "radio_event.h"
#include "telephony_log_wrapper.h"

#include "cellular_data_event_code.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_service.h"
#include "cellular_data_types.h"
#include "cellular_data_constant.h"
#include "data_service_ext_wrapper.h"
 
#ifdef ABILITY_POWER_SUPPORT
#include "power_mgr_client.h"
#endif

namespace OHOS {
namespace Telephony {
DataConnectionMonitor::DataConnectionMonitor(int32_t slotId) : TelEventHandler("DataConnectionMonitor"), slotId_(slotId)
{
    trafficManager_ = std::make_unique<TrafficManagement>(slotId);
    stallDetectionTrafficManager_ = std::make_unique<TrafficManagement>(slotId);
    if (trafficManager_ == nullptr || stallDetectionTrafficManager_ == nullptr) {
        TELEPHONY_LOGE("TrafficManager or stallDetectionTrafficManager init failed");
    }
}

bool DataConnectionMonitor::IsAggressiveRecovery()
{
    return (dataRecoveryState_ == RecoveryState::STATE_CLEANUP_CONNECTIONS) ||
        (dataRecoveryState_ == RecoveryState::STATE_REREGISTER_NETWORK) ||
        (dataRecoveryState_ == RecoveryState::STATE_RADIO_STATUS_RESTART);
}
 
bool DataConnectionMonitor::IsScreenOn()
{
    bool isScreenOn = false;
#ifdef ABILITY_POWER_SUPPORT
    isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
#endif
    TELEPHONY_LOGI("isScreenOn = %{public}d.", isScreenOn);
    return isScreenOn;
}
 
int32_t DataConnectionMonitor::GetStallDetectionPeriod()
{
    if (IsScreenOn() || IsAggressiveRecovery()) {
        return DATA_STALL_ALARM_AGGRESSIVE_DELAY_IN_MS_DEFAULT;
    }
    return DATA_STALL_ALARM_NON_AGGRESSIVE_DELAY_IN_MS_DEFAULT;
}

void DataConnectionMonitor::StartStallDetectionTimer()
{
    TELEPHONY_LOGI("Slot%{public}d: start stall detection", slotId_);
    stallDetectionEnabled = true;
    int32_t stallDetectionPeriod = GetStallDetectionPeriod();
    TELEPHONY_LOGI("stallDetectionPeriod = %{public}d", stallDetectionPeriod);
    if (!HasInnerEvent(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID)) {
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID);
        SendEvent(event, stallDetectionPeriod, Priority::LOW);
    }
}

__attribute__((no_sanitize("cfi"))) void DataConnectionMonitor::OnStallDetectionTimer()
{
    TELEPHONY_LOGD("Slot%{public}d: on stall detection", slotId_);
#ifdef OHOS_BUILD_ENABLE_DATA_SERVICE_EXT
    if (DATA_SERVICE_EXT_WRAPPER.requestTcpAndDnsPackets_) {
        DATA_SERVICE_EXT_WRAPPER.requestTcpAndDnsPackets_();
        return;
    }
#endif
    UpdateFlowInfo();
    if (noRecvPackets_ > RECOVERY_TRIGGER_PACKET) {
        HandleRecovery();
        noRecvPackets_ = 0;
    }
    int32_t stallDetectionPeriod = GetStallDetectionPeriod();
    TELEPHONY_LOGI("stallDetectionPeriod = %{public}d", stallDetectionPeriod);
    if (!HasInnerEvent(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID) && stallDetectionEnabled) {
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID);
        SendEvent(event, stallDetectionPeriod, Priority::LOW);
    }
}

void DataConnectionMonitor::StopStallDetectionTimer()
{
    TELEPHONY_LOGD("Slot%{public}d: stop stall detection", slotId_);
    stallDetectionEnabled = false;
    RemoveEvent(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID);
}

void DataConnectionMonitor::UpdateFlowInfo()
{
    if (stallDetectionTrafficManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: stallDetectionTrafficManager_ is null", slotId_);
        return;
    }
    int64_t previousSentPackets = 0;
    int64_t previousRecvPackets = 0;
    int64_t currentSentPackets = 0;
    int64_t currentRecvPackets = 0;
    stallDetectionTrafficManager_->GetPacketData(previousSentPackets, previousRecvPackets);
    stallDetectionTrafficManager_->UpdatePacketData();
    stallDetectionTrafficManager_->GetPacketData(currentSentPackets, currentRecvPackets);
    int64_t sentPackets = currentSentPackets - previousSentPackets;
    int64_t recvPackets = currentRecvPackets - previousRecvPackets;
    if (sentPackets > 0 && recvPackets == 0) {
        noRecvPackets_ += sentPackets;
    } else if ((sentPackets > 0 && recvPackets > 0) || (sentPackets == 0 && recvPackets > 0)) {
        noRecvPackets_ = 0;
        dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
    } else {
        TELEPHONY_LOGD("Slot%{public}d: Update Flow Info nothing to do", slotId_);
    }
}

void DataConnectionMonitor::HandleRecovery()
{
    if (callState_ != static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE) &&
        callState_ != static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED)) {
        TELEPHONY_LOGI("Slot%{public}d: Stop recovery while call is busy", slotId_);
        dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
        return;
    }
    switch (dataRecoveryState_) {
        case RecoveryState::STATE_REQUEST_CONTEXT_LIST: {
            TELEPHONY_LOGI("Slot%{public}d: Handle Recovery: get data call list", slotId_);
            dataRecoveryState_ = RecoveryState::STATE_CLEANUP_CONNECTIONS;
            GetPdpContextList();
            CellularDataHiSysEvent::WriteDataDeactiveBehaviorEvent(slotId_, DataDisconnectCause::ON_THE_NETWORK_SIDE);
            break;
        }
        case RecoveryState::STATE_CLEANUP_CONNECTIONS: {
            TELEPHONY_LOGI("Slot%{public}d: Handle Recovery: cleanup connections", slotId_);
            dataRecoveryState_ = RecoveryState::STATE_REREGISTER_NETWORK;
            int32_t ret = DelayedRefSingleton<CellularDataService>::GetInstance().ClearAllConnections(
                slotId_, DisConnectionReason::REASON_RETRY_CONNECTION);
            if (ret != static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)) {
                TELEPHONY_LOGE("Slot%{public}d: Handle Recovery: cleanup connections failed", slotId_);
            }
            break;
        }
        case RecoveryState::STATE_REREGISTER_NETWORK: {
            TELEPHONY_LOGI("Slot%{public}d: Handle Recovery: re-register network", slotId_);
            dataRecoveryState_ = RecoveryState::STATE_RADIO_STATUS_RESTART;
            GetPreferredNetworkPara();
            break;
        }
        case RecoveryState::STATE_RADIO_STATUS_RESTART: {
            TELEPHONY_LOGI("Slot%{public}d: Handle Recovery: radio restart", slotId_);
            dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
            int32_t ret = DelayedRefSingleton<CellularDataService>::GetInstance().ClearAllConnections(
                slotId_, DisConnectionReason::REASON_RETRY_CONNECTION);
            if (ret != static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)) {
                TELEPHONY_LOGE("Slot%{public}d: Handle Recovery: radio restart cleanup connections failed", slotId_);
            }
            SetRadioState(CORE_SERVICE_POWER_OFF, RadioEvent::RADIO_OFF);
            break;
        }
        default: {
            TELEPHONY_LOGE("Slot%{public}d: Handle Recovery is falsie", slotId_);
            break;
        }
    }
}

void DataConnectionMonitor::BeginNetStatistics()
{
    updateNetStat_ = true;
    UpdateNetTrafficState();
}

void DataConnectionMonitor::UpdateCallState(int32_t state)
{
    callState_ = state;
}

void DataConnectionMonitor::EndNetStatistics()
{
    RemoveEvent(CellularDataEventCode::MSG_RUN_MONITOR_TASK);
    updateNetStat_ = false;
    if (dataFlowType_ != CellDataFlowType::DATA_FLOW_TYPE_NONE) {
        dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_NONE;
        StateNotification::GetInstance().OnUpDataFlowtype(slotId_, dataFlowType_);
    }
}

void DataConnectionMonitor::UpdateNetTrafficState()
{
    if (!HasInnerEvent(CellularDataEventCode::MSG_RUN_MONITOR_TASK) && updateNetStat_) {
        UpdateDataFlowType();
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_RUN_MONITOR_TASK);
        SendEvent(event, DEFAULT_NET_STATISTICS_PERIOD);
    }
}

void DataConnectionMonitor::GetPdpContextList()
{
    CoreManagerInner::GetInstance().GetPdpContextList(slotId_,
        RadioEvent::RADIO_DATA_CALL_LIST_CHANGED, shared_from_this());
}

void DataConnectionMonitor::SetRadioState(const int32_t &radioState, const int32_t &eventCode)
{
    CoreManagerInner::GetInstance().SetRadioState(slotId_, eventCode, radioState, 0, shared_from_this());
}

void DataConnectionMonitor::GetPreferredNetworkPara()
{
    CoreManagerInner::GetInstance().GetPreferredNetworkPara(slotId_,
        RadioEvent::RADIO_GET_PREFERRED_NETWORK_MODE, shared_from_this());
}

void DataConnectionMonitor::SetPreferredNetworkPara(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<PreferredNetworkTypeInfo> preferredNetworkInfo = event->GetSharedObject<PreferredNetworkTypeInfo>();
    if (preferredNetworkInfo == nullptr) {
        TELEPHONY_LOGE("preferredNetworkInfo is null");
        return;
    }
    int32_t networkType = preferredNetworkInfo->preferredNetworkType;
    CoreManagerInner::GetInstance().SetPreferredNetworkPara(slotId_,
        RadioEvent::RADIO_SET_PREFERRED_NETWORK_MODE, networkType, shared_from_this());
}

RecoveryState DataConnectionMonitor::GetDataRecoveryState()
{
    return dataRecoveryState_;
}

void DataConnectionMonitor::UpdateDataFlowType()
{
    if (trafficManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: trafficManager is null", slotId_);
        return;
    }
    int64_t previousSentPackets = 0;
    int64_t previousRecvPackets = 0;
    int64_t currentSentPackets = 0;
    int64_t currentRecvPackets = 0;
    trafficManager_->GetPacketData(previousSentPackets, previousRecvPackets);
    trafficManager_->UpdatePacketData();
    trafficManager_->GetPacketData(currentSentPackets, currentRecvPackets);
    int64_t sentPackets = currentSentPackets - previousSentPackets;
    int64_t recvPackets = currentRecvPackets - previousRecvPackets;
    CellDataFlowType previousDataFlowType = dataFlowType_;
    if (previousSentPackets != 0 || previousRecvPackets != 0) {
        if (sentPackets > 0 && recvPackets == 0) {
            dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_UP;
        } else if (sentPackets == 0 && recvPackets > 0) {
            dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_DOWN;
        } else if (sentPackets > 0 && recvPackets > 0) {
            dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN;
        } else {
            dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_NONE;
        }
    }
    if (previousDataFlowType != dataFlowType_) {
        StateNotification::GetInstance().OnUpDataFlowtype(slotId_, dataFlowType_);
    }
}

CellDataFlowType DataConnectionMonitor::GetDataFlowType()
{
    return dataFlowType_;
}

void DataConnectionMonitor::SetDataFlowType(CellDataFlowType dataFlowType)
{
    if (dataFlowType_ != dataFlowType) {
        dataFlowType_ = dataFlowType;
        StateNotification::GetInstance().OnUpDataFlowtype(slotId_, dataFlowType_);
    }
}

void DataConnectionMonitor::IsNeedDoRecovery(bool needDoRecovery)
{
    if (needDoRecovery) {
        HandleRecovery();
    } else {
        dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
    }
    int32_t stallDetectionPeriod = GetStallDetectionPeriod();
    TELEPHONY_LOGI("stallDetectionPeriod = %{public}d", stallDetectionPeriod);
    if (!HasInnerEvent(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID) && stallDetectionEnabled) {
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID);
        SendEvent(event, stallDetectionPeriod, Priority::LOW);
    }
}

void DataConnectionMonitor::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    uint32_t eventID = event->GetInnerEventId();
    switch (eventID) {
        case CellularDataEventCode::MSG_RUN_MONITOR_TASK: {
            UpdateNetTrafficState();
            break;
        }
        case CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID:
            OnStallDetectionTimer();
            break;
        case RadioEvent::RADIO_DATA_CALL_LIST_CHANGED:
            TELEPHONY_LOGI("Slot%{public}d: radio call list changed complete", slotId_);
            break;
        case RadioEvent::RADIO_GET_PREFERRED_NETWORK_MODE:
            SetPreferredNetworkPara(event);
            break;
        case RadioEvent::RADIO_SET_PREFERRED_NETWORK_MODE:
            TELEPHONY_LOGI("Slot%{public}d: set preferred network mode complete", slotId_);
            break;
        case RadioEvent::RADIO_OFF:
            SetRadioState(CORE_SERVICE_POWER_ON, RadioEvent::RADIO_ON);
            break;
        case RadioEvent::RADIO_ON:
            TELEPHONY_LOGI("Slot%{public}d: set radio state on complete", slotId_);
            break;
        default:
            TELEPHONY_LOGI("Slot%{public}d: connection monitor ProcessEvent code = %{public}u", slotId_, eventID);
            break;
    }
}
} // namespace Telephony
} // namespace OHOS
