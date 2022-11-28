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

#include "sim_account_callback_proxy.h"

#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
SimAccountCallbackProxy::SimAccountCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<SimAccountCallback>(impl)
{}

void SimAccountCallbackProxy::OnSimAccountChanged()
{
    MessageParcel data;
    MessageOption option;
    MessageParcel replyParcel;
    if (!data.WriteInterfaceToken(SimAccountCallbackProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return;
    }
    uint32_t code = 0;
    remote->SendRequest(code, data, replyParcel, option);
}
} // namespace Telephony
} // namespace OHOS