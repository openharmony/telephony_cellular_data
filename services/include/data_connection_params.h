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

#ifndef DATA_CONNECTION_PARAMS_H
#define DATA_CONNECTION_PARAMS_H

#include <utility>

#include "inner_event.h"

#include "apn_holder.h"

namespace OHOS {
namespace Telephony {
class DataConnectionParams {
public:
    DataConnectionParams(
        sptr<ApnHolder> apnHolder, int32_t profileId, int32_t radioTechnology, bool nonTrafficUseOnly,
        bool roamingState, bool userDataRoaming)
        : apnHolder_(std::move(apnHolder)), profileId_(profileId), rat_(radioTechnology), roamingState_(roamingState),
        userRoaming_(userDataRoaming), nonTrafficUseOnly_(nonTrafficUseOnly)
    {}

    ~DataConnectionParams() = default;

    const sptr<ApnHolder> &GetApnHolder() const
    {
        return apnHolder_;
    }

    void SetApnHolder(const sptr<ApnHolder> &apnHolder)
    {
        apnHolder_ = apnHolder;
    }

    int32_t GetProfileId() const
    {
        return profileId_;
    }

    void SetProfileId(int32_t profileId)
    {
        profileId_ = profileId;
    }

    int32_t GetRat() const
    {
        return rat_;
    }

    void SetRat(int32_t rat)
    {
        rat_ = rat;
    }

    bool IsNonTrafficUseOnly() const
    {
        return nonTrafficUseOnly_;
    }

    void SetNonTrafficUseOnly(bool nonTrafficUseOnly)
    {
        nonTrafficUseOnly_ = nonTrafficUseOnly;
    }

    bool GetRoamingState() const
    {
        return roamingState_;
    }

    void SetRoamingState(bool roamingState)
    {
        roamingState_ = roamingState;
    }

    bool GetUserDataRoaming() const
    {
        return userRoaming_;
    }

    void GetUserDataRoaming(bool userDataRoaming)
    {
        userRoaming_ = userDataRoaming;
    }
private:
    DataConnectionParams(const DataConnectionParams &dataConnectionParams) = default;
    DataConnectionParams &operator=(const DataConnectionParams &dataConnectionParams) = default;

private:
    sptr<ApnHolder> apnHolder_;
    int32_t profileId_;
    int32_t rat_;
    bool roamingState_;
    bool userRoaming_;
    bool nonTrafficUseOnly_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_CONNECTION_PARAMS_H
