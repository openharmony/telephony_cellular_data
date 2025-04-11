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

#ifndef CELLULAR_DATA_TYPES_H
#define CELLULAR_DATA_TYPES_H

#include <cstdint>
#include <parcel.h>

namespace OHOS {
namespace Telephony {
enum class DataConnectionStatus : int32_t {
    DATA_STATE_DISCONNECTED = 11,
    DATA_STATE_CONNECTING = 12,
    DATA_STATE_CONNECTED = 13,
    DATA_STATE_SUSPENDED = 14
};

enum class CellDataFlowType : int32_t {
    DATA_FLOW_TYPE_NONE = 0,
    DATA_FLOW_TYPE_DOWN = 1,
    DATA_FLOW_TYPE_UP = 2,
    DATA_FLOW_TYPE_UP_DOWN = 3,
    DATA_FLOW_TYPE_DORMANT = 4
};

enum class DataSwitchCode : int32_t {
    CELLULAR_DATA_DISABLED = 0,
    CELLULAR_DATA_ENABLED = 1
};

enum class IntelligenceSwitchCode : int32_t {
    INTELLIGENCE_SWITCH_DISABLED = 0,
    INTELLIGENCE_SWITCH_ENABLED = 1
};

enum class RoamingSwitchCode : int32_t {
    CELLULAR_DATA_ROAMING_DISABLED = 0,
    CELLULAR_DATA_ROAMING_ENABLED = 1
};

enum class DataRespondCode : int32_t {
    SET_FAILED = 0,
    SET_SUCCESS = 1
};

enum class RequestNetCode : int32_t {
    REQUEST_FAILED = 0,
    REQUEST_SUCCESS = 1
};

enum class DataConnectState : int32_t {
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

struct ApnInfo : public Parcelable {
    std::u16string apnName = u"";
    std::u16string apn = u"";
    std::u16string mcc = u"";
    std::u16string mnc = u"";
    std::u16string user = u"";
    std::u16string type = u"";
    std::u16string proxy = u"";
    std::u16string mmsproxy = u"";

    bool Marshalling(Parcel &parcel) const
    {
        if (!parcel.WriteString16(apnName)) {
            return false;
        }
        if (!parcel.WriteString16(apn)) {
            return false;
        }
        if (!parcel.WriteString16(mcc)) {
            return false;
        }
        if (!parcel.WriteString16(mnc)) {
            return false;
        }
        if (!parcel.WriteString16(user)) {
            return false;
        }
        if (!parcel.WriteString16(type)) {
            return false;
        }
        if (!parcel.WriteString16(proxy)) {
            return false;
        }
        if (!parcel.WriteString16(mmsproxy)) {
            return false;
        }
        return true;
    };

    std::shared_ptr<ApnInfo> UnMarshalling(Parcel &parcel)
    {
        std::shared_ptr<ApnInfo> param = std::make_shared<ApnInfo>();
        if (param == nullptr || !param->ReadFromParcel(parcel)) {
            param = nullptr;
        }
        return param;
    };

    bool ReadFromParcel(Parcel &parcel)
    {
        parcel.ReadString16(apnName);
        parcel.ReadString16(apn);
        parcel.ReadString16(mcc);
        parcel.ReadString16(mnc);
        parcel.ReadString16(user);
        parcel.ReadString16(type);
        parcel.ReadString16(proxy);
        parcel.ReadString16(mmsproxy);
        return true;
    };
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_TYPES_H