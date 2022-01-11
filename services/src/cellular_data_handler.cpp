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

#include "cellular_data_handler.h"

#include "net_specifier.h"
#include "string_ex.h"

#include "cellular_data_types.h"
#include "hril_data_parcel.h"
#include "hril_call_parcel.h"
#include "i_sim_state_manager.h"
#include "network_state.h"
#include "str_convert.h"
#include "sim_state_type.h"
#include "telephony_log_wrapper.h"

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_utils.h"
#include "network_search_utils.h"
#include "sim_utils.h"

namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;
using namespace NetManagerStandard;

CellularDataHandler::CellularDataHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
    int32_t slotId) : EventHandler(runner), slotId_(slotId)
{}

void CellularDataHandler::Init()
{
    apnManager_ = std::make_unique<ApnManager>().release();
    dataSwitchSettings_ = std::make_unique<DataSwitchSettings>();
    connectionManager_ = std::make_unique<DataConnectionManager>(GetEventRunner(), slotId_).release();
    if ((apnManager_ == nullptr) || (dataSwitchSettings_ == nullptr) || (connectionManager_ == nullptr)) {
        TELEPHONY_LOGE("apnManager_ or dataSwitchSettings_ or connectionManager_ is null");
        return;
    }
    apnManager_->InitApnHolders();
    apnManager_->CreateAllApnItem();
    dataSwitchSettings_->LoadSwitchValue();
    GetConfigurationFor5G();
    SetRilLinkBandwidths();
}

CellularDataHandler::~CellularDataHandler() = default;

void CellularDataHandler::StartStallDetectionTimer()
{
    if (connectionManager_ != nullptr) {
        connectionManager_->StartStallDetectionTimer(shared_from_this());
    }
}

void CellularDataHandler::StopStallDetectionTimer()
{
    if (connectionManager_ != nullptr) {
        connectionManager_->StopStallDetectionTimer();
    }
}

bool CellularDataHandler::ReleaseNet(const NetRequest &request)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager is null.");
        return false;
    }
    int32_t id = ApnManager::FindApnIdByCapability(request.capability);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("ReleaseNet apnHolder is null.");
        return false;
    }
    std::unique_ptr<NetRequest> netRequest = std::make_unique<NetRequest>();
    netRequest->capability = request.capability;
    netRequest->ident = request.ident;
    AppExecFwk::InnerEvent::Pointer event =
        InnerEvent::Get(CellularDataEventCode::MSG_REQUEST_NETWORK, netRequest, TYPE_RELEASE_NET);
    return SendEvent(event);
}

bool CellularDataHandler::RequestNet(const NetRequest &request)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager is null.");
        return false;
    }
    int32_t id = ApnManager::FindApnIdByCapability(request.capability);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("RequestNet apnHolder is null.");
        return false;
    }
    ApnProfileState apnState = apnHolder->GetApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTED || apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_DISCONNECTING) {
        TELEPHONY_LOGE("RequestNet apn state is not ready.");
        return false;
    }
    std::unique_ptr<NetRequest> netRequest = std::make_unique<NetRequest>();
    netRequest->capability = request.capability;
    netRequest->ident = request.ident;
    AppExecFwk::InnerEvent::Pointer event =
        InnerEvent::Get(CellularDataEventCode::MSG_REQUEST_NETWORK, netRequest, TYPE_REQUEST_NET);
    return SendEvent(event);
}

bool CellularDataHandler::SetCellularDataEnable(bool userDataOn)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("SetCellularDataEnable dataSwitchSettings_ is null.");
        return false;
    }
    if (dataSwitchSettings_->GetUserDataOn() != userDataOn) {
        dataSwitchSettings_->SetUserDataOn(userDataOn);
        if (userDataOn) {
            EstablishAllApnsIfConnectable();
            if (apnManager_ == nullptr) {
                TELEPHONY_LOGE("apnManager is null.");
                return true;
            }
            const int id = DATA_CONTEXT_ROLE_DEFAULT_ID;
            sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
            if (apnHolder == nullptr) {
                TELEPHONY_LOGE("apnHolder is null.");
                return true;
            }
            if (!apnHolder->IsDataCallEnabled()) {
                NetRequest netRequest;
                netRequest.ident = IDENT_PREFIX + std::to_string(slotId_);
                netRequest.capability = NetCapabilities::NET_CAPABILITIES_INTERNET;
                apnHolder->RequestCellularData(netRequest);
                AttemptEstablishDataConnection(apnHolder);
                return true;
            }
        } else {
            ClearAllConnections(REASON_CLEAR_CONNECTION);
        }
    } else {
        TELEPHONY_LOGI("The status of the cellular data switch has not changed");
    }
    return true;
}

bool CellularDataHandler::IsCellularDataEnabled() const
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("IsCellularDataEnabled dataSwitchSettings_ is null");
        return false;
    }
    return dataSwitchSettings_->GetUserDataOn();
}

bool CellularDataHandler::IsCellularDataRoamingEnabled() const
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("IsCellularDataRoamingEnabled dataSwitchSettings_ is null");
        return false;
    }
    return dataSwitchSettings_->IsUserDataRoamingOn();
}

