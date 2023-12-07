/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DATA_SERVICE_EXT_WRAPPER_H
#define DATA_SERVICE_EXT_WRAPPER_H

#include "nocopyable.h"
#include "singleton.h"

namespace OHOS {
namespace Telephony {
class DataServiceExtWrapper final {
DECLARE_DELAYED_REF_SINGLETON(DataServiceExtWrapper);

public:
    DISALLOW_COPY_AND_MOVE(DataServiceExtWrapper);
    void InitDataServiceExtWrapper();

    typedef void (*REQUEST_TCP_AND_DNS_PACKETS)();

    REQUEST_TCP_AND_DNS_PACKETS requestTcpAndDnsPackets_ = nullptr;

private:
    void* DataServiceExtWrapperHandle_ = nullptr;
};

#define DATA_SERVICE_EXT_WRAPPER ::OHOS::DelayedRefSingleton<DataServiceExtWrapper>::GetInstance()
} // namespace Telephony
} // namespace OHOS
#endif // DATA_SERVICE_EXT_WRAPPER_H