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

#include "cellular_data_event_code.h"
#include "cellular_data_handler.h"
#include "cellular_data_state_machine.h"
#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "tel_ril_data_parcel.h"
#include "operator_config_types.h"
#include "radio_event.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
DataConnectionManager::DataConnectionManager(int32_t slotId) : StateMachine("DataConnectionManager"), slotId_(slotId)
{
    connectionMonitor_ = std::make_shared<DataConnectionMonitor>(slotId);
    if (connectionMonitor_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionMonitor_ is null", slotId_);
        return;
    }
}

DataConnectionManager::~DataConnectionManager()
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->RemoveAllEvents();
    }
}

void DataConnectionManager::Init()
{
    ccmDefaultState_ = std::make_unique<CcmDefaultState>(*this, "CcmDefaultState").release();
    if (ccmDefaultState_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ccmDefaultState_ is null", slotId_);
        return;
    }
    StateMachine::SetOriginalState(ccmDefaultState_);
    StateMachine::Start();
}

void DataConnectionManager::AddConnectionStateMachine(const std::shared_ptr<CellularDataStateMachine> &stateMachine)
{
    std::lock_guard<std::mutex> lock(stateMachineMutex_);
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
    std::lock_guard<std::mutex> lock(stateMachineMutex_);
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
    std::lock_guard<std::mutex> lock(stateMachineMutex_);
    return stateMachines_;
}

void DataConnectionManager::AddActiveConnectionByCid(const std::shared_ptr<CellularDataStateMachine> &stateMachine)
{
    std::lock_guard<std::mutex> lock(activeConnectionMutex_);
    if (stateMachine != nullptr) {
        cidActiveConnectionMap_[stateMachine->GetCid()] = stateMachine;
    }
}

std::shared_ptr<CellularDataStateMachine> DataConnectionManager::GetActiveConnectionByCid(int32_t cid)
{
    std::lock_guard<std::mutex> lock(activeConnectionMutex_);
    std::map<int32_t, std::shared_ptr<CellularDataStateMachine>>::const_iterator it = cidActiveConnectionMap_.find(cid);
    if (it != cidActiveConnectionMap_.end()) {
        return it->second;
    }
    return nullptr;
}

std::map<int32_t, std::shared_ptr<CellularDataStateMachine>> DataConnectionManager::GetActiveConnection()
{
    std::lock_guard<std::mutex> lock(activeConnectionMutex_);
    return cidActiveConnectionMap_;
}

bool DataConnectionManager::IsBandwidthSourceModem() const
{
    return bandwidthSourceModem_;
}

bool DataConnectionManager::isNoActiveConnection()
{
    std::lock_guard<std::mutex> lock(activeConnectionMutex_);
    if (cidActiveConnectionMap_.empty()) {
        return true;
    }
    return false;
}

void DataConnectionManager::RemoveActiveConnectionByCid(int32_t cid)
{
    std::lock_guard<std::mutex> lock(activeConnectionMutex_);
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
    if (stateMachineEventHandler_ == nullptr) {
        TELEPHONY_LOGE("stateMachineEventHandler_ is nullptr!");
        return;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.RegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_CONNECTED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_DATA_CALL_LIST_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(
        slotId_, stateMachineEventHandler_, RadioEvent::RADIO_LINK_CAPABILITY_CHANGED, nullptr);
}

void DataConnectionManager::UnRegisterRadioObserver() const
{
    if (stateMachineEventHandler_ == nullptr) {
        TELEPHONY_LOGE("stateMachineEventHandler_ is nullptr!");
        return;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.UnRegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_CONNECTED);
    coreInner.UnRegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_DATA_CALL_LIST_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, stateMachineEventHandler_, RadioEvent::RADIO_LINK_CAPABILITY_CHANGED);
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
        case RadioEvent::RADIO_LINK_CAPABILITY_CHANGED:
            RadioLinkCapabilityChanged(event);
            break;
        default:
            TELEPHONY_LOGE("handle nothing!");
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
            if (infos->dcList[i].cid == cid && infos->dcList[i].active > 0) {
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

void CcmDefaultState::RadioLinkCapabilityChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<DataLinkCapability> linkCapability = event->GetSharedObject<DataLinkCapability>();
    if (linkCapability == nullptr) {
        TELEPHONY_LOGE("linkCapability is null");
        return;
    }
    if (connectManager_.IsBandwidthSourceModem()) {
        std::map<int32_t, std::shared_ptr<CellularDataStateMachine>> idActiveConnectionMap =
            connectManager_.GetActiveConnection();
        for (const std::pair<const int32_t, std::shared_ptr<CellularDataStateMachine>> &it : idActiveConnectionMap) {
            if (it.second == nullptr) {
                TELEPHONY_LOGI("The activation item is null(%{public}d)", it.first);
                continue;
            }
            AppExecFwk::InnerEvent::Pointer smEvent =
                AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_LINK_CAPABILITY_CHANGED, linkCapability);
            it.second->SendEvent(smEvent);
        }
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

void DataConnectionManager::UpdateCallState(int32_t state)
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->UpdateCallState(state);
    }
}

