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

#include "cellular_data_constant.h"
#include "cellular_data_error.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_types.h"
#include "cellular_data_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "core_manager_inner.h"
#include "hitrace_meter.h"
#include "hril_call_parcel.h"
#include "net_specifier.h"
#include "radio_event.h"
#include "str_convert.h"
#include "string_ex.h"
#include "telephony_log_wrapper.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;
using namespace OHOS::EventFwk;
using namespace NetManagerStandard;
CellularDataHandler::CellularDataHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
    const EventFwk::CommonEventSubscribeInfo &sp, int32_t slotId)
    : EventHandler(runner), CommonEventSubscriber(sp), slotId_(slotId)
{}

void CellularDataHandler::Init()
{
    apnManager_ = std::make_unique<ApnManager>().release();
    dataSwitchSettings_ = std::make_unique<DataSwitchSettings>(slotId_);
    connectionManager_ = std::make_unique<DataConnectionManager>(GetEventRunner(), slotId_).release();
    settingObserver_ = std::make_unique<CellularDataSettingObserver>(shared_from_this()).release();
    roamingObserver_ = std::make_unique<CellularDataRoamingObserver>(shared_from_this(), slotId_).release();
    if ((apnManager_ == nullptr) || (dataSwitchSettings_ == nullptr) || (connectionManager_ == nullptr)) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or dataSwitchSettings_ or connectionManager_ is null", slotId_);
        return;
    }
    apnManager_->InitApnHolders();
    apnManager_->CreateAllApnItem();
    dataSwitchSettings_->LoadSwitchValue();
    GetConfigurationFor5G();
    SetRilLinkBandwidths();
}

CellularDataHandler::~CellularDataHandler()
{
    UnRegisterDataSettingObserver();
}

bool CellularDataHandler::ReleaseNet(const NetRequest &request)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null.", slotId_);
        return false;
    }
    int32_t id = ApnManager::FindApnIdByCapability(request.capability);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ReleaseNet apnHolder is null.", slotId_);
        return false;
    }
    std::unique_ptr<NetRequest> netRequest = std::make_unique<NetRequest>();
    if (netRequest == nullptr) {
        TELEPHONY_LOGE("Netrequest is null");
        return false;
    }
    netRequest->capability = request.capability;
    netRequest->ident = request.ident;
    AppExecFwk::InnerEvent::Pointer event =
        InnerEvent::Get(CellularDataEventCode::MSG_REQUEST_NETWORK, netRequest, TYPE_RELEASE_NET);
    return SendEvent(event);
}

bool CellularDataHandler::RequestNet(const NetRequest &request)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null.", slotId_);
        return false;
    }
    int32_t id = ApnManager::FindApnIdByCapability(request.capability);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: RequestNet apnHolder is null.", slotId_);
        return false;
    }
    ApnProfileState apnState = apnHolder->GetApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTED || apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_DISCONNECTING) {
        TELEPHONY_LOGE("Slot%{public}d: RequestNet apn state is connected(%{public}d).", slotId_, apnState);
        return true;
    }
    std::unique_ptr<NetRequest> netRequest = std::make_unique<NetRequest>();
    if (netRequest == nullptr) {
        TELEPHONY_LOGE("Netrequest is null");
        return false;
    }
    netRequest->capability = request.capability;
    netRequest->ident = request.ident;
    AppExecFwk::InnerEvent::Pointer event =
        InnerEvent::Get(CellularDataEventCode::MSG_REQUEST_NETWORK, netRequest, TYPE_REQUEST_NET);
    return SendEvent(event);
}

int32_t CellularDataHandler::SetCellularDataEnable(bool userDataOn)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: SetCellularDataEnable dataSwitchSettings_ is null.", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    bool dataEnabled = false;
    dataSwitchSettings_->IsUserDataOn(dataEnabled);
    if (dataEnabled != userDataOn) {
        int32_t result = dataSwitchSettings_->SetUserDataOn(userDataOn);
        if (result != TELEPHONY_ERR_SUCCESS) {
            TELEPHONY_LOGE("Slot%{public}d: SetUserDataOn %{public}d fail.", slotId_, userDataOn);
            return result;
        }
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        if (userDataOn && defaultSlotId == slotId_) {
            EstablishAllApnsIfConnectable();
            if (apnManager_ == nullptr) {
                TELEPHONY_LOGE("Slot%{public}d: apnManager is null.", slotId_);
                return TELEPHONY_ERR_SUCCESS;
            }
            const int32_t id = DATA_CONTEXT_ROLE_DEFAULT_ID;
            sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
            if (apnHolder == nullptr) {
                TELEPHONY_LOGE("Slot%{public}d: apnHolder is null.", slotId_);
                return TELEPHONY_ERR_SUCCESS;
            }
            if (!apnHolder->IsDataCallEnabled()) {
                NetRequest netRequest;
                netRequest.ident =
                    std::string(IDENT_PREFIX) + std::to_string(CoreManagerInner::GetInstance().GetSimId(slotId_));
                netRequest.capability = NetCap::NET_CAPABILITY_INTERNET;
                apnHolder->RequestCellularData(netRequest);
                AttemptEstablishDataConnection(apnHolder);
                return TELEPHONY_ERR_SUCCESS;
            }
        } else {
            CellularDataHiSysEvent::WriteDataDeactiveBehaviorEvent(slotId_, DataDisconnectCause::BY_USER);
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        }
    } else {
        TELEPHONY_LOGI("Slot%{public}d: The status of the cellular data switch has not changed", slotId_);
    }
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataHandler::IsCellularDataEnabled(bool &dataEnabled) const
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: IsCellularDataEnabled dataSwitchSettings_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return dataSwitchSettings_->IsUserDataOn(dataEnabled);
}

