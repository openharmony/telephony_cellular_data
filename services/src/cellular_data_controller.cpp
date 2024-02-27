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

CellularDataController::CellularDataController(int32_t slotId)
    : TelEventHandler("CellularDataController"), slotId_(slotId)
{}

CellularDataController::~CellularDataController()
{
    UnRegisterEvents();
    if (systemAbilityListener_ != nullptr) {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy != nullptr) {
            samgrProxy->UnSubscribeSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, systemAbilityListener_);
            samgrProxy->UnSubscribeSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, systemAbilityListener_);
            samgrProxy->UnSubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, systemAbilityListener_);
            samgrProxy->UnSubscribeSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, systemAbilityListener_);
            systemAbilityListener_ = nullptr;
        }
    }
}

void CellularDataController::Init()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_OPERATOR_CONFIG_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    cellularDataHandler_ = std::make_shared<CellularDataHandler>(subscriberInfo, slotId_);
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return;
    }
    cellularDataHandler_->Init();
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        TELEPHONY_LOGE("samgrProxy is nullptr");
        return;
    }
    systemAbilityListener_ = new (std::nothrow) SystemAbilityStatusChangeListener(slotId_, cellularDataHandler_);
    if (systemAbilityListener_ == nullptr) {
        TELEPHONY_LOGE("systemAbilityListener_ is nullptr");
        return;
    }
    samgrProxy->SubscribeSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, systemAbilityListener_);
    samgrProxy->SubscribeSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, systemAbilityListener_);
    samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, systemAbilityListener_);
    samgrProxy->SubscribeSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, systemAbilityListener_);
}

int32_t CellularDataController::SetCellularDataEnable(bool userDataEnabled)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->SetCellularDataEnable(userDataEnabled);
}

int32_t CellularDataController::GetIntelligenceSwitchState(bool &switchState)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->GetIntelligenceSwitchState(switchState);
}

int32_t CellularDataController::SetIntelligenceSwitchEnable(bool userDataEnabled)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->SetIntelligenceSwitchEnable(userDataEnabled);
}

int32_t CellularDataController::IsCellularDataEnabled(bool &dataEnabled) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->IsCellularDataEnabled(dataEnabled);
}

ApnProfileState CellularDataController::GetCellularDataState() const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return ApnProfileState::PROFILE_STATE_FAILED;
    }
    return cellularDataHandler_->GetCellularDataState();
}

ApnProfileState CellularDataController::GetCellularDataState(const std::string &apnType) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return ApnProfileState::PROFILE_STATE_FAILED;
    }
    return cellularDataHandler_->GetCellularDataState(apnType);
}

int32_t CellularDataController::IsCellularDataRoamingEnabled(bool &dataRoamingEnabled) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->IsCellularDataRoamingEnabled(dataRoamingEnabled);
}

int32_t CellularDataController::SetCellularDataRoamingEnabled(bool dataRoamingEnabled)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataHandler_->SetCellularDataRoamingEnabled(dataRoamingEnabled);
}

bool CellularDataController::ReleaseNet(const NetRequest &request)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return false;
    }
    return cellularDataHandler_->ReleaseNet(request);
}

bool CellularDataController::RequestNet(const NetRequest &request)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
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
        TELEPHONY_LOGE("Slot%{public}d: event is null.", slotId_);
        return;
    }
    size_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case CellularDataEventCode::MSG_ASYNCHRONOUS_REGISTER_EVENT_ID:
            AsynchronousRegister();
            break;
        default:
            TELEPHONY_LOGE("Slot%{public}d: nothing to do", slotId_);
            break;
    }
}

void CellularDataController::RegisterEvents()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: core is null or cellularDataHandler is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: start", slotId_);
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_STATE_CHANGE, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_RECORDS_LOADED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_ACCOUNT_LOADED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_ATTACHED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_DETACHED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_OPEN, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_CLOSE, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_STATE_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_DSDS_MODE_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_RAT_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_CALL_STATUS_INFO, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_OPEN, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_CLOSE, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_STATE_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_FREQUENCY_CHANGED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_RIL_ADAPTER_HOST_DIED, nullptr);
    coreInner.RegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_FACTORY_RESET, nullptr);
    if (slotId_ == 0) {
        sptr<NetworkSearchCallback> networkSearchCallback = std::make_unique<NetworkSearchCallback>().release();
        if (networkSearchCallback != nullptr) {
            coreInner.RegisterCellularDataObject(networkSearchCallback);
        } else {
            TELEPHONY_LOGE("Slot%{public}d: networkSearchCallback is null", slotId_);
        }
    }
    coreInner.CleanAllConnections(slotId_, RadioEvent::RADIO_CLEAN_ALL_DATA_CONNECTIONS, cellularDataHandler_);
}

