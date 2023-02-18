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

#include "cellular_data_controller.h"

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "core_manager_inner.h"
#include "network_search_callback.h"
#include "radio_event.h"
#include "telephony_log_wrapper.h"
#include "uri.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;
using namespace OHOS::EventFwk;

CellularDataController::CellularDataController(std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : AppExecFwk::EventHandler(runner), slotId_(slotId)
{}

CellularDataController::~CellularDataController()
{
    UnRegisterEvents();
    UnRegisterDatabaseObserver();
    if (netManagerListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy != nullptr) {
            samgrProxy->UnSubscribeSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, netManagerListener_);
            samgrProxy->UnSubscribeSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, netManagerListener_);
            netManagerListener_ = nullptr;
        }
    }
}

void CellularDataController::Init()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    cellularDataHandler_ = std::make_shared<CellularDataHandler>(GetEventRunner(), subscriberInfo, slotId_);
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return;
    }
    cellularDataHandler_->Init();
    RegisterDatabaseObserver();
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        TELEPHONY_LOGE("samgrProxy is nullptr");
        return;
    }
    netManagerListener_ = new (std::nothrow) SystemAbilityStatusChangeListener(slotId_, cellularDataHandler_);
    if (netManagerListener_ == nullptr) {
        TELEPHONY_LOGE("netManagerListener_ is nullptr");
        return;
    }
    samgrProxy->SubscribeSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, netManagerListener_);
    samgrProxy->SubscribeSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, netManagerListener_);
}

int32_t CellularDataController::SetCellularDataEnable(bool userDataEnabled)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: SetCellularDataEnable cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->SetCellularDataEnable(userDataEnabled);
}

int32_t CellularDataController::IsCellularDataEnabled(bool &dataEnabled) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: IsCellularDataEnabled cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->IsCellularDataEnabled(dataEnabled);
}

ApnProfileState CellularDataController::GetCellularDataState() const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: GetCellularDataState cellularDataHandler_ is null", slotId_);
        return ApnProfileState::PROFILE_STATE_FAILED;
    }
    return cellularDataHandler_->GetCellularDataState();
}

ApnProfileState CellularDataController::GetCellularDataState(const std::string &apnType) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: GetCellularDataState cellularDataHandler_ is null", slotId_);
        return ApnProfileState::PROFILE_STATE_FAILED;
    }
    return cellularDataHandler_->GetCellularDataState(apnType);
}

int32_t CellularDataController::IsCellularDataRoamingEnabled(bool &dataRoamingEnabled) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: IsCellularDataRoamingEnabled cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->IsCellularDataRoamingEnabled(dataRoamingEnabled);
}

int32_t CellularDataController::SetCellularDataRoamingEnabled(bool dataRoamingEnabled)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: SetCellularDataRoamingEnabled cellularDataHandler is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->SetCellularDataRoamingEnabled(dataRoamingEnabled);
}

void CellularDataController::SetDataPermitted(bool dataPermitted) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: SetDataPermitted cellularDataHandler is null", slotId_);
        return;
    }
    cellularDataHandler_->SetDataPermitted(dataPermitted);
}

bool CellularDataController::ReleaseNet(const NetRequest &request)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ReleaseNet cellularDataHandler_ is null", slotId_);
        return false;
    }
    return cellularDataHandler_->ReleaseNet(request);
}

bool CellularDataController::RequestNet(const NetRequest &request)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: RequestNet cellularDataHandler_ is null", slotId_);
        return false;
    }
    return cellularDataHandler_->RequestNet(request);
}

void CellularDataController::AsynchronousRegister()
{
    if (CoreManagerInner::GetInstance().IsInitFinished()) {
        TELEPHONY_LOGI("Slot%{public}d: core inited", slotId_);
        Init();
        RegisterEvents();
        return;
    }
    SendEvent(CellularDataEventCode::MSG_ASYNCHRONOUS_REGISTER_EVENT_ID, CORE_INIT_DELAY_TIME, Priority::HIGH);
}

void CellularDataController::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ProcessEvent event is null.", slotId_);
        return;
    }
    size_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case CellularDataEventCode::MSG_ASYNCHRONOUS_REGISTER_EVENT_ID:
            AsynchronousRegister();
            break;
        default:
            TELEPHONY_LOGE("Slot%{public}d: ProcessEvent nothing to do", slotId_);
            break;
    }
}

void CellularDataController::RegisterEvents()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: core is null or cellularDataHandler is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: RegisterEvents start", slotId_);
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_STATE_CHANGE, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_RECORDS_LOADED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_ACCOUNT_LOADED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_ATTACHED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_DETACHED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_OPEN, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_CLOSE, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_STATE_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_RAT_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_CALL_STATUS_INFO, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_OPEN, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_CLOSE, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_STATE_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_FREQUENCY_CHANGED, nullptr);
    if (slotId_ == 0) {
        sptr<NetworkSearchCallback> networkSearchCallback = std::make_unique<NetworkSearchCallback>().release();
        if (networkSearchCallback != nullptr) {
            coreInner.RegisterCellularDataObject(networkSearchCallback);
        } else {
            TELEPHONY_LOGE("Slot%{public}d: networkSearchCallback is null", slotId_);
        }
    }
}

