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

#ifndef BOOSTER_NET_WRAPPER_H
#define BOOSTER_NET_WRAPPER_H

#include "nocopyable.h"
#include "singleton.h"

namespace OHOS {
namespace Telephony {
class BoosterNetWrapper final {
DECLARE_DELAYED_REF_SINGLETON(BoosterNetWrapper);

public:
    DISALLOW_COPY_AND_MOVE(BoosterNetWrapper);
    void InitBoosterNetWrapper();

    typedef void (*SEND_GET_TCP_SUM_TO_KERNEL)();

    SEND_GET_TCP_SUM_TO_KERNEL sendGetTcpSumToKernel_ = nullptr;

private:
    void* boosterNetWrapperHandle_ = nullptr;
};

#define BOOSTER_NET_WRAPPER ::OHOS::DelayedRefSingleton<BoosterNetWrapper>::GetInstance()
} // namespace Telephony
} // namespace OHOS
#endif // BOOSTER_NET_WRAPPER_H