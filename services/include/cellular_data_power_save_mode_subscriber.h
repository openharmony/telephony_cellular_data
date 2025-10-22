/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef DATA_CONNECTION_POWER_STR_MANAGER_H
#define DATA_CONNECTION_POWER_STR_MANAGER_H

#include "common_event_subscriber.h"

namespace OHOS {
namespace Telephony {
class CellularDataHandler;
enum class PowerSaveModeScenario {
    ENTERING = 1,
    EXITING = 2,
    ENTERING_TIMEOUT = 3,
    EXITING_TIMEOUT = 4,
};
class CellularDataPowerSaveModeSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit CellularDataPowerSaveModeSubscriber(
        const EventFwk::CommonEventSubscribeInfo &info, std::weak_ptr<CellularDataHandler> &handler)
        : CommonEventSubscriber(info), powerSaveModeCellularDataHandler_(handler) {}
    ~CellularDataPowerSaveModeSubscriber() = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    bool FinishTelePowerCommonEvent();
private:
    void OnHandleEnterStrEvent(std::string &action);
    void OnHandleExitStrEvent(std::string &action);
    std::shared_ptr<EventFwk::AsyncCommonEventResult> strAsyncCommonEvent_ = nullptr;
    std::weak_ptr<CellularDataHandler> powerSaveModeCellularDataHandler_;
    static inline std::string lastMsg_ = "";
    static inline bool savedCellularDataStatus_ = true;
};
}  // namespace Telephony
}  // namespace OHOS
#endif
