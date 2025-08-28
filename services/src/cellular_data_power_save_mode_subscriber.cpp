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
void CellularDataPowerSaveModeSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    strAsyncCommonEvent_ = this->GoAsyncCommonEvent();
    if (action == ENTER_STR_TELEPHONY_NOTIFY) {
        TELEPHONY_LOGI("Enter str mode");
        HandleEnterStrEvent(action);
    } else if (action == EXIT_STR_TELEPHONY_NOTIFY) {
        TELEPHONY_LOGI("Exit str mode");
        HandleExitStrEvent(action);
    }
}

void CellularDataPowerSaveModeSubscriber::HandleEnterStrEvent(std::string &action)
{
    // except same msg
    if (action != lastMsg) {
        auto powerSaveModeCellularDataHandler = powerSaveModeCellularDataHandler_.lock();
        if (powerSaveModeCellularDataHandler != nullptr) {
            int32_t ret = powerSaveModeCellularDataHandler->IsCellularDataEnabled(savedCellularDataStatus_);
            TELEPHONY_LOGI("Backup cellular status = %{public}d, ret = %{public}d", savedCellularDataStatus_, ret);
            // ensure HandleDisconnectDataCompleteForMmsType reply event only in enter str mode 
            powerSaveFlag_ = true;
            powerSaveModeCellularDataHandler->SetCellularDataEnable(false);
        }
    } else {
        TELEPHONY_LOGI("Recv same msg in succession lastMsg:%{public}s", lastMsg.c_str());
        FinishTelePowerEvent();
    }
    lastMsg = ENTER_STR_TELEPHONY_NOTIFY;
}

void CellularDataPowerSaveModeSubscriber::HandleExitStrEvent(std::string &action)
{
    // reply common event firt due to activate cellular too slow
    FinishTelePowerEvent();
    if (action != lastMsg) {
        auto powerSaveModeCellularDataHandler = powerSaveModeCellularDataHandler_.lock();
        if (powerSaveModeCellularDataHandler != nullptr) {
            int32_t ret = powerSaveModeCellularDataHandler->SetCellularDataEnable(savedCellularDataStatus_);
            TELEPHONY_LOGI("Resume cellular status %{public}d, ret = %{public}d",savedCellularDataStatus_, ret);
        }
    } else {
        TELEPHONY_LOGI("Recv same msg in succession lastMsg:%{public}s", lastMsg.c_str());
    }
    lastMsg = EXIT_STR_TELEPHONY_NOTIFY;
}

bool CellularDataPowerSaveModeSubscriber::FinishTelePowerEvent()
{
    bool replyRet = false;
    if (strAsyncCommonEvent_ != nullptr) {
        replyRet = strAsyncCommonEvent_->FinishCommonEvent();
        TELEPHONY_LOGI("FinishTelePowerEvent replyRet = %{public}d", replyRet);
    } else {
        TELEPHONY_LOGE("strAsyncCommonEvent_ is nullptr");
    }
    return replyRet;
}
}  // namespace Telephony
}  // namespace OHOS