int32_t CellularDataHandler::IsCellularDataRoamingEnabled(bool &dataRoamingEnabled) const
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: IsCellularDataRoamingEnabled dataSwitchSettings_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return dataSwitchSettings_->IsUserDataRoamingOn(dataRoamingEnabled);
}

int32_t CellularDataHandler::SetCellularDataRoamingEnabled(bool dataRoamingEnabled)
{
    if (dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ or apnManager_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    bool currentDataEnabled = false;
    dataSwitchSettings_->IsUserDataRoamingOn(currentDataEnabled);
    if (currentDataEnabled == dataRoamingEnabled) {
        TELEPHONY_LOGI("Slot%{public}d: The roaming switch status has not changed", slotId_);
        return TELEPHONY_ERR_SUCCESS;
    }
    int32_t result = dataSwitchSettings_->SetUserDataRoamingOn(dataRoamingEnabled);
    if (result != TELEPHONY_ERR_SUCCESS) {
        return result;
    }
    bool roamingState = CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0;
    if (roamingState) {
        ApnProfileState apnState = apnManager_->GetOverallApnState();
        if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
            apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
            ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
        }
        EstablishAllApnsIfConnectable();
    } else {
        TELEPHONY_LOGI("Slot%{public}d: Not roaming(%{public}d), not doing anything", slotId_, roamingState);
    }
    return TELEPHONY_ERR_SUCCESS;
}

void CellularDataHandler::ClearAllConnections(DisConnectionReason reason)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ClearAllConnections:apnManager is null", slotId_);
        return;
    }
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_CONNECTED ||
        currentState == ApnProfileState::PROFILE_STATE_CONNECTING) {
        int32_t networkType = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
        CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, networkType);
        StateNotification::GetInstance().UpdateCellularDataConnectState(
            slotId_, PROFILE_STATE_DISCONNECTING, networkType);
    }
    for (const sptr<ApnHolder> &apn : apnManager_->GetAllApnHolder()) {
        ClearConnection(apn, reason);
    }

    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: in ClearAllConnections connectionManager_ is null", slotId_);
        return;
    }
    connectionManager_->StopStallDetectionTimer();
    connectionManager_->EndNetStatistics();

    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: in ClearAllConnections dataSwitchSettings_ is null", slotId_);
        return;
    }
    bool dataEnabled = false;
    dataSwitchSettings_->IsUserDataOn(dataEnabled);
    if (!dataEnabled) {
        connectionManager_->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    }
}

void CellularDataHandler::ClearConnection(const sptr<ApnHolder> &apn, DisConnectionReason reason)
{
    if (apn == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ClearConnection:apnHolder is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = apn->GetCellularDataStateMachine();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: stateMachine is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: The APN holder is of type %{public}s", slotId_, apn->GetApnType().c_str());
    std::unique_ptr<DataDisconnectParams> object = std::make_unique<DataDisconnectParams>(apn->GetApnType(), reason);
    if (object == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ClearConnection fail, object is null", slotId_);
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
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    return apnManager_->GetOverallApnState();
}

ApnProfileState CellularDataHandler::GetCellularDataState(const std::string &apnType) const
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    sptr<ApnHolder> apnHolder = apnManager_->GetApnHolder(apnType);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    return apnHolder->GetApnState();
}

void CellularDataHandler::RadioPsConnectionAttached(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: ps attached", slotId_);
    if (event == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or apnManager_ is null", slotId_);
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING || apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RadioPsConnectionDetached(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: ps detached", slotId_);
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
}

void CellularDataHandler::RoamingStateOn(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: roaming on", slotId_);
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or dataSwitchSettings_ or apnManager_ is null", slotId_);
        return;
    }
    bool roamingState = false;
    if (CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0) {
        roamingState = true;
    }
    if (!roamingState) {
        TELEPHONY_LOGE("Slot%{public}d: device not currently roaming state", slotId_);
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING || apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RoamingStateOff(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: roaming off", slotId_);
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or dataSwitchSettings_ or apnManager_ is null", slotId_);
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING || apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::PsRadioEmergencyStateOpen(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: emergency on", slotId_);
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_CONNECTED ||
        currentState == ApnProfileState::PROFILE_STATE_CONNECTING) {
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::PsRadioEmergencyStateClose(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: emergency off", slotId_);
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_IDLE ||
        currentState == ApnProfileState::PROFILE_STATE_DISCONNECTING) {
        EstablishAllApnsIfConnectable();
    }
}

void CellularDataHandler::EstablishAllApnsIfConnectable()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: EstablishAllApnsIfConnectable:apnManager is null", slotId_);
        return;
    }
    const int32_t defSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    if (defSlotId != slotId_) {
        TELEPHONY_LOGI("Slot%{public}d: Establish all default:%{public}d", slotId_, defSlotId);
        return;
    }
    for (sptr<ApnHolder> apnHolder : apnManager_->GetSortApnHolder()) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: apn is null", slotId_);
            continue;
        }
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

bool CellularDataHandler::CheckCellularDataSlotId()
{
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    if (defSlotId != slotId_) {
        TELEPHONY_LOGI("Slot%{public}d: default:%{public}d, current:%{public}d", slotId_, defSlotId, slotId_);
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_CELLULAR_DATA_SLOT_ID_MISMATCH,
            "Default cellular data slot id is not current slot id");
        return false;
    }
    return true;
}

bool CellularDataHandler::CheckAttachAndSimState(sptr<ApnHolder> &apnHolder)
{
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return false;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    bool attached = coreInner.GetPsRegState(slotId_) == (int32_t)RegServiceState::REG_STATE_IN_SERVICE;
    SimState simState = SimState::SIM_STATE_UNKNOWN;
    coreInner.GetSimState(slotId_, simState);
    TELEPHONY_LOGI("Slot%{public}d: attached: %{public}d simState: %{public}d", slotId_, attached, simState);
    bool isEmergencyApn = apnHolder->IsEmergencyType();
    if (!isEmergencyApn && !attached) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_PS_NOT_ATTACH, "It is not emergencyApn and not attached");
        return false;
    }
    if (!isEmergencyApn && (simState != SimState::SIM_STATE_READY)) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_SIM_NOT_READY, "It is not emergencyApn and sim not ready");
        return false;
    }
    return true;
}

