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
 
#include "cellular_data_power_save_mode_subscriber.h"

#include "telephony_types.h"
#include "telephony_log_wrapper.h"
#include "cellular_data_handler.h"

namespace OHOS {
namespace Telephony {
constexpr int32_t USER_OPERATION = 1;
constexpr int64_t MSG_POWER_SAVE_MODE_TIMEOUT_DELAY_TIME = 3 * 1000;
void CellularDataPowerSaveModeSubscriber::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Event is null!");
        return;
    }
    uint32_t eventCode = event->GetInnerEventId();
    TELEPHONY_LOGI("Recv CellularDataHandler eventCode: %{public}u", eventCode);
    switch(eventCode) {
        case PowerSaveModeEvent::MSG_ENTER_POWER_SAVE_MODE_COMPLETE:
            FinishTelePowerCommonEvent();
            break;
        case PowerSaveModeEvent::MSG_EXIT_POWER_SAVE_MODE_COMPLETE:
            FinishTelePowerCommonEvent();
            break;
        case PowerSaveModeEvent::MSG_POWER_SAVE_MODE_TIMEOUT:
            FinishTelePowerCommonEvent();
            break;
        default:
            break;
    }
    RemoveEvent(PowerSaveModeEvent::MSG_POWER_SAVE_MODE_TIMEOUT);
}

void CellularDataPowerSaveModeSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    strAsyncCommonEvent_ = this->GoAsyncCommonEvent();
    if (action == ENTER_STR_TELEPHONY_NOTIFY) {
        TELEPHONY_LOGI("Enter str mode");
        HandleEnterStrEvent(action);
    } else if (action == EXIT_STR_TELEPHONY_NOTIFY) {
        TELEPHONY_LOGI("Exit str mode");
        int32_t reason = data.GetCode();
        // process only in user operation
        if (reason == USER_OPERATION) {
            HandleExitStrEvent(action);
        }
    }
    FinishTelePowerCommonEvent();
}

void CellularDataPowerSaveModeSubscriber::HandleEnterStrEvent(std::string &action)
{
    // except same msg
    if (action != lastMsg_ && savedCellularDataStatus_) {
        auto event = AppExecFwk::InnerEvent::Get(PowerSaveModeEvent::MSG_POWER_SAVE_MODE_TIMEOUT);
        SendEvent(event, MSG_POWER_SAVE_MODE_TIMEOUT_DELAY_TIME);
        auto powerSaveModeCellularDataHandler = powerSaveModeCellularDataHandler_.lock();
        if (powerSaveModeCellularDataHandler != nullptr) {
            powerSaveModeCellularDataHandler->ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
            int32_t ret = powerSaveModeCellularDataHandler->IsCellularDataEnabled(savedCellularDataStatus_);
            TELEPHONY_LOGI("Backup cellular status = %{public}d, ret = %{public}d", savedCellularDataStatus_, ret);
            powerSaveModeCellularDataHandler->SetCellularDataEnable(false);
        }
    } else {
        FinishTelePowerCommonEvent();  // 不去激活，没有超时消息
        TELEPHONY_LOGI("No need to deactive data call");
    }
    lastMsg = ENTER_STR_TELEPHONY_NOTIFY;
}

void CellularDataPowerSaveModeSubscriber::HandleExitStrEvent(std::string &action)
{
    if (action != lastMsg_ && savedCellularDataStatus_) {
        auto event = AppExecFwk::InnerEvent::Get(PowerSaveModeEvent::MSG_POWER_SAVE_MODE_TIMEOUT);
        SendEvent(event, MSG_POWER_SAVE_MODE_TIMEOUT_DELAY_TIME);
        auto powerSaveModeCellularDataHandler = powerSaveModeCellularDataHandler_.lock();
        if (powerSaveModeCellularDataHandler != nullptr) {
            int32_t ret = powerSaveModeCellularDataHandler->SetCellularDataEnable(savedCellularDataStatus_);
            TELEPHONY_LOGI("Resume cellular status %{public}d, ret = %{public}d", savedCellularDataStatus_, ret);
        }
    } else {
        FinishTelePowerCommonEvent();  // 不激活，没有超时消息
        TELEPHONY_LOGI("No need to active data call");
    }
    lastMsg = EXIT_STR_TELEPHONY_NOTIFY;
}

bool CellularDataPowerSaveModeSubscriber::FinishTelePowerCommonEvent()
{
    bool replyRet = false;
    if (strAsyncCommonEvent_ != nullptr) {
        replyRet = strAsyncCommonEvent_->FinishCommonEvent();
        TELEPHONY_LOGI("FinishTelePowerCommonEvent replyRet = %{public}d", replyRet);
    } else {
        TELEPHONY_LOGE("strAsyncCommonEvent_ is nullptr");
    }
    return replyRet;
}
}  // namespace Telephony
}  // namespace OHOS
