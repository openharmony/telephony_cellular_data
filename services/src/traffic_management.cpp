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

#include "data_flow_statistics.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;

TrafficManagement::TrafficManagement() = default;

TrafficManagement::~TrafficManagement() = default;

void TrafficManagement::GetPacketData(int64_t &sendPackets, int64_t &recvPackets)
{
    sendPackets = sendPackets_;
    recvPackets = recvPackets_;
}

void TrafficManagement::UpdatePacketData()
{
    DataFlowStatistics dataState;
    const std::string interfaceName = "rmnet0";
    sendPackets_ = dataState.GetIfaceTxPackets(interfaceName);
    recvPackets_ = dataState.GetIfaceRxPackets(interfaceName);
}
} // namespace Telephony
} // namespace OHOS