bool CellularDataHandler::CheckRoamingState(sptr<ApnHolder> &apnHolder)
{
    if (dataSwitchSettings_ == nullptr || apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ or apnManager_ is null", slotId_);
        return false;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    bool isEmergencyApn = apnHolder->IsEmergencyType();
    bool isAllowActiveData = dataSwitchSettings_->IsAllowActiveData();
    bool roamingState = coreInner.GetPsRoamingState(slotId_) > 0;
    bool dataRoamingEnabled = false;
    dataSwitchSettings_->IsUserDataRoamingOn(dataRoamingEnabled);
    if (roamingState && !dataRoamingEnabled) {
        isAllowActiveData = false;
    }
    if (isEmergencyApn) {
        isAllowActiveData = true;
    }
    if (!isAllowActiveData) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_ROAMING_SWITCH_OFF_AND_ROAMING, "Data roaming is not on and is roaming");
        TELEPHONY_LOGE("Slot%{public}d: AllowActiveData:%{public}d lastCallState_:%{public}d", slotId_,
            isAllowActiveData, lastCallState_);
        return false;
    }
    if (IsRestrictedMode()) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_CALL_AND_DATA_NOT_CONCURRENCY,
            "CS call and data are not allowed concurrency");
        TELEPHONY_LOGE("Slot%{public}d: AllowActiveData:%{public}d lastCallState_:%{public}d", slotId_,
            isAllowActiveData, lastCallState_);
        return false;
    }
    return true;
}

bool CellularDataHandler::CheckApnState(sptr<ApnHolder> &apnHolder)
{
    if (apnManager_ == nullptr || apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or apnManager_ is null", slotId_);
        return false;
    }

    if (apnHolder->GetApnState() == PROFILE_STATE_FAILED) {
        apnHolder->SetApnState(PROFILE_STATE_IDLE);
    }

    if (apnHolder->GetApnState() != PROFILE_STATE_IDLE) {
        TELEPHONY_LOGI("Slot%{public}d: APN holder is not idle", slotId_);
        return false;
    }
    std::vector<sptr<ApnItem>> matchedApns = apnManager_->FilterMatchedApns(apnHolder->GetApnType());
    if (matchedApns.empty()) {
        TELEPHONY_LOGE("Slot%{public}d: AttemptEstablishDataConnection:matchedApns is empty", slotId_);
        return false;
    }
    apnHolder->SetAllMatchedApns(matchedApns);
    return true;
}

void CellularDataHandler::AttemptEstablishDataConnection(sptr<ApnHolder> &apnHolder)
{
    if (!CheckCellularDataSlotId() || !CheckAttachAndSimState(apnHolder) || !CheckRoamingState(apnHolder)) {
        return;
    }
    DelayedSingleton<CellularDataHiSysEvent>::GetInstance()->SetCellularDataActivateStartTime();
    StartTrace(HITRACE_TAG_OHOS, "ActivateCellularData");
    if (!CheckApnState(apnHolder)) {
        FinishTrace(HITRACE_TAG_OHOS);
        return;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    coreInner.GetPsRadioTech(slotId_, radioTech);
    if (!EstablishDataConnection(apnHolder, radioTech)) {
        TELEPHONY_LOGE("Slot%{public}d: Establish data connection fail", slotId_);
    }
    FinishTrace(HITRACE_TAG_OHOS);
    DelayedSingleton<CellularDataHiSysEvent>::GetInstance()->JudgingDataActivateTimeOut(slotId_, SWITCH_ON);
}

std::shared_ptr<CellularDataStateMachine> CellularDataHandler::FindIdleCellularDataConnection() const
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: in FindIdleCellularDataConnection connectionManager_ is null", slotId_);
        return nullptr;
    }
    std::vector<std::shared_ptr<CellularDataStateMachine>> allMachines = connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &connect : allMachines) {
        if (connect == nullptr || apnManager_ == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: CellularDataHandler:stateMachine or apnManager_ is null", slotId_);
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
            TELEPHONY_LOGE("Slot%{public}d: failed to create EventRunner", slotId_);
            return nullptr;
        }
        stateMachineEventLoop_->Run();
    }
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine =
        std::make_shared<CellularDataStateMachine>(connectionManager_, shared_from_this(), stateMachineEventLoop_);
    if (cellularDataStateMachine == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataStateMachine is null", slotId_);
        return nullptr;
    }
    return cellularDataStateMachine;
}

