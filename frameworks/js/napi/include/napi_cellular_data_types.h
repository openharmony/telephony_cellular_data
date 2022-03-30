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

#ifndef NAPI_CELLULAR_DATA_TYPES_H
#define NAPI_CELLULAR_DATA_TYPES_H

#include <cstdint>

namespace OHOS {
namespace Telephony {
enum class DataConnectionState : int32_t {
    /**
     * Indicates that a cellular data link is unknown.
     */
    DATA_STATE_UNKNOWN = -1,

    /**
     * Indicates that a cellular data link is disconnected.
     */
    DATA_STATE_DISCONNECTED = 0,

    /**
     * Indicates that a cellular data link is being connected.
     */
    DATA_STATE_CONNECTING = 1,

    /**
     * Indicates that a cellular data link is connected.
     */
    DATA_STATE_CONNECTED = 2,

    /**
     * Indicates that a cellular data link is suspended.
     */
    DATA_STATE_SUSPENDED = 3
};

enum class DataFlowType : int32_t {
    /**
     * Indicates that there is no uplink or downlink data.
     */
    DATA_FLOW_TYPE_NONE = 0,

    /**
     * Indicates that there is only downlink data.
     */
    DATA_FLOW_TYPE_DOWN = 1,

    /**
     * Indicates that there is only uplink data.
     */
    DATA_FLOW_TYPE_UP = 2,

    /**
     * Indicates that there is uplink and downlink data.
     */
    DATA_FLOW_TYPE_UP_DOWN = 3,

    /**
     * Indicates that there is no uplink or downlink data, and the bottom-layer link is in the dormant state.
     */
    DATA_FLOW_TYPE_DORMANT = 4
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_CELLULAR_DATA_TYPES_H