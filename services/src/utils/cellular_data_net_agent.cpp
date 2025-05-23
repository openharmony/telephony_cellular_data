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

#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "net_conn_client.h"
#include "net_policy_client.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;
namespace {
constexpr int32_t MAX_CAPABILITY_SIZE = 13;
}

CellularDataNetAgent::CellularDataNetAgent()
{
    netSuppliers_.resize(CoreManagerInner::GetInstance().GetMaxSimCount() * MAX_CAPABILITY_SIZE);
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
        auto& netManager = NetConnClient::GetInstance();
        if (netSupplier.capability > NetCap::NET_CAPABILITY_SNSSAI6) {
            TELEPHONY_LOGE("capabilities(%{public}" PRIu64 ") not support", netSupplier.capability);
            continue;
        }
        int32_t simId = CoreManagerInner::GetInstance().GetSimId(netSupplier.slotId);
        if (simId <= INVALID_SIM_ID) {
            TELEPHONY_LOGE("Slot%{public}d Invalid simId: %{public}d", slotId, simId);
            continue;
        }
        std::set<NetCap> netCap { static_cast<NetCap>(netSupplier.capability) };
        uint32_t supplierId = 0;
        int32_t result = netManager.RegisterNetSupplier(
            NetBearType::BEARER_CELLULAR, std::string(IDENT_PREFIX) + std::to_string(simId), netCap, supplierId);
        TELEPHONY_LOGI(
            "Slot%{public}d Register network supplierId: %{public}d,result:%{public}d", slotId, supplierId, result);
        if (result == NETMANAGER_SUCCESS) {
            flag = true;
            netSupplier.supplierId = supplierId;
            netSupplier.simId = simId;
            int32_t regCallback = netManager.RegisterNetSupplierCallback(netSupplier.supplierId, callBack_);
            TELEPHONY_LOGI("Register supplier callback(%{public}d)", regCallback);
            sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo();
            if (netSupplierInfo != nullptr) {
                netSupplierInfo->isAvailable_ = false;
                int32_t updateResult = UpdateNetSupplierInfo(netSupplier.supplierId, netSupplierInfo);
                TELEPHONY_LOGI("Update network result:%{public}d", updateResult);
            }
            int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
            CoreManagerInner::GetInstance().GetPsRadioTech(slotId, radioTech);
            RegisterSlotType(supplierId, radioTech);
            TELEPHONY_LOGI("RegisterSlotType: supplierId[%{public}d] slotId[%{public}d] radioTech[%{public}d]",
                supplierId, slotId, radioTech);
        }
    }
    return flag;
}

void CellularDataNetAgent::UnregisterNetSupplier(const int32_t slotId)
{
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d Invalid simId: %{public}d", slotId, simId);
        return;
    }
    for (const NetSupplier &netSupplier : netSuppliers_) {
        if (netSupplier.simId != simId) {
            continue;
        }
        auto& netManager = NetConnClient::GetInstance();
        int32_t result = netManager.UnregisterNetSupplier(netSupplier.supplierId);
        TELEPHONY_LOGI("Slot%{public}d unregister network result:%{public}d", slotId, result);
    }
}

void CellularDataNetAgent::UnregisterNetSupplierForSimUpdate(const int32_t slotId)
{
    for (NetSupplier &netSupplier : netSuppliers_) {
        if (netSupplier.slotId != slotId || netSupplier.simId <= INVALID_SIM_ID) {
            continue;
        }
        auto& netManager = NetConnClient::GetInstance();
        int32_t result = netManager.UnregisterNetSupplier(netSupplier.supplierId);
        TELEPHONY_LOGI("Slot%{public}d unregister network result:%{public}d", slotId, result);
        if (result == NETMANAGER_SUCCESS) {
            netSupplier.simId = INVALID_SIM_ID;
        }
    }
}

void CellularDataNetAgent::UnregisterAllNetSupplier()
{
    for (const NetSupplier &netSupplier : netSuppliers_) {
        int32_t result = NetConnClient::GetInstance().UnregisterNetSupplier(netSupplier.supplierId);
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
    TELEPHONY_LOGI("Unregister NetPolicy Callback is :%{public}d", registerResult);
}

int32_t CellularDataNetAgent::UpdateNetSupplierInfo(
    uint32_t supplierId, sptr<NetManagerStandard::NetSupplierInfo> &netSupplierInfo)
{
    int32_t result = NetConnClient::GetInstance().UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    if (result != NETMANAGER_SUCCESS) {
        TELEPHONY_LOGE("Update network fail, result:%{public}d", result);
    }
    for (NetSupplier &netSupplier : netSuppliers_) {
        if (netSupplier.supplierId == supplierId) {
            netSupplier.regState = result;
        }
    }
    return result;
}

void CellularDataNetAgent::UpdateNetLinkInfo(int32_t supplierId, sptr<NetManagerStandard::NetLinkInfo> &netLinkInfo)
{
    int32_t result = NetConnClient::GetInstance().UpdateNetLinkInfo(supplierId, netLinkInfo);
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

void CellularDataNetAgent::RegisterSlotType(int32_t supplierId, int32_t radioTech)
{
    int32_t result = NetConnClient::GetInstance().RegisterSlotType(supplierId, radioTech);
    TELEPHONY_LOGI("result:%{public}d", result);
}

bool CellularDataNetAgent::GetSupplierRegState(uint32_t supplierId, int32_t &regState)
{
    auto it = std::find_if(netSuppliers_.begin(), netSuppliers_.end(), [supplierId](const auto &netSupplier) {
        return netSupplier.supplierId == supplierId;
    });
    if (it != netSuppliers_.end()) {
        regState = it->regState;
        return true;
    } else {
        TELEPHONY_LOGE("not find the supplier, supplierId = %{public}d", supplierId);
        return false;
    }
}

void CellularDataNetAgent::SetReuseSupplierId(int32_t supplierId, int32_t reuseSupplierId, bool isReused)
{
    int32_t result = NetConnClient::GetInstance().SetReuseSupplierId(supplierId, reuseSupplierId, isReused);
    TELEPHONY_LOGI("result:%{public}d", result);
}

} // namespace Telephony
} // namespace OHOS