bool CellularDataHandler::SetCellularDataRoamingEnabled(bool dataRoamingEnabled)
{
    if (dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("dataSwitchSettings_ or apnManager_ is null");
        return false;
    }
    if (dataSwitchSettings_->IsUserDataRoamingOn() != dataRoamingEnabled) {
        dataSwitchSettings_->SetUserDataRoamingOn(dataRoamingEnabled);
        bool roamingState = NetworkSearchUtils::GetRoamingState(slotId_);
        if (roamingState) {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
                apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
                ClearAllConnections(REASON_RETRY_CONNECTION);
            }
            EstablishAllApnsIfConnectable();
        } else {
            TELEPHONY_LOGI("Not roaming(%{public}d), not doing anything", roamingState);
        }
    } else {
        TELEPHONY_LOGI("The roaming switch status has not changed");
    }
    return true;
}

void CellularDataHandler::ClearAllConnections(DisConnectionReason reason)
{
    int32_t networkType = NetworkSearchUtils::GetPsRadioTech(slotId_);
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("ClearAllConnections:apnManager is null");
        return;
    }
    StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_DISCONNECTING, networkType);
    for (sptr<ApnHolder> apn : apnManager_->GetAllApnHolder()) {
        ClearConnection(apn, reason);
    }
    StopStallDetectionTimer();
    if (connectionManager_ != nullptr) {
        connectionManager_->EndNetStatistics();
    }
    if (!dataSwitchSettings_->GetUserDataOn()) {
        connectionManager_->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    }
}

void CellularDataHandler::ClearConnection(sptr<ApnHolder> &apn, DisConnectionReason reason)
{
    if (apn == nullptr) {
        TELEPHONY_LOGE("ClearConnection:apnHolder is null");
        return;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = apn->GetCellularDataStateMachine();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    TELEPHONY_LOGI("The APN holder is of type %{public}s", apn->GetApnType().c_str());
    std::unique_ptr<DataDisconnectParams> object = std::make_unique<DataDisconnectParams>(apn->GetApnType(), reason);
    if (object == nullptr) {
        TELEPHONY_LOGE("ClearConnection fail, object is null");
        return;
    }
    AppExecFwk::InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, object);
    stateMachine->SendEvent(event);
    apn->SetApnState(PROFILE_STATE_DISCONNECTING);
    apn->SetCellularDataStateMachine(nullptr);
}

ApnProfileState CellularDataHandler::GetCellularDataState() const
{
    return apnManager_->GetOverallApnState();
}

