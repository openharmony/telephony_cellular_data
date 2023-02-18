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
CellularDataClient::CellularDataClient()
{
    defaultCellularDataSlotId_ = INVALID_MAIN_CARD_SLOTID;
    if (callback_ == nullptr) {
        callback_ = new DataSimAccountCallback();
    }
}

CellularDataClient::~CellularDataClient()
{
    UnregisterSimAccountCallback();
}

sptr<ICellularDataManager> CellularDataClient::GetProxy()
{
    std::lock_guard<std::mutex> lock(mutexProxy_);
    if (proxy_ != nullptr) {
        return proxy_;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        TELEPHONY_LOGE("Failed to get system ability manager");
        return nullptr;
    }
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID);
    if (obj == nullptr) {
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
    TELEPHONY_LOGI("Succeed to connect cellular data service %{public}d", proxy_ == nullptr);
    return proxy_;
}

void CellularDataClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        TELEPHONY_LOGE("OnRemoteDied failed, remote is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mutexProxy_);
    if (proxy_ == nullptr) {
        TELEPHONY_LOGE("OnRemoteDied proxy_ is nullptr");
        return;
    }
    sptr<IRemoteObject> serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
        TELEPHONY_LOGE("on remote died");
    }
}

bool CellularDataClient::IsConnect() const
{
    return (proxy_ != nullptr);
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
    TELEPHONY_LOGI("CellularDataClient::RegisterSimAccountCallback ret:%{public}d", ret);
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
    TELEPHONY_LOGI("CellularDataClient::UnregisterSimAccountCallback ret:%{public}d", ret);
}

int32_t CellularDataClient::GetDefaultCellularDataSlotId()
{
    RegisterSimAccountCallback();
    if (defaultCellularDataSlotId_ != INVALID_MAIN_CARD_SLOTID) {
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
    }
    return result;
}

int32_t CellularDataClient::UpdateDefaultCellularDataSlotId()
{
    defaultCellularDataSlotId_ = INVALID_MAIN_CARD_SLOTID;
    sptr<ICellularDataManager> proxy = GetProxy();
    if (proxy == nullptr) {
        TELEPHONY_LOGE("proxy is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    defaultCellularDataSlotId_ = proxy->GetDefaultCellularDataSlotId();
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
} // namespace Telephony
} // namespace OHOS