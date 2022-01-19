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

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_utils.h"
#include "cellular_data_types.h"
#include "hril_call_parcel.h"
#include "str_convert.h"
#include "core_manager_inner.h"
#include "radio_event.h"
#include "telephony_log_wrapper.h"
#include "telephony_types.h"

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
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        if (userDataOn && defaultSlotId == slotId_) {
            EstablishAllApnsIfConnectable();
            if (apnManager_ == nullptr) {
                TELEPHONY_LOGE("apnManager is null.");
                return true;
            }
            const int32_t id = DATA_CONTEXT_ROLE_DEFAULT_ID;
            sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
            if (apnHolder == nullptr) {
                TELEPHONY_LOGE("apnHolder is null.");
                return true;
            }
            if (!apnHolder->IsDataCallEnabled()) {
                NetRequest netRequest;
                netRequest.ident = IDENT_PREFIX + std::to_string(slotId_);
                netRequest.capability = NetCap::NET_CAPABILITY_INTERNET;
                apnHolder->RequestCellularData(netRequest);
                AttemptEstablishDataConnection(apnHolder);
                return true;
            }
        } else {
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
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
        bool roamingState = CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0;
        if (roamingState) {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
                apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
                ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
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
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("ClearAllConnections:apnManager is null");
        return;
    }
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_CONNECTED ||
        currentState == ApnProfileState::PROFILE_STATE_CONNECTING) {
        int32_t networkType = CoreManagerInner::GetInstance().GetPsRadioTech(slotId_);
        StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_,
            PROFILE_STATE_DISCONNECTING, networkType);
    }
    for (const sptr<ApnHolder> &apn : apnManager_->GetAllApnHolder()) {
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

void CellularDataHandler::ClearConnection(const sptr<ApnHolder> &apn, DisConnectionReason reason)
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
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, object);
    stateMachine->SendEvent(event);
    apn->SetApnState(PROFILE_STATE_DISCONNECTING);
    apn->SetCellularDataStateMachine(nullptr);
}

ApnProfileState CellularDataHandler::GetCellularDataState() const
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager is null");
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    return apnManager_->GetOverallApnState();
}

ApnProfileState CellularDataHandler::GetCellularDataState(const std::string &apnType) const
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager is null");
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    sptr<ApnHolder> apnHolder = apnManager_->GetApnHolder(apnType);
    return apnHolder->GetApnState();
}

