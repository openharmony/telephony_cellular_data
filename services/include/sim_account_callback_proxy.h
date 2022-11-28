/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef SIM_ACCOUNT_CALLBACK_PROXY_H
#define SIM_ACCOUNT_CALLBACK_PROXY_H

#include "sim_account_callback.h"

namespace OHOS {
namespace Telephony {
class SimAccountCallbackProxy : public IRemoteProxy<SimAccountCallback> {
public:
    explicit SimAccountCallbackProxy(const sptr<IRemoteObject> &impl);
    void OnSimAccountChanged() override;

private:
    static inline BrokerDelegator<SimAccountCallbackProxy> delegator_;
};
} // namespace Telephony
} // namespace OHOS
#endif // SIM_ACCOUNT_CALLBACK_PROXY_H