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

#ifndef RIL_ADAPTER_UTILS_H
#define RIL_ADAPTER_UTILS_H

#include "event_handler.h"
#include "inner_event.h"
#include "i_net_conn_service.h"

#include "core_manager.h"
#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
class RilAdapterUtils {
public:
    static void GetPdpContextList(int32_t slotId, const AppExecFwk::InnerEvent::Pointer &response);
    static void RegisterRilConnected(
        int32_t slotId, const std::shared_ptr<AppExecFwk::EventHandler> &handler, int what, void *obj);
    static void UnRegisterRilConnected(int32_t slotId, int what);
    static void PdpContextListUpdated(
        int32_t slotId, const std::shared_ptr<AppExecFwk::EventHandler> &handler, int what, void *obj);
    static void UnPdpContextListUpdated(int32_t slotId, int what);
    static void GetPreferredNetworkPara(int32_t slotId, const AppExecFwk::InnerEvent::Pointer &response);
    static void SetPreferredNetworkPara(
        int32_t slotId, int32_t preferredNetworkType, const AppExecFwk::InnerEvent::Pointer &response);
    static void GetLinkBandwidthInfo(int32_t slotId, int32_t cid, const AppExecFwk::InnerEvent::Pointer &response);

private:
    RilAdapterUtils() = default;
    ~RilAdapterUtils() = default;
};
} // namespace Telephony
} // namespace OHOS
#endif // RIL_ADAPTER_UTILS_H
