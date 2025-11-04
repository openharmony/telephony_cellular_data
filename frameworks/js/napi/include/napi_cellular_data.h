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

#ifndef NAPI_CELLULAR_DATA_H
#define NAPI_CELLULAR_DATA_H

#include "base_context.h"
#include "cellular_data_types.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
struct AsyncContext : BaseContext {
    int32_t slotId = 0;
    int32_t result = ERROR_SERVICE_UNAVAILABLE;
};

struct AsyncSetPreferApnContext : BaseContext {
    int32_t apnId = 0;
    int32_t result = ERROR_SERVICE_UNAVAILABLE;
};

struct PermissionPara {
    std::string func = "";
    std::string permission = "";
};

template<typename T>
struct AsyncContext1 {
    BaseContext context;
    int32_t slotId = ERROR_DEFAULT;
    T callbackVal;
    std::mutex callbackMutex;
    std::condition_variable cv;
    bool isCallbackEnd = false;
    bool isSendRequest = false;
};

struct AsyncQueryApnInfo {
    AsyncContext1<napi_value> asyncContext;
    ApnInfo queryApnPara;
    std::vector<uint32_t> apnIdList {};
};

struct AsyncQueryAllApnInfo {
    AsyncContext1<napi_value> asyncContext;
    std::vector<ApnInfo> allApnInfoList {};
};

struct AsyncGetActiveApnName {
    AsyncContext1<napi_value> asyncContext;
    std::string apnName;
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_CELLULAR_DATA_H