void CellularDataHandler::RadioPsConnectionAttached(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("slot %{public}d ps attached", slotId_);
    if (event == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("event or apnManager_ is null");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RadioPsConnectionDetached(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("slot %{public}d ps detached", slotId_);
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
}

void CellularDataHandler::RoamingStateOn(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("slot %{public}d roaming on", slotId_);
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("event or dataSwitchSettings_ or apnManager_ is null");
        return;
    }
    bool roamingState = false;
    if (CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0) {
        roamingState = true;
    }
    if (!roamingState) {
        TELEPHONY_LOGE("device  not currently roaming state");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RoamingStateOff(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("slot %{public}d roaming off", slotId_);
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("event or dataSwitchSettings_ or apnManager_ is null");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::PsRadioEmergencyStateOpen(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("slot %{public}d emergency on", slotId_);
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_CONNECTED ||
        currentState == ApnProfileState::PROFILE_STATE_CONNECTING) {
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::PsRadioEmergencyStateClose(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("slot %{public}d emergency off", slotId_);
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
    const int32_t defSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    if (defSlotId != slotId_) {
        TELEPHONY_LOGI("Establish all connect default:%{public}d,current:%{public}d", defSlotId, slotId_);
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
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    if (defSlotId != slotId_) {
        TELEPHONY_LOGI("Attempt establish connect default:%{public}d,current:%{public}d", defSlotId, slotId_);
        return;
    }
    bool attached = coreInner.GetPsRegState(slotId_) == (int32_t)RegServiceState::REG_STATE_IN_SERVICE;
    int32_t simState = coreInner.GetSimState(slotId_);
    TELEPHONY_LOGI("attached: %{public}d simState: %{public}d slot:%{public}d", attached, simState, slotId_);
    bool isEmergencyApn = apnHolder->IsEmergencyType();
    bool isAllowActiveData = dataSwitchSettings_->IsAllowActiveData();
    if (!isEmergencyApn && (!attached || (simState != (int32_t)SimState::SIM_STATE_READY))) {
        return;
    }
    bool roamingState = coreInner.GetPsRoamingState(slotId_) > 0;
    if (roamingState && !dataSwitchSettings_->IsUserDataRoamingOn()) {
        isAllowActiveData = false;
    }
    if (isEmergencyApn) {
        isAllowActiveData = true;
    }
    if (!isAllowActiveData || IsRestrictedMode()) {
        TELEPHONY_LOGE("AllowActiveData:%{public}d lastCallState_:%{public}d", isAllowActiveData, lastCallState_);
        return;
    }
    if (apnHolder->GetApnState() == PROFILE_STATE_FAILED) {
        apnHolder->SetApnState(PROFILE_STATE_IDLE);
    }
    int32_t radioTech = coreInner.GetPsRadioTech(slotId_);
    if (apnHolder->GetApnState() != PROFILE_STATE_IDLE) {
        TELEPHONY_LOGI("The slot:%{public}d APN holder is not idle", slotId_);
        return;
    }
    std::vector<sptr<ApnItem>> matchedApns = apnManager_->FilterMatchedApns(apnHolder->GetApnType());
    if (matchedApns.empty()) {
        TELEPHONY_LOGE("AttemptEstablishDataConnection:matchedApns is empty");
        return;
    }
    apnHolder->SetAllMatchedApns(matchedApns);
    if (!EstablishDataConnection(apnHolder, radioTech)) {
        TELEPHONY_LOGE("Establish slot:%{public}d data connection fail", slotId_);
    }
}

std::shared_ptr<CellularDataStateMachine> CellularDataHandler::FindIdleCellularDataConnection() const
{
    std::vector<std::shared_ptr<CellularDataStateMachine>> allMachines = connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &connect : allMachines) {
        if (connect == nullptr || apnManager_ == nullptr) {
            TELEPHONY_LOGE("CellularDataHandler:stateMachine or apnManager_ is null");
            return nullptr;
        }
        if (connect->IsInactiveState() && apnManager_->IsDataConnectionNotUsed(connect)) {
            return connect;
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
    return cellularDataStateMachine;
}

bool CellularDataHandler::EstablishDataConnection(sptr<ApnHolder> &apnHolder, int32_t radioTech)
{
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
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
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
        connectionManager_->AddConnectionStateMachine(cellularDataStateMachine);
    }
    cellularDataStateMachine->SetCapability(apnHolder->GetCapability());
    apnHolder->SetCurrentApn(apnItem);
    apnHolder->SetApnState(PROFILE_STATE_CONNECTING);
    apnHolder->SetCellularDataStateMachine(cellularDataStateMachine);
    bool roamingState = CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0;
    bool userDataRoaming = dataSwitchSettings_->IsUserDataRoamingOn();
    StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_CONNECTING, radioTech);
    std::unique_ptr<DataConnectionParams> object =
        std::make_unique<DataConnectionParams>(apnHolder, profileId, radioTech, roamingState, userDataRoaming, true);
    TELEPHONY_LOGI("MSG_SM_CONNECT slot:%{public}d profileId:%{public}d type:%{public}s networkType:%{public}d",
        slotId_, profileId, apnHolder->GetApnType().c_str(), radioTech);
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, object);
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
        int32_t networkType = CoreManagerInner::GetInstance().GetPsRadioTech(slotId_);
        StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_CONNECTED, networkType);
        CheckAndUpdateNetInfo(apnHolder, setupDataCallResultInfo);
        StartStallDetectionTimer();
        if (connectionManager_ != nullptr) {
            connectionManager_->BeginNetStatistics();
        }
        if (!physicalConnectionActiveState_) {
            physicalConnectionActiveState_ = true;
            CoreManagerInner::GetInstance().DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
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
        TELEPHONY_LOGE("event or apnManager is null");
        return;
    }
    std::unique_ptr<DataDisconnectParams> object = event->GetUniqueObject<DataDisconnectParams>();
    int32_t apnId = apnManager_->FindApnIdByApnName(object->GetApnType());
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(apnId);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is null, apnId is %{public}d", apnId);
        return;
    }
    DisConnectionReason reason = object->GetReason();
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    int32_t networkType = CoreManagerInner::GetInstance().GetPsRadioTech(slotId_);
    StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_IDLE, networkType);
    TELEPHONY_LOGI("Slot:%{public}d apn type: %{public}s call:%{public}d", slotId_, apnHolder->GetApnType().c_str(),
        apnHolder->IsDataCallEnabled());
    bool noActiveConnection = connectionManager_->isNoActiveConnection();
    if (noActiveConnection && physicalConnectionActiveState_) {
        physicalConnectionActiveState_ = false;
        CoreManagerInner::GetInstance().DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
    } else if (!noActiveConnection && !physicalConnectionActiveState_) {
        physicalConnectionActiveState_ = true;
        CoreManagerInner::GetInstance().DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
    }
    if (apnHolder->IsDataCallEnabled()) {
        if (apnHolder->GetApnState() == PROFILE_STATE_IDLE || apnHolder->GetApnState() == PROFILE_STATE_FAILED) {
            apnHolder->SetCellularDataStateMachine(nullptr);
        }
        if (reason == DisConnectionReason::REASON_RETRY_CONNECTION) {
            int64_t delayTime = apnHolder->GetRetryDelay();
            TELEPHONY_LOGI("Establish a data connection. The apn type is %{public}s", apnHolder->GetApnType().c_str());
            SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, apnId, delayTime);
        }
    }
    if (connectionManager_ != nullptr && !apnManager_->HasAnyConnectedState()) {
        connectionManager_->StopStallDetectionTimer();
        connectionManager_->EndNetStatistics();
    }
    if (reason == DisConnectionReason::REASON_CHANGE_CONNECTION) {
        HandleSortConnection();
    }
}