bool CellularDataHandler::EstablishDataConnection(sptr<ApnHolder> &apnHolder, int32_t radioTech)
{
    sptr<ApnItem> apnItem = apnHolder->GetNextRetryApn();
    if (apnItem == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnItem is null", slotId_);
        return false;
    }
    int32_t profileId = apnItem->attr_.profileId_;
    profileId == INVALID_PROFILE_ID ? apnHolder->GetProfileId(apnHolder->GetApnType()) : profileId;
    if (!multipleConnectionsEnabled_) {
        if (HasAnyHigherPriorityConnection(apnHolder)) {
            TELEPHONY_LOGE("Slot%{public}d: has higher priority connection", slotId_);
            return false;
        }
        ApnProfileState apnState = apnManager_->GetOverallApnState();
        if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
            apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        }
    }
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine = FindIdleCellularDataConnection();
    if (cellularDataStateMachine == nullptr) {
        cellularDataStateMachine = CreateCellularDataConnect();
        if (cellularDataStateMachine == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: cellularDataStateMachine is null", slotId_);
            return false;
        }
        cellularDataStateMachine->Init();
        if (connectionManager_ == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: in EstablishDataConnection connectionManager_ is null", slotId_);
            return false;
        }
        connectionManager_->AddConnectionStateMachine(cellularDataStateMachine);
    }
    cellularDataStateMachine->SetCapability(apnHolder->GetCapability());
    apnHolder->SetCurrentApn(apnItem);
    apnHolder->SetApnState(PROFILE_STATE_CONNECTING);
    apnHolder->SetCellularDataStateMachine(cellularDataStateMachine);
    bool roamingState = CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0;
    bool userDataRoaming = false;
    dataSwitchSettings_->IsUserDataRoamingOn(userDataRoaming);
    StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_CONNECTING, radioTech);
    std::unique_ptr<DataConnectionParams> object =
        std::make_unique<DataConnectionParams>(apnHolder, profileId, radioTech, roamingState, userDataRoaming, true);
    if (object == nullptr) {
        TELEPHONY_LOGE("DataConnectionParams is nullptr");
        return false;
    }
    TELEPHONY_LOGI("Slot%{public}d: MSG_SM_CONNECT profileId:%{public}d type:%{public}s networkType:%{public}d",
        slotId_, profileId, apnHolder->GetApnType().c_str(), radioTech);
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, object);
    cellularDataStateMachine->SendEvent(event);
    return true;
}

void CellularDataHandler::EstablishDataConnectionComplete(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    std::shared_ptr<SetupDataCallResultInfo> resultInfo = event->GetSharedObject<SetupDataCallResultInfo>();
    if ((resultInfo != nullptr) && (apnManager_ != nullptr)) {
        sptr<ApnHolder> apnHolder = apnManager_->GetApnHolder(apnManager_->FindApnNameByApnId(resultInfo->flag));
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: flag:%{public}d complete apnHolder is null", slotId_, resultInfo->flag);
            return;
        }
        apnHolder->SetApnState(PROFILE_STATE_CONNECTED);
        apnHolder->InitialApnRetryCount();
        int32_t networkType = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
        CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, networkType);
        StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_CONNECTED, networkType);
        std::shared_ptr<CellularDataStateMachine> stateMachine = apnHolder->GetCellularDataStateMachine();
        if (stateMachine != nullptr) {
            stateMachine->UpdateNetworkInfo(*resultInfo);
        } else {
            TELEPHONY_LOGE(
                "Slot%{public}d:update network info stateMachine(%{public}d) is null", slotId_, resultInfo->flag);
        }
        if (connectionManager_ != nullptr) {
            connectionManager_->StartStallDetectionTimer();
            connectionManager_->BeginNetStatistics();
        }
        if (!physicalConnectionActiveState_) {
            physicalConnectionActiveState_ = true;
            CoreManagerInner::GetInstance().DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
        }
    }
}

int32_t CellularDataHandler::GetSlotId() const
{
    return slotId_;
}

void CellularDataHandler::DisconnectDataComplete(const InnerEvent::Pointer &event)
{
    if ((event == nullptr) || (apnManager_ == nullptr)) {
        TELEPHONY_LOGE("Slot%{public}d: event or apnManager is null", slotId_);
        return;
    }
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: in DisconnectDataComplete connectionManager_ is null", slotId_);
        return;
    }
    std::unique_ptr<DataDisconnectParams> object = event->GetUniqueObject<DataDisconnectParams>();
    int32_t apnId = apnManager_->FindApnIdByApnName(object->GetApnType());
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(apnId);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null, apnId is %{public}d", slotId_, apnId);
        return;
    }
    DisConnectionReason reason = object->GetReason();
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    int32_t networkType = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, networkType);
    StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, PROFILE_STATE_IDLE, networkType);
    TELEPHONY_LOGI("Slot%{public}d: apn type: %{public}s call:%{public}d", slotId_, apnHolder->GetApnType().c_str(),
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
            TELEPHONY_LOGI("Slot%{public}d: Establish a data connection. The apn type is %{public}s", slotId_,
                apnHolder->GetApnType().c_str());
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
                TELEPHONY_LOGI("Slot%{public}d: HandleSortConnection the apn type is %{public}s", slotId_,
                    sortApnHolder->GetApnType().c_str());
                SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, apnId, delayTime);
                break;
            }
        }
    }
}

