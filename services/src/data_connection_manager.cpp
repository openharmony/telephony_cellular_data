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

#include "data_connection_manager.h"

#include "core_manager_inner.h"
#include "hril_data_parcel.h"
#include "radio_event.h"
#include "telephony_log_wrapper.h"

#include "cellular_data_event_code.h"
#include "cellular_data_handler.h"
#include "cellular_data_state_machine.h"
#include "cellular_data_utils.h"

namespace OHOS {
namespace Telephony {
DataConnectionManager::DataConnectionManager(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : StateMachine(runner), slotId_(slotId)
{
    connectionMonitor_ = std::make_shared<DataConnectionMonitor>(runner, slotId);
    ccmDefaultState_ = std::make_unique<CcmDefaultState>(*this, "CcmDefaultState").release();
    if (connectionMonitor_ == nullptr || ccmDefaultState_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionMonitor_ or ccmDefaultState is null", slotId_);
        return;
    }
    StateMachine::SetOriginalState(ccmDefaultState_);
    StateMachine::Start();
}

DataConnectionManager::~DataConnectionManager()
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->RemoveAllEvents();
    }
}

void DataConnectionManager::AddConnectionStateMachine(const std::shared_ptr<CellularDataStateMachine> &stateMachine)
{
    if (stateMachine != nullptr) {
        stateMachines_.push_back(stateMachine);
    }
}

void DataConnectionManager::RemoveConnectionStateMachine(const std::shared_ptr<CellularDataStateMachine> &stateMachine)
{
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: stateMachine is null", slotId_);
        return;
    }
    for (std::vector<std::shared_ptr<CellularDataStateMachine>>::const_iterator iter =
        stateMachines_.begin(); iter != stateMachines_.end(); iter++) {
        if (*iter.base() == stateMachine) {
            stateMachines_.erase(iter);
            break;
        }
    }
}

std::vector<std::shared_ptr<CellularDataStateMachine>> DataConnectionManager::GetAllConnectionMachine()
{
    return stateMachines_;
}

void DataConnectionManager::AddActiveConnectionByCid(const std::shared_ptr<CellularDataStateMachine> &stateMachine)
{
    if (stateMachine != nullptr) {
        cidActiveConnectionMap_[stateMachine->GetCid()] = stateMachine;
    }
}

std::shared_ptr<CellularDataStateMachine> DataConnectionManager::GetActiveConnectionByCid(int32_t cid) const
{
    std::map<int32_t, std::shared_ptr<CellularDataStateMachine>>::const_iterator it = cidActiveConnectionMap_.find(cid);
    if (it != cidActiveConnectionMap_.end()) {
        return it->second;
    }
    return nullptr;
}

std::map<int32_t, std::shared_ptr<CellularDataStateMachine>> DataConnectionManager::GetActiveConnection() const
{
    return cidActiveConnectionMap_;
}

bool DataConnectionManager::isNoActiveConnection() const
{
    if (cidActiveConnectionMap_.empty()) {
        return true;
    }
    return false;
}

void DataConnectionManager::RemoveActiveConnectionByCid(int32_t cid)
{
    if (cidActiveConnectionMap_.find(cid) != cidActiveConnectionMap_.end()) {
        cidActiveConnectionMap_.erase(cid);
    }
}

void DataConnectionManager::StartStallDetectionTimer()
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->StartStallDetectionTimer();
    }
}

void DataConnectionManager::StopStallDetectionTimer()
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->StopStallDetectionTimer();
    }
}

void DataConnectionManager::RegisterRadioObserver()
{
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.RegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_CONNECTED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, stateMachineEventHandler_,
        RadioEvent::RADIO_DATA_CALL_LIST_CHANGED, nullptr);
}

void DataConnectionManager::UnRegisterRadioObserver() const
{
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.UnRegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_CONNECTED);
    coreInner.UnRegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_DATA_CALL_LIST_CHANGED);
}

void CcmDefaultState::StateBegin()
{
    connectManager_.RegisterRadioObserver();
}

void CcmDefaultState::StateEnd()
{
    connectManager_.UnRegisterRadioObserver();
}

