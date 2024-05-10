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

#include "cellular_data_client.h"

#include "__mutex_base"
#include "cellular_data_types.h"
#include "i_cellular_data_manager.h"
#include "if_system_ability_manager.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "memory"
#include "refbase.h"
#include "system_ability_definition.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
int32_t CellularDataClient::defaultCellularDataSlotId_ = INVALID_MAIN_CARD_SLOTID;
int32_t CellularDataClient::defaultCellularDataSimId_ = 0;
CellularDataClient::CellularDataClient()
{
    if (callback_ == nullptr) {
        callback_ = new DataSimAccountCallback();
    }
}

CellularDataClient::~CellularDataClient()
{
    UnregisterSimAccountCallback();
}

bool CellularDataClient::IsValidSlotId(int32_t slotId)
{
    return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT));
}

sptr<ICellularDataManager> CellularDataClient::GetProxy()
{
    std::lock_guard<std::mutex> lock(mutexProxy_);
    if (proxy_ != nullptr) {
        return proxy_;
    }

    sptr<IRemoteObject> obj;
    if (!IsCellularDataSysAbilityExist(obj)) {
        TELEPHONY_LOGE("Failed to get cellular data service");
        return nullptr;
    }
    std::unique_ptr<CellularDataDeathRecipient> recipient = std::make_unique<CellularDataDeathRecipient>(*this);
    if (recipient == nullptr) {
        TELEPHONY_LOGE("recipient is null");
        return nullptr;
    }
    sptr<IRemoteObject::DeathRecipient> dr(recipient.release());
    if ((obj->IsProxyObject()) && (!obj->AddDeathRecipient(dr))) {
        TELEPHONY_LOGE("Failed to add death recipient");
        return nullptr;
    }
    proxy_ = iface_cast<ICellularDataManager>(obj);
    deathRecipient_ = dr;
    TELEPHONY_LOGD("Succeed to connect cellular data service %{public}d", proxy_ == nullptr);
    return proxy_;
}

void CellularDataClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mutexProxy_);
    if (proxy_ == nullptr) {
        TELEPHONY_LOGE("proxy_ is nullptr");
        return;
    }
    sptr<IRemoteObject> serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
        defaultCellularDataSlotId_ = INVALID_MAIN_CARD_SLOTID;
        defaultCellularDataSimId_ = 0;
        registerStatus_ = false;
        TELEPHONY_LOGE("on remote died");
    }
}

bool CellularDataClient::IsConnect()
{
    sptr<ICellularDataManager> proxy = GetProxy();
    return (proxy != nullptr);
}

void CellularDataClient::RegisterSimAccountCallback()
{
    if (callback_ == nullptr) {
        TELEPHONY_LOGE("callback_ is nullptr");
        return;
    }
    if (registerStatus_) {
        return;
    }
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return;
    }
    int32_t ret = proxy->RegisterSimAccountCallback(callback_);
    TELEPHONY_LOGD("ret:%{public}d", ret);
    if (ret == TELEPHONY_ERR_SUCCESS) {
        registerStatus_ = true;
    }
}

void CellularDataClient::UnregisterSimAccountCallback()
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return;
    }
    int32_t ret = proxy->UnregisterSimAccountCallback();
    TELEPHONY_LOGD("ret:%{public}d", ret);
}

int32_t CellularDataClient::GetDefaultCellularDataSlotId()
{
    RegisterSimAccountCallback();
    if (IsValidSlotId(defaultCellularDataSlotId_)) {
        return defaultCellularDataSlotId_;
    }
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    defaultCellularDataSlotId_ = proxy->GetDefaultCellularDataSlotId();
    return defaultCellularDataSlotId_;
}

int32_t CellularDataClient::GetDefaultCellularDataSimId(int32_t &simId)
{
    RegisterSimAccountCallback();
    if (defaultCellularDataSimId_ > 0) {
        simId = defaultCellularDataSimId_;
        return TELEPHONY_ERR_SUCCESS;
    }
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = proxy->GetDefaultCellularDataSimId(simId);
    if (result == TELEPHONY_ERR_SUCCESS) {
        defaultCellularDataSimId_ = simId;
    }
    return result;
}