void CellularDataHandler::RadioPsConnectionAttached(const InnerEvent::Pointer &event)
{
    if (event == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("event or apnManager_ is null");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RadioPsConnectionDetached(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    ClearAllConnections(REASON_CLEAR_CONNECTION);
}

void CellularDataHandler::RoamingStateOn(const InnerEvent::Pointer &event)
{
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("event or dataSwitchSettings_ or apnManager_ is null");
        return;
    }
    bool roamingState = NetworkSearchUtils::GetRoamingState(slotId_);
    if (!roamingState) {
        TELEPHONY_LOGE("device  not currently roaming state");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RoamingStateOff(const InnerEvent::Pointer &event)
{
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("event or dataSwitchSettings_ or apnManager_ is null");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::PsRadioEmergencyStateOpen(const InnerEvent::Pointer &event)
{
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_CONNECTED ||
        currentState == ApnProfileState::PROFILE_STATE_CONNECTING) {
        ClearAllConnections(REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::PsRadioEmergencyStateClose(const InnerEvent::Pointer &event)
{
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_IDLE ||
        currentState == ApnProfileState::PROFILE_STATE_DISCONNECTING) {
        EstablishAllApnsIfConnectable();
    }
}

void CellularDataHandler::EstablishAllApnsIfConnectable()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("EstablishAllApnsIfConnectable:apnManager is null");
        return;
    }
    for (sptr<ApnHolder> apnHolder : apnManager_->GetSortApnHolder()) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("apn is null");
            continue;
        }
        TELEPHONY_LOGI("IsDataCallEnabled := %{public}d, type:= %{public}s, state:= %{public}d",
            apnHolder->IsDataCallEnabled(), apnHolder->GetApnType().c_str(), apnHolder->GetApnState());
        if (apnHolder->IsDataCallEnabled()) {
            ApnProfileState apnState = apnHolder->GetApnState();
            if (apnState == PROFILE_STATE_FAILED || apnState == PROFILE_STATE_RETRYING) {
                apnHolder->ReleaseDataConnection();
            }
            if (apnHolder->IsDataCallConnectable()) {
                AttemptEstablishDataConnection(apnHolder);
            }
        }
    }
}

void CellularDataHandler::AttemptEstablishDataConnection(sptr<ApnHolder> &apnHolder)
{
    if (dataSwitchSettings_ == nullptr || apnManager_ == nullptr || apnHolder == nullptr) {
        TELEPHONY_LOGE("dataSwitchSettings_ or apnManager_ is null");
        return;
    }
    bool attached = NetworkSearchUtils::GetAttachedState(slotId_);
    int32_t simState = SimUtils::GetSimState(slotId_);
    TELEPHONY_LOGI("attached: %{public}d simState: %{public}d", attached, simState);
    bool isEmergencyApn = apnHolder->IsEmergencyType();
    bool isAllowActiveData = dataSwitchSettings_->IsAllowActiveData();
    if (!isEmergencyApn && (!attached || (simState != (int32_t)SimState::SIM_STATE_READY))) {
        TELEPHONY_LOGE("attached:%{public}d simState:%{public}d", attached, simState);
        return;
    }
    if (NetworkSearchUtils::GetRoamingState(slotId_) && !dataSwitchSettings_->IsUserDataRoamingOn()) {
        isAllowActiveData = false;
    }
    if (isEmergencyApn) {
        isAllowActiveData = true;
    }
    if (IsRestrictedMode()) {
        TELEPHONY_LOGE("lastCallState_ = %{public}d", lastCallState_);
        isAllowActiveData = false;
    }
    if (!isAllowActiveData) {
        TELEPHONY_LOGE("isAllowActiveData:%{public}d attached:%{public}d simState:%{public}d", isAllowActiveData,
            attached, simState);
        return;
    }
    if (apnHolder->GetApnState() == PROFILE_STATE_FAILED) {
        apnHolder->SetApnState(PROFILE_STATE_IDLE);
    }
    int32_t radioTech = NetworkSearchUtils::GetPsRadioTech(slotId_);
    if (apnHolder->GetApnState() == PROFILE_STATE_IDLE) {
        std::vector<sptr<ApnItem>> matchedApns = apnManager_->FilterMatchedApns(apnHolder->GetApnType(), radioTech);
        if (matchedApns.empty()) {
            TELEPHONY_LOGE("AttemptEstablishDataConnection:matchedApns is empty");
            return;
        }
        apnHolder->SetAllMatchedApns(matchedApns);
        if (!EstablishDataConnection(apnHolder, radioTech)) {
            TELEPHONY_LOGE("EstablishDataConnection fail");
        }
    } else {
        TELEPHONY_LOGI("The APN holder is not idle");
    }
}

std::shared_ptr<CellularDataStateMachine> CellularDataHandler::FindIdleCellularDataConnection() const
{
    for (const std::shared_ptr<CellularDataStateMachine> &stateMachine : intStateMachineMap_) {
        if (stateMachine == nullptr || apnManager_ == nullptr) {
            TELEPHONY_LOGE("CellularDataHandler:stateMachine or apnManager_ is null");
            return nullptr;
        }
        if (stateMachine->IsInactiveState() && apnManager_->IsDataConnectionNotUsed(stateMachine)) {
            return stateMachine;
        }
    }
    return nullptr;
}

std::shared_ptr<CellularDataStateMachine> CellularDataHandler::CreateCellularDataConnect()
{
    if (stateMachineEventLoop_ == nullptr) {
        stateMachineEventLoop_ = AppExecFwk::EventRunner::Create("CellularDataStateMachine");
        if (stateMachineEventLoop_ == nullptr) {
            TELEPHONY_LOGE("failed to create EventRunner");
            return nullptr;
        }
        stateMachineEventLoop_->Run();
    }
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine =
        std::make_shared<CellularDataStateMachine>(connectionManager_, shared_from_this(), stateMachineEventLoop_);
    if (cellularDataStateMachine == nullptr) {
        TELEPHONY_LOGE("cellularDataStateMachine is null");
        return nullptr;
    }
    intStateMachineMap_.push_back(cellularDataStateMachine);
    return cellularDataStateMachine;
}

bool CellularDataHandler::EstablishDataConnection(sptr<ApnHolder> &apnHolder, int32_t radioTech)
{
    if (apnHolder == nullptr || apnManager_ == nullptr || connectionManager_ == nullptr) {
        TELEPHONY_LOGE("apnHolder or apnManager or connectionManager is null");
        return false;
    }
    if (SimUtils::GetDefaultCellularDataSlotId() != slotId_) {
        TELEPHONY_LOGE("a change in the card");
        return false;
    }
    sptr<ApnItem> apnItem = apnHolder->GetNextRetryApn();
    if (apnItem == nullptr) {
        TELEPHONY_LOGE("apnItem is null");
        return false;
    }
    int32_t profileId = apnItem->attr_.profileId_;
    profileId == INVALID_PROFILE_ID ? apnHolder->GetProfileId(apnHolder->GetApnType()) : profileId;
    if (HasAnyHigherPriorityConnection(apnHolder)) {
        TELEPHONY_LOGE("has higher priority connection");
        return false;
    } else if (apnManager_->GetOverallApnState() == ApnProfileState::PROFILE_STATE_CONNECTED ||
        apnManager_->GetOverallApnState() == ApnProfileState::PROFILE_STATE_CONNECTING) {
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine = FindIdleCellularDataConnection();
    if (cellularDataStateMachine == nullptr) {
        cellularDataStateMachine = CreateCellularDataConnect();
        if (cellularDataStateMachine == nullptr) {
            TELEPHONY_LOGE("cellularDataStateMachine is null");
            return false;
        }
        cellularDataStateMachine->Init();
    }
    connectionManager_->AddConnectionStateMachine(cellularDataStateMachine);
    cellularDataStateMachine->SetCapability(apnHolder->GetCapability());
    apnHolder->SetCurrentApn(apnItem);
    apnHolder->SetApnState(PROFILE_STATE_CONNECTING);
    apnHolder->SetCellularDataStateMachine(cellularDataStateMachine);
    bool roamingState = NetworkSearchUtils::GetRoamingState(slotId_);
    bool userDataRoaming = dataSwitchSettings_->IsUserDataRoamingOn();
    StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_CONNECTING, radioTech);
    std::unique_ptr<DataConnectionParams> object =
        std::make_unique<DataConnectionParams>(apnHolder, profileId, radioTech, roamingState, userDataRoaming, true);
    TELEPHONY_LOGI("MSG_SM_CONNECT profileId:%{public}d type:%{public}s networkType:%{public}d",
        profileId, apnHolder->GetApnType().c_str(), radioTech);
    AppExecFwk::InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, object);
    cellularDataStateMachine->SendEvent(event);
    return true;
}

