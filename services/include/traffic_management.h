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

#ifndef TELEPHONY_TRAFFIC_MANAGEMENT_H
#define TELEPHONY_TRAFFIC_MANAGEMENT_H

#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
class TrafficManagement {
public:
    TrafficManagement();
    ~TrafficManagement();

    /**
     * Get the packet data
     *
     * @param sendPackets transport data
     * @param recvPackets receive data
     */
    void GetPacketData(int64_t &sendPackets, int64_t &recvPackets);

    /**
     * Update packet data
     */
    void UpdatePacketData();

private:
    int64_t sendPackets_ = 0;
    int64_t recvPackets_ = 0;
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_TRAFFIC_MANAGEMENT_H
