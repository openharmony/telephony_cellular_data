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

#include "cellular_data_net_agent.h"

#include <cinttypes>

#include "core_manager_inner.h"
#include "net_conn_client.h"
#include "net_policy_client.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;

CellularDataNetAgent::CellularDataNetAgent()
{
    callBack_ = std::make_unique<NetManagerCallBack>().release();
    tacticsCallBack_ = std::make_unique<NetManagerTacticsCallBack>().release();
    if (callBack_ == nullptr || tacticsCallBack_ == nullptr) {
        TELEPHONY_LOGE("Callback or tacticsCallBack init failed");
    }
}

CellularDataNetAgent::~CellularDataNetAgent() = default;

bool CellularDataNetAgent::RegisterNetSupplier(const int32_t slotId)
{
    bool flag = false;
    for (NetSupplier &netSupplier : netSuppliers_) {
        if (netSupplier.slotId != slotId) {
            continue;
        }
        std::shared_ptr<NetManagerStandard::NetConnClient> netManager = DelayedSingleton<NetConnClient>::GetInstance();
        if (netManager == nullptr) {
            TELEPHONY_LOGE("NetConnClient is null");
            return false;
        }
        if (netSupplier.capability > NetCap::NET_CAPABILITY_INTERNAL_DEFAULT) {
            TELEPHONY_LOGE("capabilities(%{public}" PRIu64 ") not support", netSupplier.capability);
            continue;
        }
        int32_t simId = CoreManagerInner::GetInstance().GetSimId(netSupplier.slotId);
        if (simId <= INVALID_SIM_ID) {
            return false;
        }
        std::set<NetCap> netCap { static_cast<NetCap>(netSupplier.capability) };
        uint32_t supplierId = 0;
        int32_t result = netManager->RegisterNetSupplier(
            NetBearType::BEARER_CELLULAR, std::string(IDENT_PREFIX) + std::to_string(simId), netCap, supplierId);
        if (result == NETMANAGER_SUCCESS) {
            TELEPHONY_LOGI("Register network successful, supplierId[%{public}d]", supplierId);
            flag = true;
            netSupplier.supplierId = supplierId;
            int32_t regCallback = netManager->RegisterNetSupplierCallback(netSupplier.supplierId, callBack_);
            TELEPHONY_LOGI("Register supplier callback(%{public}d)", regCallback);
        }
    }
    return flag;
}

void CellularDataNetAgent::UnregisterNetSupplier()
{
    for (const NetSupplier &netSupplier : netSuppliers_) {
        std::shared_ptr<NetManagerStandard::NetConnClient> netManager = DelayedSingleton<NetConnClient>::GetInstance();
        if (netManager == nullptr) {
            TELEPHONY_LOGE("NetConnClient is null");
            return;
        }
        int32_t result = netManager->UnregisterNetSupplier(netSupplier.supplierId);
        TELEPHONY_LOGI("Unregister network result:%{public}d", result);
    }
    netSuppliers_.clear();
}

bool CellularDataNetAgent::RegisterPolicyCallback()
{
    std::shared_ptr<NetManagerStandard::NetPolicyClient> netPolicy = DelayedSingleton<NetPolicyClient>::GetInstance();
    if (netPolicy == nullptr) {
        TELEPHONY_LOGE("Net Policy Client is null");
        return false;
    }
    int32_t registerResult = netPolicy->RegisterNetPolicyCallback(tacticsCallBack_);
    if (registerResult == NETMANAGER_SUCCESS) {
        TELEPHONY_LOGI("Register NetPolicy Callback successful");
        return true;
    }
    return false;
}

void CellularDataNetAgent::UnregisterPolicyCallback()
{
    std::shared_ptr<NetManagerStandard::NetPolicyClient> netPolicy = DelayedSingleton<NetPolicyClient>::GetInstance();
    if (netPolicy == nullptr) {
        TELEPHONY_LOGE("Net Policy Client is null");
        return;
    }
    int32_t registerResult = netPolicy->UnregisterNetPolicyCallback(tacticsCallBack_);
    TELEPHONY_LOGI("Register NetPolicy Callback is :%{public}d", registerResult);
}

void CellularDataNetAgent::UpdateNetSupplierInfo(
    int32_t supplierId, sptr<NetManagerStandard::NetSupplierInfo> &netSupplierInfo)
{
    std::shared_ptr<NetManagerStandard::NetConnClient> netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (netManager == nullptr) {
        TELEPHONY_LOGE("NetConnClient is null");
        return;
    }
    int32_t result = netManager->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    TELEPHONY_LOGI("Update network result:%{public}d", result);
}

void CellularDataNetAgent::UpdateNetLinkInfo(int32_t supplierId, sptr<NetManagerStandard::NetLinkInfo> &netLinkInfo)
{
    std::shared_ptr<NetManagerStandard::NetConnClient> netManager = DelayedSingleton<NetConnClient>::GetInstance();
    if (netManager == nullptr) {
        TELEPHONY_LOGE("NetConnClient is null");
        return;
    }
    int32_t result = netManager->UpdateNetLinkInfo(supplierId, netLinkInfo);
    TELEPHONY_LOGI("result:%{public}d", result);
}

void CellularDataNetAgent::AddNetSupplier(const NetSupplier &netSupplier)
{
    netSuppliers_.push_back(netSupplier);
}

void CellularDataNetAgent::ClearNetSupplier()
{
    netSuppliers_.clear();
}

int32_t CellularDataNetAgent::GetSupplierId(const int32_t slotId, uint64_t capability) const
{
    for (const NetSupplier &netSupplier : netSuppliers_) {
        if (netSupplier.slotId == slotId && netSupplier.capability == capability) {
            TELEPHONY_LOGI(
                "find supplierId %{public}d capability:%{public}" PRIu64 "", netSupplier.supplierId, capability);
            return netSupplier.supplierId;
        }
    }
    return 0;
}
} // namespace Telephony
} // namespace OHOS