void CellularDataHandler::EstablishDataConnectionComplete(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo =
        event->GetSharedObject<SetupDataCallResultInfo>();
    if ((setupDataCallResultInfo != nullptr) && (apnManager_ != nullptr)) {
        TELEPHONY_LOGI("setupDataCallResultInfo->flag:%{public}d", setupDataCallResultInfo->flag);
        sptr<ApnHolder> apnHolder =
            apnManager_->GetApnHolder(apnManager_->FindApnNameByApnId(setupDataCallResultInfo->flag));
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("MSG_ESTABLISH_DATA_CONNECTION_COMPLETE apnHolder is null");
            return;
        }
        apnHolder->SetApnState(PROFILE_STATE_CONNECTED);
        apnHolder->InitialApnRetryCount();
        int32_t networkType = NetworkSearchUtils::GetPsRadioTech(slotId_);
        StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_CONNECTED, networkType);
        CheckAndUpdateNetInfo(apnHolder, setupDataCallResultInfo);
        StartStallDetectionTimer();
        if (connectionManager_ != nullptr) {
            connectionManager_->BeginNetStatistics();
        }
        if (!physicalConnectionActiveState_) {
            physicalConnectionActiveState_ = true;
            NetworkSearchUtils::DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
        }
    }
}

void CellularDataHandler::CheckAndUpdateNetInfo(
    sptr<ApnHolder> &apnHolder, std::shared_ptr<SetupDataCallResultInfo> &infos) const
{
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is null");
        return;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = apnHolder->GetCellularDataStateMachine();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("start update net info,but stateMachine is null!");
        return;
    }
    if (infos == nullptr) {
        TELEPHONY_LOGE("infos is null");
        return;
    }
    stateMachine->UpdateNetworkInfo(*infos);
}

int32_t CellularDataHandler::GetSlotId() const
{
    return slotId_;
}

void CellularDataHandler::DisconnectDataComplete(const InnerEvent::Pointer &event)
{
    if ((event == nullptr) || (apnManager_ == nullptr)) {
        TELEPHONY_LOGE("event or  apnManager is null");
        return;
    }
    std::unique_ptr<DataDisconnectParams> object = event->GetUniqueObject<DataDisconnectParams>();
    int32_t apnId = apnManager_->FindApnIdByApnName(object->GetApnType());
    DisConnectionReason reason = object->GetReason();
    TELEPHONY_LOGI("apn id:%{public}d", apnId);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(apnId);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is null");
        return;
    }
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    int32_t networkType = NetworkSearchUtils::GetPsRadioTech(slotId_);
    StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_IDLE, networkType);
    TELEPHONY_LOGI("Apn holder data call: %{public}d", apnHolder->IsDataCallEnabled());
    bool noActiveConnection = connectionManager_->isNoActiveConnection();
    if (noActiveConnection && physicalConnectionActiveState_) {
        physicalConnectionActiveState_ = false;
        NetworkSearchUtils::DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
    } else if (!noActiveConnection && !physicalConnectionActiveState_) {
        physicalConnectionActiveState_ = true;
        NetworkSearchUtils::DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
    }
    if (apnHolder->IsDataCallEnabled() && reason == REASON_RETRY_CONNECTION) {
        if (apnHolder->GetApnState() == PROFILE_STATE_IDLE || apnHolder->GetApnState() == PROFILE_STATE_FAILED) {
            apnHolder->SetCellularDataStateMachine(nullptr);
        }
        int64_t delayTime = apnHolder->GetRetryDelay();
        TELEPHONY_LOGI("Establish a data connection. The apn type is %{public}s", apnHolder->GetApnType().c_str());
        SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, apnId, delayTime);
    }
    if (!apnManager_->HasAnyConnectedState()) {
        if (connectionManager_ != nullptr) {
            connectionManager_->StopStallDetectionTimer();
            connectionManager_->EndNetStatistics();
        }
    }
}

