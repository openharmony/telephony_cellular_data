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

#include "common_event_manager.h"
#include "uri.h"

#include "core_manager.h"
#include "telephony_log_wrapper.h"

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "network_search_callback.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;

CellularDataController::CellularDataController(std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId)
    : AppExecFwk::EventHandler(runner), slotId_(slotId)
{}

CellularDataController::~CellularDataController()
{
    UnRegisterEvents();
    UnRegisterDataObserver();
}

void CellularDataController::Init()
{
    cellularDataHandler_ = std::make_shared<CellularDataHandler>(GetEventRunner(), slotId_);
    settingObserver_ = std::make_unique<CellularDataSettingObserver>(cellularDataHandler_).release();
    roamingObserver_ = std::make_unique<CellularDataRoamingObserver>(cellularDataHandler_).release();
    cellularDataRdbObserver_ = std::make_unique<CellularDataRdbObserver>(cellularDataHandler_).release();
    if (cellularDataHandler_ == nullptr || settingObserver_ == nullptr || cellularDataRdbObserver_ == nullptr) {
        TELEPHONY_LOGE("CellularDataController init failed, "
            "cellularDataHandler_ or settingObserver_ or cellularDataRdbObserver_ is null");
        return;
    }
    cellularDataHandler_->Init();
    RegisterDatabaseObserver();
}

bool CellularDataController::SetCellularDataEnable(bool userDataEnabled)
{
    TELEPHONY_LOGI("userDataEnabled =: %{public}d", userDataEnabled);
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("SetCellularDataEnable cellularDataHandler_ is null");
        return false;
    }
    return cellularDataHandler_->SetCellularDataEnable(userDataEnabled);
}

bool CellularDataController::IsCellularDataEnabled() const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("IsCellularDataEnabled cellularDataHandler_ is null");
        return false;
    }
    return cellularDataHandler_->IsCellularDataEnabled();
}

ApnProfileState CellularDataController::GetCellularDataState(const std::string &apnType) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("GetCellularDataState cellularDataHandler_ is null");
        return ApnProfileState::PROFILE_STATE_FAILED;
    }
    return cellularDataHandler_->GetCellularDataState();
}

bool CellularDataController::IsCellularDataRoamingEnabled() const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("IsCellularDataRoamingEnabled cellularDataHandler_ is null");
        return false;
    }
    return cellularDataHandler_->IsCellularDataRoamingEnabled();
}

bool CellularDataController::SetCellularDataRoamingEnabled(bool dataRoamingEnabled)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("SetCellularDataRoamingEnabled cellularDataHandler is null");
        return false;
    }
    return cellularDataHandler_->SetCellularDataRoamingEnabled(dataRoamingEnabled);
}

bool CellularDataController::ReleaseNet(const NetRequest &request)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("ReleaseNet cellularDataHandler_ is null");
        return false;
    }
    return cellularDataHandler_->ReleaseNet(request);
}

bool CellularDataController::RequestNet(const NetRequest &request)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("RequestNet cellularDataHandler_ is null");
        return false;
    }
    return cellularDataHandler_->RequestNet(request);
}

void CellularDataController::SendRegisterNetManagerEvent()
{
    SendEvent(CellularDataEventCode::MSG_REG_NET_MANAGER, REG_NET_MANAGER_DELAY_TIME, Priority::LOW);
}

void CellularDataController::SendRegisterPolicyEvent()
{
    SendEvent(CellularDataEventCode::MSG_REG_POLICY_CALL_BACK, REG_NET_MANAGER_DELAY_TIME, Priority::LOW);
}

void CellularDataController::AsynchronousRegister()
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr && core->IsInitCore()) {
        TELEPHONY_LOGI("core inited %{public}d", slotId_);
        Init();
        RegisterEvents();
        return;
    }
    TELEPHONY_LOGI("AsynchronousRegister := %{public}d", slotId_);
    SendEvent(CellularDataEventCode::MSG_ASYNCHRONOUS_REGISTER_EVENT_ID, CORE_INIT_DELAY_TIME, Priority::HIGH);
}

void CellularDataController::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("ProcessEvent event is null.");
        return;
    }
    int32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case CellularDataEventCode::MSG_REG_NET_MANAGER: {
            if (!CellularDataNetAgent::GetInstance().RegisterNetSupplier()) {
                SendRegisterNetManagerEvent();
            }
            break;
        }
        case CellularDataEventCode::MSG_REG_POLICY_CALL_BACK: {
            if (CellularDataNetAgent::GetInstance().RegisterPolicyCallback() != 0) {
                SendRegisterPolicyEvent();
            }
            break;
        }
        case CellularDataEventCode::MSG_ASYNCHRONOUS_REGISTER_EVENT_ID:
            AsynchronousRegister();
            break;
        default:
            TELEPHONY_LOGE("ProcessEvent nothing to do");
            break;
    }
}

