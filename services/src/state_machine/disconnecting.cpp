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

#include "disconnecting.h"

#include "hril_data_parcel.h"
#include "telephony_log_wrapper.h"
#include "radio_event.h"

#include "cellular_data_event_code.h"
#include "inactive.h"

namespace OHOS {
namespace Telephony {
void Disconnecting::StateBegin()
{
    TELEPHONY_LOGI("Enter disconnecting state");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(sptr<State>(this));
}

void Disconnecting::StateEnd()
{
    TELEPHONY_LOGI("Disconnecting::exit");
    isActive_ = false;
}

void Disconnecting::ProcessDisconnectTimeout(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return;
    }
    int32_t connectId = event->GetParam();
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    if (connectId != stateMachine->connectId_) {
        return;
    }
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
    inActive->SetReason(DisConnectionReason::REASON_CLEAR_CONNECTION);
    stateMachine->TransitionTo(stateMachine->inActiveState_);
    TELEPHONY_LOGI("ProcessDisconnectTimeout");
}

bool Disconnecting::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    bool retVal = false;
    uint32_t eventCode = event->GetInnerEventId();
    switch (eventCode) {
        case CellularDataEventCode::MSG_SM_CONNECT:
            TELEPHONY_LOGI("Disconnecting::MSG_SM_CONNECT");
            stateMachine->DeferEvent(std::move(event));
            retVal = PROCESSED;
            break;
        case RadioEvent::RADIO_RIL_DEACTIVATE_DATA_CALL: {
            Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
            std::shared_ptr<HRilRadioResponseInfo> rilInfo = event->GetSharedObject<HRilRadioResponseInfo>();
            if (rilInfo == nullptr) {
                TELEPHONY_LOGE("SetupDataCallResultInfo and HRilRadioResponseInfo is null");
                stateMachine->stateMachineEventHandler_->RemoveEvent(CellularDataEventCode::MSG_CONNECT_TIMEOUT_CHECK);
                inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
                stateMachine->TransitionTo(stateMachine->inActiveState_);
                retVal = PROCESSED;
                break;
            }
            if (stateMachine->connectId_ != rilInfo->flag) {
                TELEPHONY_LOGE("connectId is %{public}d, flag is %{public}d", stateMachine->connectId_, rilInfo->flag);
                retVal = PROCESSED;
                break;
            }
            TELEPHONY_LOGI("HRilRadioResponseInfo error is %{public}d", static_cast<int32_t>(rilInfo->error));
            stateMachine->stateMachineEventHandler_->RemoveEvent(CellularDataEventCode::MSG_CONNECT_TIMEOUT_CHECK);
            inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
            stateMachine->TransitionTo(stateMachine->inActiveState_);
            retVal = PROCESSED;
            break;
        }
        case CellularDataEventCode::MSG_DISCONNECT_TIMEOUT_CHECK:
            ProcessDisconnectTimeout(event);
            retVal = PROCESSED;
            break;
        default:
            TELEPHONY_LOGE("disconnecting StateProcess do nothing!");
            break;
    }
    return retVal;
}
} // namespace Telephony
} // namespace OHOS