void CellularDataHandler::MsgEstablishDataConnection(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr && event == nullptr) {
        TELEPHONY_LOGE("apnManager_ or event is null");
        return;
    }
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(event->GetParam());
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is null");
        return;
    }
    TELEPHONY_LOGI("Establish a data connection. The APN holder data call %{public}d ", apnHolder->IsDataCallEnabled());
    if (apnHolder->IsDataCallEnabled()) {
        AttemptEstablishDataConnection(apnHolder);
    } else {
        ClearConnection(apnHolder, REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::MsgRequestNetwork(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr && event == nullptr) {
        TELEPHONY_LOGE("apnManager_ or event is null");
        return;
    }
    std::unique_ptr<NetRequest> netRequest = event->GetUniqueObject<NetRequest>();
    if (netRequest == nullptr) {
        TELEPHONY_LOGE("netRequest is null");
        return;
    }
    NetRequest request;
    request.ident = netRequest->ident;
    request.capability = netRequest->capability;
    int32_t id = ApnManager::FindApnIdByCapability(request.capability);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is null.");
        return;
    }
    if (event->GetParam() == TYPE_REQUEST_NET) {
        apnHolder->RequestCellularData(request);
    } else {
        apnHolder->ReleaseCellularData(request);
        if (apnHolder->IsDataCallEnabled()) {
            return;
        }
    }
    AppExecFwk::InnerEvent::Pointer innerEvent =
        InnerEvent::Get(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, id);
    (void)SendEvent(innerEvent);
}

void CellularDataHandler::ProcessEvent(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("start ProcessEvent but event is null!");
        return;
    }
    uint32_t eventCode = event->GetInnerEventId();
    std::map<uint32_t, Fun>::iterator it = eventIdMap_.find(eventCode);
    if (it != eventIdMap_.end()) {
        (this->*(it->second))(event);
    }
}

void CellularDataHandler::HandleSettingSwitchChanged(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    bool setting_switch = event->GetParam();
    TELEPHONY_LOGI("setting switch = %{public}d", setting_switch);
}

void CellularDataHandler::HandleVoiceCallChanged(int32_t state)
{
    if (apnManager_ == nullptr || connectionManager_ == nullptr) {
        TELEPHONY_LOGE("HandleVoiceCallChanged start apnManager or connectionManager is null!");
        return;
    }
    TELEPHONY_LOGI("handle voice call changed lastState:%{public}d, state:%{public}d", lastCallState_, state);
    // next to check if radio technology support voice and data at same time.
    bool support = NetworkSearchUtils::GetPsRadioTech(slotId_) == (int32_t)RadioTech::RADIO_TECHNOLOGY_GSM;
    if (lastCallState_ != state) {
        // call is busy
        lastCallState_ = state;
        if (state != TelCallStatus::CALL_STATE_RELEASED) {
            if (apnManager_->HasAnyConnectedState() && support) {
                connectionManager_->EndNetStatistics();
                connectionManager_->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
                StopStallDetectionTimer();
                disconnectionReason_ = REASON_GSM_AND_CALLING_ONLY;
            }
        } else {
            if (apnManager_->HasAnyConnectedState() && support) {
                connectionManager_->BeginNetStatistics();
                StartStallDetectionTimer();
            }
            disconnectionReason_ = REASON_NORMAL;
            TELEPHONY_LOGI("HandleVoiceCallChanged EstablishAllApnsIfConnectable");
            EstablishAllApnsIfConnectable();
        }
        TELEPHONY_LOGI("disconnectionReason_=%{public}d", disconnectionReason_);
    }
}

void CellularDataHandler::HandleSimStateOrRecordsChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case ObserverHandler::RADIO_SIM_STATE_CHANGE: {
            int32_t simState = SimUtils::GetSimState(slotId_);
            TELEPHONY_LOGI("sim state is :%{public}d", simState);
            if (simState != static_cast<int32_t>(SimState::SIM_STATE_READY)) {
                ClearAllConnections(REASON_CLEAR_CONNECTION);
            } else {
                if (lastIccID_ != u"" && lastIccID_ == SimUtils::GetSimIccId(slotId_)) {
                    EstablishAllApnsIfConnectable();
                }
            }
            break;
        }
        case ObserverHandler::RADIO_SIM_RECORDS_LOADED: {
            std::u16string iccID = SimUtils::GetSimIccId(slotId_);
            int32_t simState = SimUtils::GetSimState(slotId_);
            TELEPHONY_LOGI("sim state is :%{public}d, iccId is %{public}s", simState, Str16ToStr8(iccID).c_str());
            if (simState == static_cast<int32_t >(SimState::SIM_STATE_READY) && iccID != u"") {
                if (iccID != lastIccID_) {
                    GetConfigurationFor5G();
                    lastIccID_ = iccID;
                    AppExecFwk::InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_APN_CHANGED);
                    SendEvent(event);
                } else if (lastIccID_ == iccID) {
                    TELEPHONY_LOGI("sim state changed, but iccID not changed.");
                    // the sim card status has changed to ready, so try to connect
                    EstablishAllApnsIfConnectable();
                }
            }
            break;
        }
        default:
            break;
    }
}

