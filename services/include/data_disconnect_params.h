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

#ifndef DATA_DISCONNECT_PARAMS_H
#define DATA_DISCONNECT_PARAMS_H

#include <string>
#include <utility>

#include "inner_event.h"

namespace OHOS {
namespace Telephony {
class DataDisconnectParams {
public:
    DataDisconnectParams(const std::string &apnType, DisConnectionReason reason)
        : apnType_(apnType), reason_(reason)
    {}

    ~DataDisconnectParams() = default;

    const std::string &GetApnType() const
    {
        return apnType_;
    }

    void SetApnType(const std::string &apnType)
    {
        apnType_ = apnType;
    }

    DisConnectionReason GetReason() const
    {
        return reason_;
    }

    void SetReason(DisConnectionReason reason)
    {
        reason_ = reason;
    }

private:
    DataDisconnectParams(const DataDisconnectParams &dataDisconnectParams) = default;
    DataDisconnectParams &operator=(const DataDisconnectParams &dataDisconnectParams) = default;

private:
    std::string apnType_;
    DisConnectionReason reason_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_DISCONNECT_PARAMS_H