void CellularDataHandler::MsgEstablishDataConnection(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr || event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or event is null", slotId_);
        return;
    }
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(event->GetParam());
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: Establish a data connection. APN holder type:%{public}s call:%{public}d", slotId_,
        apnHolder->GetApnType().c_str(), apnHolder->IsDataCallEnabled());
    if (apnHolder->IsDataCallEnabled()) {
        AttemptEstablishDataConnection(apnHolder);
    } else {
        DisConnectionReason reason = DisConnectionReason::REASON_CHANGE_CONNECTION;
        if (multipleConnectionsEnabled_) {
            reason = DisConnectionReason::REASON_CLEAR_CONNECTION;
        }
        ClearConnection(apnHolder, reason);
    }
}

void CellularDataHandler::MsgRequestNetwork(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr || event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or event is null", slotId_);
        return;
    }
    std::unique_ptr<NetRequest> netRequest = event->GetUniqueObject<NetRequest>();
    if (netRequest == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: netRequest is null", slotId_);
        return;
    }
    NetRequest request;
    request.ident = netRequest->ident;
    request.capability = netRequest->capability;
    int32_t id = ApnManager::FindApnIdByCapability(request.capability);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null.", slotId_);
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
        TELEPHONY_LOGE("Slot%{public}d: send data connection event failed", slotId_);
    }
}

void CellularDataHandler::ProcessEvent(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: start ProcessEvent but event is null!", slotId_);
        return;
    }
    uint32_t eventCode = event->GetInnerEventId();
    std::map<uint32_t, Fun>::iterator it = eventIdMap_.find(eventCode);
    if (it != eventIdMap_.end()) {
        (this->*(it->second))(event);
    }
}

void CellularDataHandler::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    if (EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED == action) {
        int32_t slotId = want.GetIntParam("slotId", 0);
        if (slotId_ != slotId) {
            return;
        }
        int32_t state = want.GetIntParam("state", CALL_STATUS_UNKNOWN);
        if (state == CALL_STATUS_UNKNOWN) {
            TELEPHONY_LOGE("Slot%{public}d: unknown call state=%{public}d", slotId, state);
            return;
        }
        HandleVoiceCallChanged(state);
    } else {
        TELEPHONY_LOGI("Slot%{public}d: action=%{public}s code=%{public}d", slotId_, action.c_str(), data.GetCode());
    }
}

void CellularDataHandler::HandleSettingSwitchChanged(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    bool setting_switch = event->GetParam();
    TELEPHONY_LOGI("Slot%{public}d: setting switch = %{public}d", slotId_, setting_switch);
}

void CellularDataHandler::HandleVoiceCallChanged(int32_t state)
{
    if (apnManager_ == nullptr || connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager or connectionManager is null!", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: handle voice call changed lastState:%{public}d, state:%{public}d", slotId_,
        lastCallState_, state);
    // next to check if radio technology support voice and data at same time.
    int32_t psRadioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, psRadioTech);
    bool support = (psRadioTech == static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_GSM));
    if (lastCallState_ != state) {
        // call is busy
        lastCallState_ = state;
        if (state != TelCallStatus::CALL_STATUS_IDLE && state != TelCallStatus::CALL_STATUS_DISCONNECTED) {
            if (apnManager_->HasAnyConnectedState() && support) {
                connectionManager_->EndNetStatistics();
                connectionManager_->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
                connectionManager_->StopStallDetectionTimer();
                disconnectionReason_ = DisConnectionReason::REASON_GSM_AND_CALLING_ONLY;
            }
        } else {
            if (apnManager_->HasAnyConnectedState() && support) {
                connectionManager_->StartStallDetectionTimer();
                connectionManager_->BeginNetStatistics();
            }
            disconnectionReason_ = DisConnectionReason::REASON_NORMAL;
            TELEPHONY_LOGI("Slot%{public}d: HandleVoiceCallChanged EstablishAllApnsIfConnectable", slotId_);
            EstablishAllApnsIfConnectable();
        }
        TELEPHONY_LOGI("Slot%{public}d: disconnectionReason_=%{public}d", slotId_, disconnectionReason_);
    }
}