sptr<ApnManager> CellularDataHandler::GetApnManager() const
{
    return apnManager_;
}

int32_t CellularDataHandler::HandleApnChanged(const std::string &apns)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager_ is null");
        return 0;
    }
    TELEPHONY_LOGI("ApnChanged apn:=  %{public}s", apns.c_str());
    std::string numeric = NetworkSearchUtils::GetOperatorNumeric(slotId_);
    int32_t result = 0;
    for (int32_t i = 0; i < DEFAULT_READ_APN_TIME; ++i) {
        result = apnManager_->CreateAllApnItemByDatabase(numeric);
        if (result != 0) {
            break;
        }
    }
    if (result == 0) {
        apnManager_->CreateAllApnItem();
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(REASON_RETRY_CONNECTION);
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        if (apnHolder == nullptr) {
            continue;
        }
        int32_t id = apnManager_->FindApnIdByApnName(apnHolder->GetApnType());
        SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, id, ESTABLISH_DATA_CONNECTION_DELAY);
    }
    return result;
}

void CellularDataHandler::HandleApnChanged(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager_ is null");
        return;
    }
    std::string numeric = NetworkSearchUtils::GetOperatorNumeric(slotId_);
    int32_t result = 0;
    for (int32_t i = 0; i < DEFAULT_READ_APN_TIME; ++i) {
        result = apnManager_->CreateAllApnItemByDatabase(numeric);
        if (result != 0) {
            break;
        }
    }
    if (result == 0) {
        apnManager_->CreateAllApnItem();
    }
    SetRilAttachApn();
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(REASON_RETRY_CONNECTION);
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        if (apnHolder == nullptr) {
            continue;
        }
        int32_t id = apnManager_->FindApnIdByApnName(apnHolder->GetApnType());
        SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, id, ESTABLISH_DATA_CONNECTION_DELAY);
    }
}

int32_t CellularDataHandler::GetCellularDataFlowType()
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("connection manager is null");
        return 0;
    }
    return connectionManager_->GetDataFlowType();
}

void CellularDataHandler::HandleRadioStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr || event == nullptr) {
        TELEPHONY_LOGE("radio off or not available apnManager or event is null!");
        return;
    }
    std::shared_ptr<int> object = event->GetSharedObject<int>();
    if (object == nullptr) {
        TELEPHONY_LOGE("HandleRadioStateChanged object is nullptr!");
        return;
    }
    TELEPHONY_LOGI("Radio changed with state: %{public}d", *object);
    switch (*object) {
        case CORE_SERVICE_POWER_OFF:
        case CORE_SERVICE_POWER_NOT_AVAILABLE: {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            TELEPHONY_LOGI("apn state is %{public}d", apnState);
            if (apnState != ApnProfileState::PROFILE_STATE_IDLE) {
                ClearAllConnections(REASON_CLEAR_CONNECTION);
            }
            break;
        }
        case CORE_SERVICE_POWER_ON:
            SetRilLinkBandwidths();
            EstablishAllApnsIfConnectable();
            break;
        default:
            TELEPHONY_LOGI("un-handle state:%{public}d", *object);
            break;
    }
}

void CellularDataHandler::PsDataRatChanged(const InnerEvent::Pointer &event)
{
    int32_t radioTech = NetworkSearchUtils::GetPsRadioTech(slotId_);
    TELEPHONY_LOGI("radioTech is %{public}d", radioTech);
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    if (!IsCellularDataEnabled()) {
        TELEPHONY_LOGE("data enable is close");
        return;
    }
    bool attached = NetworkSearchUtils::GetAttachedState(slotId_);
    if (!attached) {
        TELEPHONY_LOGE("attached is false ");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::HandleCallStateUpdate(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    std::shared_ptr<CallStatusInfo> callStateInfo = event->GetSharedObject<CallStatusInfo>();
    if (callStateInfo != nullptr) {
        int32_t callId = callStateInfo->callId;
        int32_t status = callStateInfo->status;
        TELEPHONY_LOGI("HandleCallStateUpdate callId=%{public}d, status= %{public}d", callId, status);
        if (status == TelCallStatus::CALL_STATUS_UNKNOWN) {
            return;
        }
        HandleVoiceCallChanged(status);
    }
}

void CellularDataHandler::ClearAllConnectionsFormerSlot()
{
    ClearAllConnections(REASON_CLEAR_CONNECTION);
}

void CellularDataHandler::ConnectDataNeWork()
{
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::SetPolicyDataOn(bool enable)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("dataSwitchSettings is null");
        return;
    }
    bool policyDataOn = dataSwitchSettings_->IsPolicyDataOn();
    if (policyDataOn != enable) {
        dataSwitchSettings_->SetPolicyDataOn(enable);
        if (enable) {
            EstablishAllApnsIfConnectable();
        } else {
            ClearAllConnections(REASON_CLEAR_CONNECTION);
        }
    }
}

bool CellularDataHandler::IsRestrictedMode() const
{
    bool support = NetworkSearchUtils::GetPsRadioTech(slotId_) == (int32_t)RadioTech::RADIO_TECHNOLOGY_GSM;
    bool inCall = lastCallState_ != TelCallStatus::CALL_STATE_RELEASED;
    TELEPHONY_LOGI("radio technology is gsm only:%{public}d and call is busy:%{public}d", support, inCall);
    return inCall && support;
}

int32_t CellularDataHandler::GetDisConnectionReason()
{
    return disconnectionReason_;
}

void CellularDataHandler::SetRilAttachApn()
{
    sptr<ApnItem> attachApn = apnManager_->GetRilAttachApn();
    if (attachApn == nullptr) {
        TELEPHONY_LOGE("attachApn is null");
        return;
    }
    ITelRilManager::CellularDataProfile dataProfile(attachApn->attr_.profileId_, std::string(attachApn->attr_.apn_),
        std::string(attachApn->attr_.protocol_), attachApn->attr_.authType_, std::string(attachApn->attr_.user_),
        std::string(attachApn->attr_.password_), std::string(attachApn->attr_.roamingProtocol_));

    AppExecFwk::InnerEvent::Pointer event =
        InnerEvent::Get(CellularDataEventCode::MSG_SET_RIL_ATTACH_APN);
    event->SetOwner(shared_from_this());
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr) {
        core->SetInitApnInfo(dataProfile, event);
    }
}