void CellularDataController::RegisterEvents()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("core is null or cellularDataHandler is null");
        return;
    }
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr) {
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_SIM_STATE_CHANGE, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_SIM_RECORDS_LOADED, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_CONNECTION_ATTACHED, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_CONNECTION_DETACHED, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_ROAMING_OPEN, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_ROAMING_CLOSE, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_STATE_CHANGED, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_RAT_CHANGED, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_CALL_STATUS_INFO, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_EMERGENCY_STATE_OPEN, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_EMERGENCY_STATE_CLOSE, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_NR_STATE_CHANGED, nullptr);
        core->RegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_NR_FREQUENCY_CHANGED, nullptr);
        sptr<NetworkSearchCallback> networkSearchCallback = std::make_unique<NetworkSearchCallback>().release();
        if (networkSearchCallback == nullptr) {
            TELEPHONY_LOGE("networkSearchCallback is null");
            return;
        }
        core->RegisterCellularDataObject(networkSearchCallback);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId_);
    }
}

void CellularDataController::UnRegisterEvents()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("UnRegisterEvents cellularDataHandler_ is null");
        return;
    }
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId_);
    if (core != nullptr) {
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_SIM_STATE_CHANGE);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_SIM_RECORDS_LOADED);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_CONNECTION_ATTACHED);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_CONNECTION_DETACHED);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_ROAMING_OPEN);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_ROAMING_CLOSE);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_STATE_CHANGED);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_PS_RAT_CHANGED);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_CALL_STATUS_INFO);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_EMERGENCY_STATE_OPEN);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_EMERGENCY_STATE_CLOSE);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_NR_STATE_CHANGED);
        core->UnRegisterCoreNotify(cellularDataHandler_, ObserverHandler::RADIO_NR_FREQUENCY_CHANGED);
        core->UnRegisterCellularDataObject(nullptr);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId_);
    }
}

void CellularDataController::UnRegisterDataObserver()
{
    std::shared_ptr<CellularDataRdbHelper> cellularDataDbHelper = CellularDataRdbHelper::GetInstance();
    cellularDataDbHelper->UnRegisterObserver(cellularDataRdbObserver_);
    Uri dataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    Uri dataRoamingUri(CELLULAR_DATA_SETTING_DATA_ROAMING_URI);
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    settingHelper->UnRegisterSettingsObserver(dataEnableUri, settingObserver_);
    settingHelper->UnRegisterSettingsObserver(dataRoamingUri, roamingObserver_);
}

void CellularDataController::RegisterDatabaseObserver()
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    Uri dataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    Uri dataRoamingUri(CELLULAR_DATA_SETTING_DATA_ROAMING_URI);
    settingHelper->RegisterSettingsObserver(dataEnableUri, settingObserver_);
    settingHelper->RegisterSettingsObserver(dataRoamingUri, roamingObserver_);
    std::shared_ptr<CellularDataRdbHelper> cellularDataDbHelper = CellularDataRdbHelper::GetInstance();
    cellularDataDbHelper->RegisterObserver(cellularDataRdbObserver_);
}

int32_t CellularDataController::HandleApnChanged(const std::string &apns)
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("ApnChanged cellularDataHandler_ is null");
        return static_cast<int32_t>(DataRespondCode::SET_FAILED);
    }
    return cellularDataHandler_->HandleApnChanged(apns);
}

int32_t CellularDataController::GetCellularDataFlowType()
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("cellular data handler is null");
        return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    }
    return cellularDataHandler_->GetCellularDataFlowType();
}

void CellularDataController::ClearAllConnectionsFormerSlot()
{
    if (cellularDataHandler_ != nullptr) {
        cellularDataHandler_->ClearAllConnectionsFormerSlot();
    }
}

void CellularDataController::ConnectDataNetWork()
{
    if (cellularDataHandler_ != nullptr) {
        cellularDataHandler_->ConnectDataNeWork();
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

int32_t CellularDataController::GetDisConnectionReason()
{
    if (cellularDataHandler_ != nullptr) {
        return cellularDataHandler_->GetDisConnectionReason();
    }
    return REASON_NORMAL;
}

bool CellularDataController::HasInternetCapability(const int32_t cid) const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("cellularDataHandler_ is null");
        return false;
    }
    return cellularDataHandler_->HasInternetCapability(cid);
}

bool CellularDataController::ClearAllConnections() const
{
    if (cellularDataHandler_ == nullptr) {
        TELEPHONY_LOGE("SetCellularDataRoamingEnabled cellularDataHandler is null");
        return false;
    }
    cellularDataHandler_->ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    return true;
}
} // namespace Telephony
} // namespace OHOS