void CellularDataController::UnRegisterEvents()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler_ is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: start", slotId_);
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_STATE_CHANGE);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_RECORDS_LOADED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_SIM_ACCOUNT_LOADED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_ATTACHED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_CONNECTION_DETACHED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_OPEN);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_ROAMING_CLOSE);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_STATE_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_DSDS_MODE_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_PS_RAT_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_CALL_STATUS_INFO);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_OPEN);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_EMERGENCY_STATE_CLOSE);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_STATE_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_NR_FREQUENCY_CHANGED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_RIL_ADAPTER_HOST_DIED);
    coreInner.UnRegisterCoreNotify(slotId_, cellularDataHandler_, RadioEvent::RADIO_FACTORY_RESET);
    TELEPHONY_LOGI("Slot%{public}d: end", slotId_);
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

bool CellularDataController::ChangeConnectionForDsds(bool enable) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler is null", slotId_);
        return false;
    }
    cellularDataHandler_->ChangeConnectionForDsds(enable);
    return true;
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
            if (handler_ != nullptr) {
                handler_->ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
                CellularDataNetAgent::GetInstance().UnregisterNetSupplier(slotId_);
                CellularDataNetAgent::GetInstance().RegisterNetSupplier(slotId_);
            }
            break;
        case COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID:
            TELEPHONY_LOGI("COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID running");
            if (slotId_ == 0) {
                CellularDataNetAgent::GetInstance().UnregisterPolicyCallback();
                CellularDataNetAgent::GetInstance().RegisterPolicyCallback();
            }
            break;
        case COMMON_EVENT_SERVICE_ID:
            TELEPHONY_LOGI("COMMON_EVENT_SERVICE_ID running");
            if (handler_ != nullptr) {
                bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(handler_);
                TELEPHONY_LOGI("subscribeResult = %{public}d", subscribeResult);
            }
            break;
        case DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID:
            TELEPHONY_LOGI("DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID running");
            if (handler_ != nullptr) {
                handler_->RegisterDataSettingObserver();
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
            break;
        case COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID:
            TELEPHONY_LOGE("COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID stopped");
            break;
        case COMMON_EVENT_SERVICE_ID:
            TELEPHONY_LOGE("COMMON_EVENT_SERVICE_ID stopped");
            if (handler_ != nullptr) {
                bool unSubscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(handler_);
                TELEPHONY_LOGI("unSubscribeResult = %{public}d", unSubscribeResult);
            }
            break;
        case DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID:
            TELEPHONY_LOGE("DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID stopped");
            break;
        default:
            TELEPHONY_LOGE("systemAbilityId is invalid");
            break;
    }
}

void CellularDataController::GetDataConnApnAttr(ApnItem::Attribute &apnAttr) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: GetDataConnApnAttr cellularDataHandler_ is null", slotId_);
        return;
    }
    cellularDataHandler_->GetDataConnApnAttr(apnAttr);
}

std::string CellularDataController::GetDataConnIpType() const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: GetDataConnIpType cellularDataHandler_ is null", slotId_);
        return "";
    }
    return cellularDataHandler_->GetDataConnIpType();
}

int32_t CellularDataController::GetDataRecoveryState()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataHandler is null", slotId_);
        return false;
    }
    return cellularDataHandler_->GetDataRecoveryState();
}

void CellularDataController::IsNeedDoRecovery(bool needDoRecovery) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: IsNeedDoRecovery cellularDataHandler_ is null", slotId_);
        return;
    }
    cellularDataHandler_->IsNeedDoRecovery(needDoRecovery);
}
} // namespace Telephony
} // namespace OHOS