void CellularDataHandler::SetRilAttachApnResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    std::shared_ptr<TelRilResponseInfo<int32_t>> rilInfo = event->GetSharedObject<TelRilResponseInfo<int32_t>>();
    if (rilInfo == nullptr) {
        TELEPHONY_LOGE("HRilRadioResponseInfo is null");
        return;
    }
    if (rilInfo->errorNo != 0) {
        TELEPHONY_LOGE("SetRilAttachApn error");
    }
}

bool CellularDataHandler::HasAnyHigherPriorityConnection(const sptr<ApnHolder> &apnHolder)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager is null");
        return false;
    }
    if (apnManager_->GetSortApnHolder().empty()) {
        TELEPHONY_LOGE("SortApnHolder is null");
        return false;
    }
    for (const sptr<ApnHolder> &sortApnHolder : apnManager_->GetSortApnHolder()) {
        if (sortApnHolder->GetPriority() > apnHolder->GetPriority()) {
            if (sortApnHolder->IsDataCallEnabled() &&
                (sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTED ||
                sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTING ||
                sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_DISCONNECTING)) {
                return true;
            }
        }
    }
    return false;
}

bool CellularDataHandler::HasInternetCapability(const int32_t cid) const
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("connectionManager is null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> activeStateMachine = connectionManager_->GetActiveConnectionByCid(cid);
    if (activeStateMachine == nullptr) {
        TELEPHONY_LOGE("get activeStateMachine by cid fail");
        return false;
    }
    int64_t netCapability = static_cast<int64_t>(activeStateMachine->GetCapability());
    if ((netCapability & (1 << (static_cast<int32_t>(NET_CAPABILITIES_INTERNET) - 1))) != 0) {
        return true;
    }
    return false;
}

void CellularDataHandler::GetConfigurationFor5G()
{
    // get 5G configurations
    unMeteredAllNsaConfig_ = ParseOperatorConfig(u"allmeterednas");
    unMeteredNrNsaMmwaveConfig_ = ParseOperatorConfig(u"meterednrnsammware");
    unMeteredNrNsaSub6Config_ = ParseOperatorConfig(u"meteredNrnsasub6");
    unMeteredAllNrsaConfig_ = ParseOperatorConfig(u"meteredallnrsa");
    unMeteredNrsaMmwaveConfig_ = ParseOperatorConfig(u"meterednrsammware");
    unMeteredNrsaSub6Config_ = ParseOperatorConfig(u"meterednrsasub6");
    unMeteredRoamingConfig_ = ParseOperatorConfig(u"meteredroaming");
    GetDefaultConfiguration();
}

bool CellularDataHandler::ParseOperatorConfig(const std::u16string &configName)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    OperatorConfig configsFor5G;
    if (core == nullptr) {
        TELEPHONY_LOGE("get coremanager return null");
        return false;
    }
    core->GetOperatorConfigs(slotId_, configsFor5G);
    if (configsFor5G.configValue.count(configName) > 0) {
        std::string flag = Str16ToStr8(configsFor5G.configValue[configName]);
        TELEPHONY_LOGI("parse operator 5G config: %{public}s", flag.c_str());
        if (flag == "true") {
            return true;
        }
    }
    return false;
}

void CellularDataHandler::GetDefaultConfiguration()
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("connectionManager is null");
        return;
    }
    connectionManager_->GetDefaultBandWidthsConfig();
    connectionManager_->GetDefaultTcpBufferConfig();
    GetDefaultUpLinkThresholdsConfig();
    GetDefaultDownLinkThresholdsConfig();
    defaultMobileMtuConfig_ = CellularDataUtils::GetDefaultMobileMtuConfig();
    TELEPHONY_LOGI("defaultMobileMtuConfig_ = %{public}d", defaultMobileMtuConfig_);
    defaultPreferApn_ = CellularDataUtils::GetDefaultPreferApnConfig();
    TELEPHONY_LOGI("defaultPreferApn_ is %{public}d", defaultPreferApn_);
}