void CellularDataHandler::HandleSimStateOrRecordsChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    std::u16string iccId;
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case RadioEvent::RADIO_SIM_STATE_CHANGE: {
            SimState simState = SimState::SIM_STATE_UNKNOWN;
            CoreManagerInner::GetInstance().GetSimState(slotId_, simState);
            TELEPHONY_LOGI("Slot%{public}d: sim state is :%{public}d", slotId_, simState);
            if (simState != SimState::SIM_STATE_READY) {
                ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
                if (simState == SimState::SIM_STATE_NOT_PRESENT) {
                    UnRegisterDataSettingObserver();
                }
            } else {
                dataSwitchSettings_->LoadSwitchValue();
                CoreManagerInner::GetInstance().GetSimIccId(slotId_, iccId);
                if (lastIccId_ != u"" && lastIccId_ == iccId) {
                    EstablishAllApnsIfConnectable();
                }
            }
            break;
        }
        case RadioEvent::RADIO_SIM_RECORDS_LOADED: {
            CoreManagerInner::GetInstance().GetSimIccId(slotId_, iccId);
            SimState simState = SimState::SIM_STATE_UNKNOWN;
            CoreManagerInner::GetInstance().GetSimState(slotId_, simState);
            TELEPHONY_LOGI("Slot%{public}d: sim records loaded state is :%{public}d", slotId_, simState);
            if (simState == SimState::SIM_STATE_READY && iccId != u"") {
                if (iccId != lastIccId_) {
                    dataSwitchSettings_->SetPolicyDataOn(true);
                    GetConfigurationFor5G();
                    lastIccId_ = iccId;
                    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_APN_CHANGED);
                    SendEvent(event);
                } else if (lastIccId_ == iccId) {
                    TELEPHONY_LOGI("Slot%{public}d: sim state changed, but iccId not changed.", slotId_);
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

void CellularDataHandler::HandleSimAccountLoaded(const InnerEvent::Pointer &event)
{
    if (event == nullptr || dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or dataSwitchSettings_ is null", slotId_);
        return;
    }
    CellularDataNetAgent::GetInstance().RegisterNetSupplier(slotId_);
    if (slotId_ == 0) {
        CellularDataNetAgent::GetInstance().RegisterPolicyCallback();
    }
    RegisterDataSettingObserver();
    dataSwitchSettings_->LoadSwitchValue();
    int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    TELEPHONY_LOGI("Slot%{public}d: HandleSimAccountLoaded defaultSlotId is: %{public}d", slotId_, defaultSlotId);
    if (slotId_ == defaultSlotId) {
        SetDataPermitted(true);
        EstablishAllApnsIfConnectable();
    } else {
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        SetDataPermitted(false);
    }
}

bool CellularDataHandler::HandleApnChanged()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager is null");
        return false;
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        TELEPHONY_LOGI("Slot%{public}d: apn type:%{public}s state:%{public}d", slotId_, apnHolder->GetApnType().c_str(),
            apnHolder->GetApnState());
    }
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_APN_CHANGED);
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: get apn changed event is null", slotId_);
        return false;
    }
    return SendEvent(event);
}

void CellularDataHandler::HandleApnChanged(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ is null", slotId_);
        return;
    }
    std::u16string operatorNumeric;
    CoreManagerInner::GetInstance().GetSimOperatorNumeric(slotId_, operatorNumeric);
    std::string numeric = Str16ToStr8(operatorNumeric);
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
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING || apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
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
        TELEPHONY_LOGE("Slot%{public}d: connection manager is null", slotId_);
        return 0;
    }
    return connectionManager_->GetDataFlowType();
}

void CellularDataHandler::HandleRadioStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr || event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: radio off or not available apnManager or event is null!", slotId_);
        return;
    }
    std::shared_ptr<HRilInt32Parcel> object = event->GetSharedObject<HRilInt32Parcel>();
    if (object == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: HandleRadioStateChanged object is nullptr!", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: Radio changed with state: %{public}d", slotId_, object->data);
    switch (object->data) {
        case CORE_SERVICE_POWER_OFF:
        case CORE_SERVICE_POWER_NOT_AVAILABLE: {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            TELEPHONY_LOGI("Slot%{public}d: apn state is %{public}d", slotId_, apnState);
            if (apnState != ApnProfileState::PROFILE_STATE_IDLE) {
                ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
            }
            break;
        }
        case CORE_SERVICE_POWER_ON:
            SyncDataPermitted();
            SetRilLinkBandwidths();
            EstablishAllApnsIfConnectable();
            break;
        default:
            TELEPHONY_LOGI("Slot%{public}d: un-handle state:%{public}d", slotId_, object->data);
            break;
    }
}

void CellularDataHandler::PsDataRatChanged(const InnerEvent::Pointer &event)
{
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    coreInner.GetPsRadioTech(slotId_, radioTech);
    TELEPHONY_LOGI("Slot%{public}d: radioTech is %{public}d", slotId_, radioTech);
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    bool dataEnabled = false;
    IsCellularDataEnabled(dataEnabled);
    if (!dataEnabled) {
        TELEPHONY_LOGE("Slot%{public}d: data enable is close", slotId_);
        return;
    }
    bool attached = coreInner.GetPsRegState(slotId_) == (int32_t)RegServiceState::REG_STATE_IN_SERVICE;
    if (!attached) {
        TELEPHONY_LOGE("Slot%{public}d: attached is false", slotId_);
        return;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING || apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::SetPolicyDataOn(bool enable)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings is null", slotId_);
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
    int32_t networkType = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, networkType);
    bool support = (networkType == (int32_t)RadioTech::RADIO_TECHNOLOGY_GSM);
    bool inCall = (lastCallState_ != TelCallStatus::CALL_STATUS_IDLE &&
                   lastCallState_ != TelCallStatus::CALL_STATUS_DISCONNECTED);
    TELEPHONY_LOGI("Slot%{public}d: radio technology is gsm only:%{public}d and call is busy:%{public}d", slotId_,
        support, inCall);
    return inCall && support;
}

DisConnectionReason CellularDataHandler::GetDisConnectionReason()
{
    return disconnectionReason_;
}

void CellularDataHandler::SyncDataPermitted()
{
    const int32_t defSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    if (defSlotId < 0) {
        TELEPHONY_LOGE("Slot%{public}d: defSlotId is %{public}d.", slotId_, defSlotId);
        return;
    }
    if (defSlotId == slotId_) {
        SetDataPermitted(true);
    } else {
        SetDataPermitted(false);
    }
}