bool CcmDefaultState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    int32_t id = event->GetInnerEventId();
    switch (id) {
        case RadioEvent::RADIO_CONNECTED:
            TELEPHONY_LOGI("Radio is connected");
            break;
        case RadioEvent::RADIO_DATA_CALL_LIST_CHANGED:
            RadioDataCallListChanged(event);
            break;
        default:
            TELEPHONY_LOGE("StateProcess handle nothing!");
            return false;
    }
    return true;
}

void CcmDefaultState::RadioDataCallListChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<DataCallResultList> infos = event->GetSharedObject<DataCallResultList>();
    if (infos == nullptr) {
        TELEPHONY_LOGE("setupDataCallResultInfo is null");
        return;
    }
    UpdateNetworkInfo(event);
    std::vector<std::shared_ptr<CellularDataStateMachine>> retryDataConnection;
    std::map<int32_t, std::shared_ptr<CellularDataStateMachine>> idActiveConnectionMap =
        connectManager_.GetActiveConnection();
    for (const std::pair<const int32_t, std::shared_ptr<CellularDataStateMachine>>& it : idActiveConnectionMap) {
        bool isPush = true;
        if (it.second == nullptr) {
            TELEPHONY_LOGI("The activation item is null(%{public}d)", it.first);
            continue;
        }
        int32_t cid = it.second->GetCid();
        for (size_t i = 0; i < infos->dcList.size(); ++i) {
            if (infos->dcList[i].cid == cid) {
                isPush = false;
                break;
            }
        }
        if (isPush) {
            TELEPHONY_LOGI("cid:%{public}d add to retry", it.first);
            retryDataConnection.push_back(it.second);
        }
    }
    for (std::shared_ptr<CellularDataStateMachine> &it : retryDataConnection) {
        if (it == nullptr) {
            TELEPHONY_LOGI(" retryDataConnection is null");
            continue;
        }
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_LOST_CONNECTION);
        it->SendEvent(event);
    }
}

void CcmDefaultState::UpdateNetworkInfo(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<DataCallResultList> infos = event->GetSharedObject<DataCallResultList>();
    if (infos == nullptr) {
        TELEPHONY_LOGE("dataCallResultList is null");
        return;
    }
    for (SetupDataCallResultInfo &it : infos->dcList) {
        std::shared_ptr<CellularDataStateMachine> dataConnect = connectManager_.GetActiveConnectionByCid(it.cid);
        if (dataConnect == nullptr) {
            TELEPHONY_LOGE("get active connection by cid is :=  %{public}d flag:=  %{public}d ", it.cid, it.flag);
            continue;
        }
        dataConnect->UpdateNetworkInfo(it);
    }
}

void DataConnectionManager::BeginNetStatistics()
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->BeginNetStatistics();
    }
}

void DataConnectionManager::EndNetStatistics()
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->EndNetStatistics();
    }
}

int32_t DataConnectionManager::GetSlotId() const
{
    return slotId_;
}

int32_t DataConnectionManager::GetDataFlowType()
{
    if (connectionMonitor_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connection monitor is null", slotId_);
        return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    }
    CellDataFlowType flowType = connectionMonitor_->GetDataFlowType();
    return static_cast<int32_t>(flowType);
}

void DataConnectionManager::SetDataFlowType(CellDataFlowType dataFlowType)
{
    if (connectionMonitor_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connection monitor is null", slotId_);
        return;
    }
    connectionMonitor_->SetDataFlowType(dataFlowType);
}

std::string DataConnectionManager::GetDefaultBandWidthsConfig()
{
    bandwidthConfigMap_.clear();
    char bandWidthBuffer[MAX_BUFFER_SIZE] = {0};
    GetParameter(CONFIG_BANDWIDTH, DEFAULT_BANDWIDTH_CONFIG, bandWidthBuffer, MAX_BUFFER_SIZE);
    std::vector<std::string> linkBandwidthVec = CellularDataUtils::Split(bandWidthBuffer, ";");
    for (std::string temp : linkBandwidthVec) {
        std::vector<std::string> linkBandwidths = CellularDataUtils::Split(temp, ":");
        if (linkBandwidths.size() == VALID_VECTOR_SIZE) {
            std::string key = linkBandwidths.front();
            std::string linkUpDownBandwidth = linkBandwidths.back();
            std::vector<std::string> upDownBandwidthValue = CellularDataUtils::Split(linkUpDownBandwidth, ",");
            if (upDownBandwidthValue.size() == VALID_VECTOR_SIZE) {
                LinkBandwidthInfo linkBandwidthInfo;
                linkBandwidthInfo.upBandwidth = (atoi)(upDownBandwidthValue.front().c_str());
                linkBandwidthInfo.downBandwidth = (atoi)(upDownBandwidthValue.back().c_str());
                bandwidthConfigMap_.emplace(key, linkBandwidthInfo);
            }
        }
    }
    TELEPHONY_LOGI("Slot%{public}d: BANDWIDTH_CONFIG_MAP size is %{public}zu", slotId_, bandwidthConfigMap_.size());
    return "bandWidth";
}