void CellularDataHandler::HandleRadioNrStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("connectionManager is null");
        return;
    }
    TELEPHONY_LOGI("receive HandleRadioNrStateChanged event");
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine>& cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(ObserverHandler::ObserverHandlerId::RADIO_NR_STATE_CHANGED);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::HandleRadioNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("connectionManager is null");
        return;
    }
    TELEPHONY_LOGI("receive HandleRadioNrFrequencyChanged event");
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine>& cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(ObserverHandler::ObserverHandlerId::RADIO_NR_FREQUENCY_CHANGED);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::GetDefaultUpLinkThresholdsConfig()
{
    upLinkThresholds_.clear();
    char upLinkConfig[UP_DOWN_LINK_SIZE] = {0};
    GetParameter(CONFIG_UPLINK_THRESHOLDS.c_str(), CAPACITY_THRESHOLDS_FOR_UPLINK.c_str(),
        upLinkConfig, UP_DOWN_LINK_SIZE);
    TELEPHONY_LOGI("upLinkThresholds = %{public}s", upLinkConfig);
    upLinkThresholds_ = CellularDataUtils::Split(upLinkConfig, ",");
}

void CellularDataHandler::GetDefaultDownLinkThresholdsConfig()
{
    downLinkThresholds_.clear();
    char downLinkConfig[UP_DOWN_LINK_SIZE] = {0};
    GetParameter(CONFIG_DOWNLINK_THRESHOLDS.c_str(), CAPACITY_THRESHOLDS_FOR_DOWNLINK.c_str(),
        downLinkConfig, UP_DOWN_LINK_SIZE);
    TELEPHONY_LOGI("downLinkThresholds_ = %{public}s", downLinkConfig);
    downLinkThresholds_ = CellularDataUtils::Split(downLinkConfig, ",");
}

void CellularDataHandler::SetRilLinkBandwidths()
{
    LinkBandwidthRule linkBandwidth;
    linkBandwidth.rat = NetworkSearchUtils::GetPsRadioTech(slotId_);
    linkBandwidth.delayMs = DELAY_SET_RIL_BANDWIDTH_MS;
    linkBandwidth.delayUplinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    linkBandwidth.delayDownlinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    for (std::string upLinkThreshold : upLinkThresholds_) {
        linkBandwidth.maximumUplinkKbps.push_back(atoi(upLinkThreshold.c_str()));
    }
    for (std::string downLinkThreshold : downLinkThresholds_) {
        linkBandwidth.maximumDownlinkKbps.push_back(atoi(downLinkThreshold.c_str()));
    }
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SET_RIL_BANDWIDTH);
    event->SetOwner(shared_from_this());
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr) {
        TELEPHONY_LOGI("SetRilLinkBandwidths");
        core->SetLinkBandwidthReportingRule(linkBandwidth, event);
    }
}

void CellularDataHandler::HandleDBSettingEnableChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("HandleDBSettingEnableChanged dataSwitchSettings_ is null.");
        return;
    }
    int64_t value = event->GetParam();
    if (dataSwitchSettings_->GetUserDataOn() != value) {
        dataSwitchSettings_->SetUserDataOn(value);
        if (value) {
            EstablishAllApnsIfConnectable();
            if (apnManager_ == nullptr) {
                TELEPHONY_LOGE("apnManager is null.");
                return;
            }
            const int id = DATA_CONTEXT_ROLE_DEFAULT_ID;
            sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
            if (apnHolder == nullptr) {
                TELEPHONY_LOGE("apnHolder is null.");
                return;
            }
            if (!apnHolder->IsDataCallEnabled()) {
                NetRequest netRequest;
                netRequest.ident = IDENT_PREFIX + std::to_string(slotId_);
                netRequest.capability = NetCapabilities::NET_CAPABILITIES_INTERNET;
                apnHolder->RequestCellularData(netRequest);
                AttemptEstablishDataConnection(apnHolder);
                return;
            }
        } else {
            ClearAllConnections(REASON_CLEAR_CONNECTION);
        }
    } else {
        TELEPHONY_LOGI("The status of the cellular data switch has not changed");
    }
}

void CellularDataHandler::HandleDBSettingRoamingChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    if (dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("dataSwitchSettings_ or apnManager_ is null");
        return;
    }
    int64_t value = event->GetParam();
    if (dataSwitchSettings_->IsUserDataRoamingOn() != value) {
        dataSwitchSettings_->SetUserDataRoamingOn(value);
        bool roamingState = NetworkSearchUtils::GetRoamingState(slotId_);
        if (roamingState) {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
                apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
                ClearAllConnections(REASON_RETRY_CONNECTION);
            }
            EstablishAllApnsIfConnectable();
        } else {
            TELEPHONY_LOGI("Not roaming(%{public}d), not doing anything", roamingState);
        }
    } else {
        TELEPHONY_LOGI("The roaming switch status has not changed");
    }
}
} // namespace Telephony
} // namespace OHOS