void CellularDataHandler::SetDataPermitted(bool dataPermitted)
{
    TELEPHONY_LOGI("Slot%{public}d: dataPermitted is %{public}d.", slotId_, dataPermitted);
    int32_t maxSimCount = CoreManagerInner::GetInstance().GetMaxSimCount();
    if (maxSimCount <= 1) {
        TELEPHONY_LOGE("Slot%{public}d: maxSimCount is: %{public}d", slotId_, maxSimCount);
        return;
    }
    CoreManagerInner::GetInstance().SetDataPermitted(
        slotId_, CellularDataEventCode::MSG_SET_DATA_PERMITTED, dataPermitted, shared_from_this());
}

void CellularDataHandler::SetDataPermittedResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SetDataPermittedResponse:Slot%{public}d: event is null", slotId_);
        return;
    }
    std::shared_ptr<TelRilResponseInfo<int32_t>> rilInfo = event->GetSharedObject<TelRilResponseInfo<int32_t>>();
    if (rilInfo == nullptr) {
        TELEPHONY_LOGE("SetDataPermittedResponse:Slot%{public}d: HRilRadioResponseInfo is null", slotId_);
        return;
    }
    if (rilInfo->errorNo != 0) {
        TELEPHONY_LOGE("Slot%{public}d: SetDataPermitted error", slotId_);
    }
}

void CellularDataHandler::SetRilAttachApn()
{
    sptr<ApnItem> attachApn = apnManager_->GetRilAttachApn();
    if (attachApn == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: attachApn is null", slotId_);
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
    CoreManagerInner::GetInstance().SetInitApnInfo(
        slotId_, CellularDataEventCode::MSG_SET_RIL_ATTACH_APN, dataProfile, shared_from_this());
}

void CellularDataHandler::SetRilAttachApnResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SetRilAttachApnResponse:Slot%{public}d: event is null", slotId_);
        return;
    }
    std::shared_ptr<TelRilResponseInfo<int32_t>> rilInfo = event->GetSharedObject<TelRilResponseInfo<int32_t>>();
    if (rilInfo == nullptr) {
        TELEPHONY_LOGE("SetRilAttachApnResponse:Slot%{public}d: HRilRadioResponseInfo is null", slotId_);
        return;
    }
    if (rilInfo->errorNo != 0) {
        TELEPHONY_LOGE("Slot%{public}d: SetRilAttachApn error", slotId_);
    }
}

bool CellularDataHandler::HasAnyHigherPriorityConnection(const sptr<ApnHolder> &apnHolder)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return false;
    }
    std::vector<sptr<ApnHolder>> sortApnHolders = apnManager_->GetSortApnHolder();
    if (sortApnHolders.empty()) {
        TELEPHONY_LOGE("Slot%{public}d: SortApnHolder is null", slotId_);
        return false;
    }
    for (const sptr<ApnHolder> &sortApnHolder : sortApnHolders) {
        if (sortApnHolder->GetPriority() > apnHolder->GetPriority()) {
            if (sortApnHolder->IsDataCallEnabled() &&
                (sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTED ||
                    sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTING ||
                    sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_DISCONNECTING)) {
                CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
                    CellularDataErrorCode::DATA_ERROR_HAS_HIGHER_PRIORITY_CONNECTION,
                    "There is higher priority connection");
                return true;
            }
        }
    }
    return false;
}

bool CellularDataHandler::HasInternetCapability(const int32_t cid) const
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> activeStateMachine = connectionManager_->GetActiveConnectionByCid(cid);
    if (activeStateMachine == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: get activeStateMachine by cid fail", slotId_);
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
        TELEPHONY_LOGI("Slot%{public}d: parse operator 5G config: %{public}s", slotId_, flag.c_str());
        if (flag == "true") {
            return true;
        }
    }
    return false;
}

void CellularDataHandler::GetDefaultConfiguration()
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return;
    }
    connectionManager_->GetDefaultBandWidthsConfig();
    connectionManager_->GetDefaultTcpBufferConfig();
    GetDefaultUpLinkThresholdsConfig();
    GetDefaultDownLinkThresholdsConfig();
    defaultMobileMtuConfig_ = CellularDataUtils::GetDefaultMobileMtuConfig();
    TELEPHONY_LOGI("Slot%{public}d: defaultMobileMtuConfig_ = %{public}d", slotId_, defaultMobileMtuConfig_);
    defaultPreferApn_ = CellularDataUtils::GetDefaultPreferApnConfig();
    TELEPHONY_LOGI("Slot%{public}d: defaultPreferApn_ is %{public}d", slotId_, defaultPreferApn_);
    multipleConnectionsEnabled_ = CellularDataUtils::GetDefaultMultipleConnectionsConfig();
    TELEPHONY_LOGI("Slot%{public}d: multipleConnectionsEnabled_ = %{public}d", slotId_, multipleConnectionsEnabled_);
}

void CellularDataHandler::HandleRadioNrStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: receive HandleRadioNrStateChanged event", slotId_);
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(RadioEvent::RADIO_NR_STATE_CHANGED);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::HandleRadioNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: receive HandleRadioNrFrequencyChanged event", slotId_);
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(RadioEvent::RADIO_NR_FREQUENCY_CHANGED);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::GetDefaultUpLinkThresholdsConfig()
{
    upLinkThresholds_.clear();
    char upLinkConfig[UP_DOWN_LINK_SIZE] = { 0 };
    GetParameter(CONFIG_UPLINK_THRESHOLDS, CAPACITY_THRESHOLDS_FOR_UPLINK, upLinkConfig, UP_DOWN_LINK_SIZE);
    TELEPHONY_LOGI("Slot%{public}d: upLinkThresholds = %{public}s", slotId_, upLinkConfig);
    upLinkThresholds_ = CellularDataUtils::Split(upLinkConfig, ",");
}