int32_t DataConnectionManager::GetDataRecoveryState()
{
    if (connectionMonitor_ != nullptr) {
        return static_cast<int32_t>(connectionMonitor_->GetDataRecoveryState());
    }
    return -1;
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

void DataConnectionManager::GetDefaultBandWidthsConfig()
{
    OperatorConfig operatorConfig;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, operatorConfig);
    if (operatorConfig.boolValue.find(KEY_BANDWIDTH_SOURCE_USE_MODEM_BOOL) != operatorConfig.boolValue.end()) {
        bandwidthSourceModem_ = operatorConfig.boolValue[KEY_BANDWIDTH_SOURCE_USE_MODEM_BOOL];
    }
    if (operatorConfig.boolValue.find(KEY_UPLINK_BANDWIDTH_NR_NSA_USE_LTE_VALUE_BOOL) !=
        operatorConfig.boolValue.end()) {
        uplinkUseLte_ = operatorConfig.boolValue[KEY_UPLINK_BANDWIDTH_NR_NSA_USE_LTE_VALUE_BOOL];
    }
    std::vector<std::string> linkBandwidthVec;
    if (operatorConfig.stringArrayValue.find(KEY_BANDWIDTH_STRING_ARRAY) != operatorConfig.stringArrayValue.end()) {
        linkBandwidthVec = operatorConfig.stringArrayValue[KEY_BANDWIDTH_STRING_ARRAY];
    }
    if (linkBandwidthVec.empty()) {
        linkBandwidthVec = CellularDataUtils::Split(DEFAULT_BANDWIDTH_CONFIG, ";");
    }
    bandwidthConfigMap_.clear();
    for (std::string temp : linkBandwidthVec) {
        std::vector<std::string> linkBandwidths = CellularDataUtils::Split(temp, ":");
        if (linkBandwidths.size() == VALID_VECTOR_SIZE) {
            std::string key = linkBandwidths.front();
            std::string linkUpDownBandwidth = linkBandwidths.back();
            std::vector<std::string> upDownBandwidthValue = CellularDataUtils::Split(linkUpDownBandwidth, ",");
            if (upDownBandwidthValue.size() == VALID_VECTOR_SIZE) {
                LinkBandwidthInfo linkBandwidthInfo;
                linkBandwidthInfo.downBandwidth = (atoi)(upDownBandwidthValue.front().c_str());
                linkBandwidthInfo.upBandwidth = (atoi)(upDownBandwidthValue.back().c_str());
                bandwidthConfigMap_.emplace(key, linkBandwidthInfo);
            }
        }
    }
    TELEPHONY_LOGI("Slot%{public}d: BANDWIDTH_CONFIG_MAP size is %{public}zu", slotId_, bandwidthConfigMap_.size());
    UpdateBandWidthsUseLte();
}

void DataConnectionManager::UpdateBandWidthsUseLte()
{
    if (!uplinkUseLte_) {
        return;
    }
    std::map<std::string, LinkBandwidthInfo>::iterator iter = bandwidthConfigMap_.find("LTE");
    if (iter != bandwidthConfigMap_.end()) {
        LinkBandwidthInfo lteLinkBandwidthInfo = iter->second;
        TELEPHONY_LOGI("Slot%{public}d: name is %{public}s upBandwidth = %{public}u downBandwidth = %{public}u",
            slotId_, iter->first.c_str(), lteLinkBandwidthInfo.upBandwidth, lteLinkBandwidthInfo.downBandwidth);
        iter = bandwidthConfigMap_.find("NR_NSA");
        if (iter != bandwidthConfigMap_.end()) {
            iter->second.upBandwidth = lteLinkBandwidthInfo.upBandwidth;
        }
        iter = bandwidthConfigMap_.find("NR_NSA_MMWAVE");
        if (iter != bandwidthConfigMap_.end()) {
            iter->second.upBandwidth = lteLinkBandwidthInfo.upBandwidth;
        }
    }
}

void DataConnectionManager::GetDefaultTcpBufferConfig()
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
    TELEPHONY_LOGI("Slot%{public}d: accessRadioName is %{private}s", slotId_, radioTechName.c_str());
    std::map<std::string, LinkBandwidthInfo>::iterator iter = bandwidthConfigMap_.find(radioTechName);
    if (iter != bandwidthConfigMap_.end()) {
        linkBandwidthInfo = iter->second;
        TELEPHONY_LOGI("Slot%{public}d: name is %{private}s upBandwidth = %{public}u downBandwidth = %{public}u",
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

void DataConnectionManager::IsNeedDoRecovery(bool needDoRecovery) const
{
    if (connectionMonitor_ != nullptr) {
        connectionMonitor_->IsNeedDoRecovery(needDoRecovery);
    }
}
} // namespace Telephony
} // namespace OHOS