void CellularDataController::UnRegisterEvents()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: UnRegisterEvents cellularDataHandler_ is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: UnRegisterEvents start", slotId_);
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_STATE_CHANGE);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_RECORDS_LOADED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_ACCOUNT_LOADED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_ATTACHED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_DETACHED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_OPEN);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_CLOSE);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_STATE_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_RAT_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_CALL_STATUS_INFO);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_OPEN);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_CLOSE);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_STATE_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_FREQUENCY_CHANGED);
    TELEPHONY_LOGI("Slot%{public}d: UnRegisterEvents end", slotId_);
}

void CellularDataController::UnRegisterDatabaseObserver()
{
    if (cellularDataRdbObserver_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataRdbObserver_ is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataRdbHelper> cellularDataDbHelper = CellularDataRdbHelper::GetInstance();
    cellularDataDbHelper->UnRegisterObserver(cellularDataRdbObserver_);
}

void CellularDataController::RegisterDatabaseObserver()
{
    cellularDataRdbObserver_ = std::make_unique<CellularDataRdbObserver>(cellularDataHandler_).release();
    if (cellularDataRdbObserver_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataRdbObserver_ is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataRdbHelper> cellularDataDbHelper = CellularDataRdbHelper::GetInstance();
    cellularDataDbHelper->RegisterObserver(cellularDataRdbObserver_);
}

bool CellularDataController::HandleApnChanged()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ApnChanged cellularDataHandler_ is null", slotId_);
        return static_cast<int32_t>(DataRespondCode::SET_FAILED);
    }
    return cellularDataHandler_->HandleApnChanged();
}

int32_t CellularDataController::GetCellularDataFlowType()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellular data handler is null", slotId_);
        return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    }
    return cellularDataHandler_->GetCellularDataFlowType();
}

void CellularDataController::EstablishDataConnection()
{
    if (cellularDataHandler_ != nullptr) {
        cellularDataHandler_->EstablishAllApnsIfConnectable();
    }
}

int32_t CellularDataController::SetPolicyDataOn(bool enable)
{
    if (cellularDataHandler_ != nullptr) {
        cellularDataHandler_->SetPolicyDataOn(enable);
    }
    return static_cast<int32_t>(DataRespondCode::SET_SUCCESS);
}

bool CellularDataController::IsRestrictedMode() const
{
    if (cellularDataHandler_ != nullptr) {
        return cellularDataHandler_->IsRestrictedMode();
    }
    return false;
}

DisConnectionReason CellularDataController::GetDisConnectionReason()
{
    if (cellularDataHandler_ != nullptr) {
        return cellularDataHandler_->GetDisConnectionReason();
    }
    return DisConnectionReason::REASON_NORMAL;
}

bool CellularDataController::HasInternetCapability(const int32_t cid) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return false;
    }
    return cellularDataHandler_->HasInternetCapability(cid);
}

bool CellularDataController::ClearAllConnections(DisConnectionReason reason) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler is null", slotId_);
        return false;
    }
    cellularDataHandler_->ClearAllConnections(reason);
    return true;
}

CellularDataController::SystemAbilityStatusChangeListener::SystemAbilityStatusChangeListener(
    int32_t slotId, std::shared_ptr<CellularDataHandler> handler)
    : slotId_(slotId), handler_(handler)
{}

void CellularDataController::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    switch (systemAbilityId) {
        case COMM_NET_CONN_MANAGER_SYS_ABILITY_ID:
            TELEPHONY_LOGI("COMM_NET_CONN_MANAGER_SYS_ABILITY_ID running");
            if (isNetStopped_ && handler_ != nullptr) {
                handler_->ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
                CellularDataNetAgent::GetInstance().RegisterNetSupplier(slotId_);
                handler_->EstablishAllApnsIfConnectable();
                isNetStopped_ = false;
            } else {
                CellularDataNetAgent::GetInstance().RegisterNetSupplier(slotId_);
            }
            break;
        case COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID:
            TELEPHONY_LOGI("COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID running");
            if (slotId_ == 0) {
                CellularDataNetAgent::GetInstance().RegisterPolicyCallback();
            }
            break;
        default:
            TELEPHONY_LOGE("systemAbilityId is invalid");
            break;
    }
}

void CellularDataController::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    switch (systemAbilityId) {
        case COMM_NET_CONN_MANAGER_SYS_ABILITY_ID:
            TELEPHONY_LOGE("COMM_NET_CONN_MANAGER_SYS_ABILITY_ID stopped");
            isNetStopped_ = true;
            break;
        case COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID:
            TELEPHONY_LOGE("COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID stopped");
            break;
        default:
            TELEPHONY_LOGE("systemAbilityId is invalid");
            break;
    }
}
} // namespace Telephony
} // namespace OHOS