void CellularDataHandler::GetDefaultDownLinkThresholdsConfig()
{
    downLinkThresholds_.clear();
    char downLinkConfig[UP_DOWN_LINK_SIZE] = { 0 };
    GetParameter(CONFIG_DOWNLINK_THRESHOLDS, CAPACITY_THRESHOLDS_FOR_DOWNLINK, downLinkConfig, UP_DOWN_LINK_SIZE);
    TELEPHONY_LOGI("Slot%{public}d: downLinkThresholds_ = %{public}s", slotId_, downLinkConfig);
    downLinkThresholds_ = CellularDataUtils::Split(downLinkConfig, ",");
}

void CellularDataHandler::SetRilLinkBandwidths()
{
    LinkBandwidthRule linkBandwidth;
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, linkBandwidth.rat);
    linkBandwidth.delayMs = DELAY_SET_RIL_BANDWIDTH_MS;
    linkBandwidth.delayUplinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    linkBandwidth.delayDownlinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    for (std::string upLinkThreshold : upLinkThresholds_) {
        linkBandwidth.maximumUplinkKbps.push_back(atoi(upLinkThreshold.c_str()));
    }
    for (std::string downLinkThreshold : downLinkThresholds_) {
        linkBandwidth.maximumDownlinkKbps.push_back(atoi(downLinkThreshold.c_str()));
    }
    CoreManagerInner::GetInstance().SetLinkBandwidthReportingRule(
        slotId_, CellularDataEventCode::MSG_SET_RIL_BANDWIDTH, linkBandwidth, shared_from_this());
}

void CellularDataHandler::HandleDBSettingEnableChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: HandleDBSettingEnableChanged dataSwitchSettings_ is null.", slotId_);
        return;
    }
    int64_t value = event->GetParam();
    bool dataEnabled = false;
    dataSwitchSettings_->IsUserDataOn(dataEnabled);
    if (dataEnabled != value) {
        dataSwitchSettings_->SetUserDataOn(value);
        if (value) {
            EstablishAllApnsIfConnectable();
            if (apnManager_ == nullptr) {
                TELEPHONY_LOGE("Slot%{public}d: apnManager is null.", slotId_);
                return;
            }
            const int32_t id = DATA_CONTEXT_ROLE_DEFAULT_ID;
            sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
            if (apnHolder == nullptr) {
                TELEPHONY_LOGE("Slot%{public}d: apnHolder is null.", slotId_);
                return;
            }
            if (!apnHolder->IsDataCallEnabled()) {
                NetRequest netRequest;
                netRequest.ident =
                    std::string(IDENT_PREFIX) + std::to_string(CoreManagerInner::GetInstance().GetSimId(slotId_));
                netRequest.capability = NetCap::NET_CAPABILITY_INTERNET;
                apnHolder->RequestCellularData(netRequest);
                AttemptEstablishDataConnection(apnHolder);
                return;
            }
        } else {
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        }
    } else {
        TELEPHONY_LOGI("Slot%{public}d: The status of the cellular data switch has not changed", slotId_);
    }
}

void CellularDataHandler::HandleDBSettingRoamingChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    if (dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ or apnManager_ is null", slotId_);
        return;
    }
    int64_t value = event->GetParam();
    bool dataRoamingEnabled = false;
    dataSwitchSettings_->IsUserDataRoamingOn(dataRoamingEnabled);
    if (dataRoamingEnabled != value) {
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
            TELEPHONY_LOGI("Slot%{public}d: Not roaming(%{public}d), not doing anything", slotId_, roamingState);
        }
    } else {
        TELEPHONY_LOGI("Slot%{public}d: The roaming switch status has not changed", slotId_);
    }
}

void CellularDataHandler::UnRegisterDataSettingObserver()
{
    if (settingObserver_ == nullptr || roamingObserver_ == nullptr) {
        TELEPHONY_LOGE(
            "UnRegisterDataSettingObserver:Slot%{public}d: settingObserver_ or roamingObserver_ is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("UnRegisterDataSettingObserver:Slot%{public}d: settingHelper is null", slotId_);
        return;
    }
    Uri dataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    settingHelper->UnRegisterSettingsObserver(dataEnableUri, settingObserver_);

    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE(
            "UnRegisterDataSettingObserver:Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return;
    }
    Uri dataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    settingHelper->UnRegisterSettingsObserver(dataRoamingUri, roamingObserver_);
}

void CellularDataHandler::RegisterDataSettingObserver()
{
    if (settingObserver_ == nullptr || roamingObserver_ == nullptr) {
        TELEPHONY_LOGE(
            "RegisterDataSettingObserver:Slot%{public}d: settingObserver_ or roamingObserver_ is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("RegisterDataSettingObserver:Slot%{public}d: settingHelper is null", slotId_);
        return;
    }
    Uri dataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    settingHelper->RegisterSettingsObserver(dataEnableUri, settingObserver_);

    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE(
            "RegisterDataSettingObserver:Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return;
    }
    Uri dataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    settingHelper->RegisterSettingsObserver(dataRoamingUri, roamingObserver_);
}
} // namespace Telephony
} // namespace OHOS