void CellularDataHandler::HandleSortConnection()
{
    ApnProfileState state = apnManager_->GetOverallApnState();
    if (state == PROFILE_STATE_IDLE || state == PROFILE_STATE_FAILED) {
        for (const sptr<ApnHolder> &sortApnHolder : apnManager_->GetSortApnHolder()) {
            if (sortApnHolder->IsDataCallEnabled()) {
                int64_t delayTime = sortApnHolder->GetRetryDelay();
                int32_t apnId = apnManager_->FindApnIdByApnName(sortApnHolder->GetApnType());
                TELEPHONY_LOGI("HandleSortConnection the apn type is %{public}s", sortApnHolder->GetApnType().c_str());
                SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, apnId, delayTime);
                break;
            }
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
    TELEPHONY_LOGI("Establish a data connection. APN holder type:%{public}s call:%{public}d",
        apnHolder->GetApnType().c_str(), apnHolder->IsDataCallEnabled());
    if (apnHolder->IsDataCallEnabled()) {
        AttemptEstablishDataConnection(apnHolder);
    } else {
        ClearConnection(apnHolder, DisConnectionReason::REASON_CHANGE_CONNECTION);
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
    InnerEvent::Pointer innerEvent = InnerEvent::Get(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, id);
    if (!SendEvent(innerEvent)) {
        TELEPHONY_LOGE("send data connection event failed");
    }
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
    bool support = CoreManagerInner::GetInstance().GetPsRadioTech(slotId_) == (int32_t)RadioTech::RADIO_TECHNOLOGY_GSM;
    if (lastCallState_ != state) {
        // call is busy
        lastCallState_ = state;
        if (state != TelCallStatus::CALL_STATE_RELEASED) {
            if (apnManager_->HasAnyConnectedState() && support) {
                connectionManager_->EndNetStatistics();
                connectionManager_->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
                StopStallDetectionTimer();
                disconnectionReason_ = DisConnectionReason::REASON_GSM_AND_CALLING_ONLY;
            }
        } else {
            if (apnManager_->HasAnyConnectedState() && support) {
                connectionManager_->BeginNetStatistics();
                StartStallDetectionTimer();
            }
            disconnectionReason_ = DisConnectionReason::REASON_NORMAL;
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
        case RadioEvent::RADIO_SIM_STATE_CHANGE: {
            int32_t simState = CoreManagerInner::GetInstance().GetSimState(slotId_);
            TELEPHONY_LOGI("sim state is :%{public}d", simState);
            if (simState != static_cast<int32_t>(SimState::SIM_STATE_READY)) {
                ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
            } else {
                if (lastIccID_ != u"" && lastIccID_ == CoreManagerInner::GetInstance().GetSimIccId(slotId_)) {
                    EstablishAllApnsIfConnectable();
                }
            }
            break;
        }
        case RadioEvent::RADIO_SIM_RECORDS_LOADED: {
            std::u16string iccID = CoreManagerInner::GetInstance().GetSimIccId(slotId_);
            int32_t simState = CoreManagerInner::GetInstance().GetSimState(slotId_);
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

bool CellularDataHandler::HandleApnChanged()
{
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_APN_CHANGED);
    if (event == nullptr) {
        TELEPHONY_LOGE("get apn changed event is null");
        return false;
    }
    return SendEvent(event);
}

void CellularDataHandler::HandleApnChanged(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager_ is null");
        return;
    }
    std::string numeric = Str16ToStr8(CoreManagerInner::GetInstance().GetSimOperatorNumeric(slotId_));
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
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
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
    std::shared_ptr<HrilInt32Parcel> object = event->GetSharedObject<HrilInt32Parcel>();
    if (object == nullptr) {
        TELEPHONY_LOGE("HandleRadioStateChanged object is nullptr!");
        return;
    }
    TELEPHONY_LOGI("Radio changed with state: %{public}d slot:%{public}d", object->data, slotId_);
    switch (object->data) {
        case CORE_SERVICE_POWER_OFF:
        case CORE_SERVICE_POWER_NOT_AVAILABLE: {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            TELEPHONY_LOGI("apn state is %{public}d", apnState);
            if (apnState != ApnProfileState::PROFILE_STATE_IDLE) {
                ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
            }
            break;
        }
        case CORE_SERVICE_POWER_ON:
            SetRilLinkBandwidths();
            EstablishAllApnsIfConnectable();
            break;
        default:
            TELEPHONY_LOGI("un-handle state:%{public}d", object->data);
            break;
    }
}

void CellularDataHandler::PsDataRatChanged(const InnerEvent::Pointer &event)
{
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    int32_t radioTech = coreInner.GetPsRadioTech(slotId_);
    TELEPHONY_LOGI("radioTech slot %{public}d is %{public}d", slotId_, radioTech);
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    if (!IsCellularDataEnabled()) {
        TELEPHONY_LOGE("slot %{public}d data enable is close", slotId_);
        return;
    }
    bool attached = coreInner.GetPsRegState(slotId_) == (int32_t)RegServiceState::REG_STATE_IN_SERVICE;
    if (!attached) {
        TELEPHONY_LOGE("attached is false ");
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
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
        TELEPHONY_LOGI("HandleCallStateUpdate slot:%{public}d callId:%{public}d,status:%{public}d",
            slotId_, callId, status);
        if (status == TelCallStatus::CALL_STATUS_UNKNOWN) {
            return;
        }
        HandleVoiceCallChanged(status);
    }
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
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        }
    }
}

bool CellularDataHandler::IsRestrictedMode() const
{
    bool support = CoreManagerInner::GetInstance().GetPsRadioTech(slotId_) == (int32_t)RadioTech::RADIO_TECHNOLOGY_GSM;
    bool inCall = lastCallState_ != TelCallStatus::CALL_STATE_RELEASED;
    TELEPHONY_LOGI("radio technology is gsm only:%{public}d and call is busy:%{public}d", support, inCall);
    return inCall && support;
}

DisConnectionReason CellularDataHandler::GetDisConnectionReason()
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
    DataProfile dataProfile;
    dataProfile.profileId = attachApn->attr_.profileId_;
    dataProfile.apn = attachApn->attr_.apn_;
    dataProfile.protocol = attachApn->attr_.protocol_;
    dataProfile.verType = attachApn->attr_.authType_;
    dataProfile.userName = attachApn->attr_.user_;
    dataProfile.password = attachApn->attr_.password_;
    dataProfile.roamingProtocol = attachApn->attr_.roamingProtocol_;
    CoreManagerInner::GetInstance().SetInitApnInfo(slotId_,
        CellularDataEventCode::MSG_SET_RIL_ATTACH_APN, dataProfile, shared_from_this());
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
    std::vector<sptr<ApnHolder>> sortApnHolders = apnManager_->GetSortApnHolder();
    if (sortApnHolders.empty()) {
        TELEPHONY_LOGE("SortApnHolder is null");
        return false;
    }
    for (const sptr<ApnHolder> &sortApnHolder : sortApnHolders) {
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
    uint64_t netCapability = activeStateMachine->GetCapability();
    if (netCapability == NetCap::NET_CAPABILITY_INTERNET) {
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
    OperatorConfig configsFor5G;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, configsFor5G);
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
    TELEPHONY_LOGI("slot:%{public}d receive HandleRadioNrStateChanged event", slotId_);
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine>& cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(RadioEvent::RADIO_NR_STATE_CHANGED);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::HandleRadioNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("connectionManager is null");
        return;
    }
    TELEPHONY_LOGI("slot:%{public}d receive HandleRadioNrFrequencyChanged event", slotId_);
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine>& cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(RadioEvent::RADIO_NR_FREQUENCY_CHANGED);
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
    linkBandwidth.rat = CoreManagerInner::GetInstance().GetPsRadioTech(slotId_);
    linkBandwidth.delayMs = DELAY_SET_RIL_BANDWIDTH_MS;
    linkBandwidth.delayUplinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    linkBandwidth.delayDownlinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    for (std::string upLinkThreshold : upLinkThresholds_) {
        linkBandwidth.maximumUplinkKbps.push_back(atoi(upLinkThreshold.c_str()));
    }
    for (std::string downLinkThreshold : downLinkThresholds_) {
        linkBandwidth.maximumDownlinkKbps.push_back(atoi(downLinkThreshold.c_str()));
    }
    CoreManagerInner::GetInstance().SetLinkBandwidthReportingRule(slotId_, CellularDataEventCode::MSG_SET_RIL_BANDWIDTH,
        linkBandwidth, shared_from_this());
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
            const int32_t id = DATA_CONTEXT_ROLE_DEFAULT_ID;
            sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
            if (apnHolder == nullptr) {
                TELEPHONY_LOGE("apnHolder is null.");
                return;
            }
            if (!apnHolder->IsDataCallEnabled()) {
                NetRequest netRequest;
                netRequest.ident = IDENT_PREFIX + std::to_string(slotId_);
                netRequest.capability = NetCap::NET_CAPABILITY_INTERNET;
                apnHolder->RequestCellularData(netRequest);
                AttemptEstablishDataConnection(apnHolder);
                return;
            }
        } else {
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
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
        bool roamingState = false;
        if (CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0) {
            roamingState = true;
        }
        if (roamingState) {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
                apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
                ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
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