int32_t CellularDataClient::SetDefaultCellularDataSlotId(int32_t slotId)
{
    RegisterSimAccountCallback();
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = proxy->SetDefaultCellularDataSlotId(slotId);
    if (result == TELEPHONY_ERR_SUCCESS) {
        defaultCellularDataSlotId_ = slotId;
        int32_t simId = 0;
        int32_t ret = proxy->GetDefaultCellularDataSimId(simId);
        if (ret == TELEPHONY_ERR_SUCCESS) {
            defaultCellularDataSimId_ = simId;
        }
    }
    return result;
}

int32_t CellularDataClient::UpdateDefaultCellularDataSlotId()
{
    defaultCellularDataSlotId_ = INVALID_MAIN_CARD_SLOTID;
    defaultCellularDataSimId_ = 0;
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    defaultCellularDataSlotId_ = proxy->GetDefaultCellularDataSlotId();
    proxy->GetDefaultCellularDataSimId(defaultCellularDataSimId_);
    return defaultCellularDataSlotId_;
}

int32_t CellularDataClient::EnableCellularData(bool enable)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->EnableCellularData(enable);
}

int32_t CellularDataClient::EnableIntelligenceSwitch(bool enable)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->EnableIntelligenceSwitch(enable);
}

int32_t CellularDataClient::IsCellularDataEnabled(bool &dataEnabled)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->IsCellularDataEnabled(dataEnabled);
}

int32_t CellularDataClient::GetCellularDataState()
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->GetCellularDataState();
}

int32_t CellularDataClient::GetApnState(int32_t slotId, const std::string &apnType)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->GetApnState(slotId, apnType);
}

int32_t CellularDataClient::GetDataRecoveryState()
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->GetDataRecoveryState();
}

int32_t CellularDataClient::IsCellularDataRoamingEnabled(int32_t slotId, bool &dataRoamingEnabled)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->IsCellularDataRoamingEnabled(slotId, dataRoamingEnabled);
}

int32_t CellularDataClient::EnableCellularDataRoaming(int32_t slotId, bool enable)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->EnableCellularDataRoaming(slotId, enable);
}

int32_t CellularDataClient::GetCellularDataFlowType()
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->GetCellularDataFlowType();
}

int32_t CellularDataClient::HasInternetCapability(int32_t slotId, int32_t cid)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->HasInternetCapability(slotId, cid);
}

int32_t CellularDataClient::ClearCellularDataConnections(int32_t slotId)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->ClearCellularDataConnections(slotId);
}

int32_t CellularDataClient::GetDataConnApnAttr(int32_t slotId, ApnItem::Attribute &apnAttr)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->GetDataConnApnAttr(slotId, apnAttr);
}

int32_t CellularDataClient::GetDataConnIpType(int32_t slotId, std::string &ipType)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->GetDataConnIpType(slotId, ipType);
}

int32_t CellularDataClient::ClearAllConnections(int32_t slotId, DisConnectionReason reason)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->ClearAllConnections(slotId, reason);
}

int32_t CellularDataClient::HandleApnChanged(int32_t slotId)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->HandleApnChanged(slotId);
}

int32_t CellularDataClient::IsNeedDoRecovery(int32_t slotId, bool needDoRecovery)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->IsNeedDoRecovery(slotId, needDoRecovery);
}

int32_t CellularDataClient::InitCellularDataController(int32_t slotId)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->InitCellularDataController(slotId);
}

int32_t CellularDataClient::GetIntelligenceSwitchState(bool &switchState)
{
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy->GetIntelligenceSwitchState(switchState);
}

bool CellularDataClient::IsCellularDataSysAbilityExist(sptr<IRemoteObject> &object) __attribute__((no_sanitize("cfi")))
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        TELEPHONY_LOGE("IsCellularDataSysAbilityExist Get ISystemAbilityManager failed, no SystemAbilityManager");
        return false;
    }
    object = sm->CheckSystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID);
    if (object == nullptr) {
        TELEPHONY_LOGE("No CesServiceAbility");
        return false;
    }
    return true;
}
} // namespace Telephony
} // namespace OHOS