std::string DataConnectionManager::GetDefaultTcpBufferConfig()
{
    tcpBufferConfigMap_.clear();
    char tcpBufferConfig[MAX_BUFFER_SIZE] = {0};
    GetParameter(CONFIG_TCP_BUFFER, DEFAULT_TCP_BUFFER_CONFIG, tcpBufferConfig, MAX_BUFFER_SIZE);
    std::vector<std::string> tcpBufferVec = CellularDataUtils::Split(tcpBufferConfig, ";");
    for (std::string tcpBuffer : tcpBufferVec) {
        std::vector<std::string> str = CellularDataUtils::Split(tcpBuffer, ":");
        tcpBufferConfigMap_.emplace(str.front(), str.back());
    }
    TELEPHONY_LOGI("Slot%{public}d: TCP_BUFFER_CONFIG_MAP size is %{public}zu", slotId_, tcpBufferConfigMap_.size());
    return tcpBufferConfig;
}

LinkBandwidthInfo DataConnectionManager::GetBandwidthsByRadioTech(const int32_t radioTech)
{
    LinkBandwidthInfo linkBandwidthInfo;
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    NrState nrState = coreInner.GetNrState(slotId_);
    FrequencyType frequencyType = coreInner.GetFrequencyType(slotId_);
    std::string radioTechName = CellularDataUtils::ConvertRadioTechToRadioName(radioTech);
    if (radioTech == (int32_t)RadioTech::RADIO_TECHNOLOGY_LTE &&
        (nrState == NrState::NR_NSA_STATE_DUAL_CONNECTED || nrState == NrState::NR_NSA_STATE_CONNECTED_DETECT)) {
        if (frequencyType == FrequencyType::FREQ_TYPE_MMWAVE) {
            radioTechName = "NR_NSA_MMWAVE";
        } else {
            radioTechName = "NR_NSA";
        }
    }
    if (radioTechName == "NR") {
        radioTechName = "NR_SA";
    }
    TELEPHONY_LOGI("Slot%{public}d: accessRadioName is %{public}s", slotId_, radioTechName.c_str());
    std::map<std::string, LinkBandwidthInfo>::iterator iter = bandwidthConfigMap_.find(radioTechName);
    if (iter != bandwidthConfigMap_.end()) {
        linkBandwidthInfo = iter->second;
        TELEPHONY_LOGI("Slot%{public}d: name is %{public}s upBandwidth = %{public}u downBandwidth = %{public}u",
            slotId_, iter->first.c_str(), linkBandwidthInfo.upBandwidth, linkBandwidthInfo.downBandwidth);
    }
    return linkBandwidthInfo;
}

std::string DataConnectionManager::GetTcpBufferByRadioTech(const int32_t radioTech)
{
    std::string tcpBuffer = "";
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    NrState nrState = coreInner.GetNrState(slotId_);
    std::string radioTechName = CellularDataUtils::ConvertRadioTechToRadioName(radioTech);
    if ((radioTech == (int32_t)RadioTech::RADIO_TECHNOLOGY_LTE ||
        radioTech == (int32_t)RadioTech::RADIO_TECHNOLOGY_LTE_CA) &&
        (nrState == NrState::NR_NSA_STATE_DUAL_CONNECTED || nrState == NrState::NR_NSA_STATE_CONNECTED_DETECT)) {
        radioTechName = "NR";
    }
    std::map<std::string, std::string>::iterator iter = tcpBufferConfigMap_.find(radioTechName);
    if (iter != tcpBufferConfigMap_.end()) {
        tcpBuffer = iter->second;
    }
    return tcpBuffer;
}
} // namespace Telephony
} // namespace OHOS
