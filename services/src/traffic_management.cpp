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

#include "traffic_management.h"

#include "core_manager_inner.h"
#include "data_flow_statistics.h"
#include "net_conn_client.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;

TrafficManagement::TrafficManagement(int32_t slotId) : slotId_(slotId) {}

TrafficManagement::~TrafficManagement() = default;

void TrafficManagement::GetPacketData(int64_t &sendPackets, int64_t &recvPackets)
{
    sendPackets = sendPackets_;
    recvPackets = recvPackets_;
}

void TrafficManagement::UpdatePacketData()
{
    DataFlowStatistics dataState;
    const std::string interfaceName = GetIfaceName();
    if (!interfaceName.empty()) {
        sendPackets_ = dataState.GetIfaceTxPackets(interfaceName);
        recvPackets_ = dataState.GetIfaceRxPackets(interfaceName);
    }
}

std::string TrafficManagement::GetIfaceName()
{
    std::string ifaceName = "";
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    std::list<int32_t> netIdList;
    int32_t ret = NetConnClient::GetInstance().GetNetIdByIdentifier(IDENT_PREFIX + std::to_string(simId), netIdList);
    if (ret != NETMANAGER_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: get netIdList by identifier failed, ret = %{public}d", slotId_, ret);
        return ifaceName;
    }
    std::list<sptr<NetManagerStandard::NetHandle>> netList;
    int32_t result = NetConnClient::GetInstance().GetAllNets(netList);
    if (result != NETMANAGER_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: get all nets failed, ret = %{public}d", slotId_, result);
        return ifaceName;
    }
    for (sptr<NetManagerStandard::NetHandle> netHandle : netList) {
        for (auto netId : netIdList) {
            TELEPHONY_LOGD("Slot%{public}d: netId = %{public}d, netHandle->GetNetId() = %{public}d", slotId_, netId,
                netHandle->GetNetId());
            if (netId == netHandle->GetNetId()) {
                NetLinkInfo info;
                NetConnClient::GetInstance().GetConnectionProperties(*netHandle, info);
                ifaceName = info.ifaceName_;
                TELEPHONY_LOGD("Slot%{public}d: data is connected ifaceName = %{public}s", slotId_, ifaceName.c_str());
                return ifaceName;
            }
        }
    }
    return ifaceName;
}
} // namespace Telephony
} // namespace OHOS
