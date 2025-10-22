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
void CellularDataPowerSaveModeSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    strAsyncCommonEvent_ = this->GoAsyncCommonEvent();
    if (action == ENTER_STR_TELEPHONY_NOTIFY) {
        TELEPHONY_LOGI("Enter str mode");
        OnHandleEnterStrEvent(action);
    } else if (action == EXIT_STR_TELEPHONY_NOTIFY) {
        TELEPHONY_LOGI("Exit str mode");
        int32_t reason = data.GetCode();
        // process only in user operation
        if (reason == USER_OPERATION) {
            OnHandleExitStrEvent(action);
        } else {
            FinishTelePowerCommonEvent();
        }
    }
}

void CellularDataPowerSaveModeSubscriber::OnHandleEnterStrEvent(std::string &action)
{
    // except same msg
    if (action != lastMsg_) {
        auto powerSaveModeCellularDataHandler = powerSaveModeCellularDataHandler_.lock();
        if (powerSaveModeCellularDataHandler != nullptr) {
            powerSaveModeCellularDataHandler->ReplyCommonEventScenario(PowerSaveModeScenario::ENTERING_TIMEOUT);
            powerSaveModeCellularDataHandler->ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
            int32_t ret = powerSaveModeCellularDataHandler->IsCellularDataEnabled(savedCellularDataStatus_);
            TELEPHONY_LOGI("Backup cellular status = %{public}d, ret = %{public}d", savedCellularDataStatus_, ret);
            powerSaveModeCellularDataHandler->SetCellularDataEnable(false);
        }
    } else {
        FinishTelePowerCommonEvent();
        TELEPHONY_LOGE("Recv same msg");
    }
    lastMsg_ = ENTER_STR_TELEPHONY_NOTIFY;
}

void CellularDataPowerSaveModeSubscriber::OnHandleExitStrEvent(std::string &action)
{
    if (action != lastMsg_) {
        auto powerSaveModeCellularDataHandler = powerSaveModeCellularDataHandler_.lock();
        if (powerSaveModeCellularDataHandler != nullptr) {
            powerSaveModeCellularDataHandler->ReplyCommonEventScenario(PowerSaveModeScenario::EXITING_TIMEOUT);
            powerSaveModeCellularDataHandler->EstablishAllApnsIfConnectable();
            int32_t ret = powerSaveModeCellularDataHandler->SetCellularDataEnable(savedCellularDataStatus_);
            TELEPHONY_LOGI("Resume cellular status %{public}d, ret = %{public}d", savedCellularDataStatus_, ret);
        }
    } else {
        FinishTelePowerCommonEvent();
        TELEPHONY_LOGE("Recv same msg");
    }
    lastMsg_ = EXIT_STR_TELEPHONY_NOTIFY;
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
