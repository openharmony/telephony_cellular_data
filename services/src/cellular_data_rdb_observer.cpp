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

#include "cellular_data_rdb_observer.h"

#include "cellular_data_event_code.h"
#include "inner_event.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;

CellularDataRdbObserver::CellularDataRdbObserver(std::weak_ptr<TelEventHandler> &&cellularDataHandler)
    : cellularDataHandler_(std::move(cellularDataHandler))
{}

CellularDataRdbObserver::~CellularDataRdbObserver() = default;

void CellularDataRdbObserver::OnChange()
{
    TELEPHONY_LOGI("OnChange");
    auto cellularDataHandler = cellularDataHandler_.lock();
    if (cellularDataHandler == nullptr) {
        TELEPHONY_LOGE("cellularDataHandler is null");
        return;
    }
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_APN_CHANGED);
    cellularDataHandler->SendEvent(event);
}
} // namespace Telephony
} // namespace OHOS