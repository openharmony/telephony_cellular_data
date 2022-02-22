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

#ifndef CELLULAR_DATA_NET_AGENT_H
#define CELLULAR_DATA_NET_AGENT_H

#include <memory>
#include <string>
#include <singleton.h>
#include <utility>

#include "i_net_conn_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "cellular_data_constant.h"
#include "net_manager_call_back.h"
#include "net_manager_tactics_call_back.h"

namespace OHOS {
namespace Telephony {
class CellularDataNetAgent : public DelayedRefSingleton<CellularDataNetAgent> {
    DECLARE_DELAYED_REF_SINGLETON(CellularDataNetAgent);

public:
    /**
     * Register the network information with the network management module
     *
     * @return true if register success else return false;
     */
    bool RegisterNetSupplier(const int32_t slotId);

    /**
     * Cancel the registration information to the network management
     */
    void UnregisterNetSupplier();

    bool RegisterPolicyCallback();
    void UnregisterPolicyCallback();

    /**
     * Update network information
     *
     * @param supplierId network unique identity id returned after network registration
     * @param netSupplierInfo network data information
     */
    void UpdateNetSupplierInfo(int32_t supplierId, sptr<NetManagerStandard::NetSupplierInfo> &netSupplierInfo);

    /**
     * Update link information
     *
     * @param supplierId network unique identity id returned after network registration
     * @param netLinkInfo network link data information
     */
    void UpdateNetLinkInfo(int32_t supplierId, sptr<NetManagerStandard::NetLinkInfo> &netLinkInfo);

    /**
     * Collect all network
     *
     * @param netSupplier a specific network
     */
    void AddNetSupplier(const NetSupplier &netSupplier);

    /**
     * Clear all network
     */
    void ClearNetSupplier();

    /**
     * Gets the unique identity of successful registered registration
     *
     * @param slotId card slot identification
     * @param capability a network capability
     * @return unique identify
     */
    int32_t GetSupplierId(const int32_t slotId, uint64_t capability) const;

private:
    std::vector<NetSupplier> netSuppliers_;
    sptr<NetManagerCallBack> callBack_;
    sptr<NetManagerTacticsCallBack> tacticsCallBack_;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_NET_AGENT_H
