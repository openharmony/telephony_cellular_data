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

#include "ril_adapter_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void RilAdapterUtils::GetPdpContextList(int32_t slotId, const AppExecFwk::InnerEvent::Pointer &response)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->GetPdpContextList(response);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

void RilAdapterUtils::RegisterRilConnected(
    int32_t slotId, const std::shared_ptr<AppExecFwk::EventHandler> &handler, int what, void *obj)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->RegisterCoreNotify(handler, what, obj);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

void RilAdapterUtils::PdpContextListUpdated(
    int32_t slotId, const std::shared_ptr<AppExecFwk::EventHandler> &handler, int what, void *obj)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->RegisterCoreNotify(handler, what, obj);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

void RilAdapterUtils::UnRegisterRilConnected(int32_t slotId, int what)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->UnRegisterCoreNotify(nullptr, what);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

void RilAdapterUtils::UnPdpContextListUpdated(int32_t slotId, int what)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->UnRegisterCoreNotify(nullptr, what);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

void RilAdapterUtils::GetPreferredNetworkPara(int32_t slotId, const AppExecFwk::InnerEvent::Pointer &response)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->GetPreferredNetworkPara(response);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

void RilAdapterUtils::SetPreferredNetworkPara(
    int32_t slotId, int32_t preferredNetworkType, const AppExecFwk::InnerEvent::Pointer &response)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->SetPreferredNetworkPara(preferredNetworkType, response);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}

void RilAdapterUtils::GetLinkBandwidthInfo(
    int32_t slotId, int32_t cid, const AppExecFwk::InnerEvent::Pointer &response)
{
    std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
    if (core != nullptr) {
        core->GetLinkBandwidthInfo(cid, response);
    } else {
        TELEPHONY_LOGE("core is null slotId:%{public}d", slotId);
    }
}
} // namespace Telephony
} // namespace OHOS