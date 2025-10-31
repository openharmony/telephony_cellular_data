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

#include "activating.h"

#include "cellular_data_hisysevent.h"
#include "inactive.h"
#include "radio_event.h"
#include "apn_manager.h"

namespace OHOS {
namespace Telephony {
static const int32_t MAX_RIL_ERR_RETRY_COUNT = 3;

void Activating::StateBegin()
{
    TELEPHONY_LOGI("Enter activating state");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(sptr<State>(this));
}

void Activating::StateEnd()
{
    TELEPHONY_LOGI("Exit activating state");
    isActive_ = false;
}

bool Activating::RilActivatePdpContextDone(const AppExecFwk::InnerEvent::Pointer &event)
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
    std::shared_ptr<SetupDataCallResultInfo> resultInfo = event->GetSharedObject<SetupDataCallResultInfo>();
    if (resultInfo == nullptr) {
        TELEPHONY_LOGI("resultInfo null, goto RilErrorResponse");
        return RilErrorResponse(event);
    }
    rilErrTryCount_ = 0;
    TELEPHONY_LOGI("callDone active: %{public}d flag: %{public}d, cid: %{public}d, reason: %{public}d",
        resultInfo->active, resultInfo->flag, resultInfo->cid, resultInfo->reason);
    if (stateMachine->connectId_ != resultInfo->flag) {
        TELEPHONY_LOGE("connectId is %{public}d, flag is %{public}d", stateMachine->connectId_, resultInfo->flag);
        return false;
    }
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    if (inActive == nullptr) {
        TELEPHONY_LOGE("Inactive is null");
        return false;
    }
    stateMachine->SetCid(resultInfo->cid);
    if (resultInfo->reason != 0 || resultInfo->active == 0) {
        resultInfo->retryScene = static_cast<int32_t>(RetryScene::RETRY_SCENE_SETUP_DATA);
        inActive->SetDataCallResultInfo(resultInfo);
        inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
        stateMachine->TransitionTo(stateMachine->inActiveState_);
        return true;
    }
    if (stateMachine->cdConnectionManager_ != nullptr) {
        stateMachine->cdConnectionManager_->AddActiveConnectionByCid(stateMachine_.lock());
    } else {
        TELEPHONY_LOGE("cdConnectionManager is null");
    }
    stateMachine->DeferEvent(std::move(event));
    stateMachine->TransitionTo(stateMachine->activeState_);
    return true;
}

bool Activating::RilErrorResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    std::shared_ptr<RadioResponseInfo> rilInfo = event->GetSharedObject<RadioResponseInfo>();
    if (rilInfo == nullptr) {
        TELEPHONY_LOGE("SetupDataCallResultInfo and RadioResponseInfo is null");
        return false;
    }
    if (stateMachine->connectId_ != rilInfo->flag) {
        TELEPHONY_LOGE("connectId is %{public}d, flag is %{public}d", stateMachine->connectId_, rilInfo->flag);
        return false;
    }
    TELEPHONY_LOGI("RadioResponseInfo flag:%{public}d error:%{public}d", rilInfo->flag, rilInfo->error);
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    if (inActive == nullptr) {
        TELEPHONY_LOGE("Inactive is null");
        return false;
    }
    ++rilErrTryCount_;
    if (rilErrTryCount_ > MAX_RIL_ERR_RETRY_COUNT) {
        rilErrTryCount_ = 0;
        inActive->SetPdpErrorReason(PdpErrorReason::PDP_ERR_TO_CLEAR_CONNECTION);
    } else {
        inActive->SetPdpErrorReason(PdpErrorReason::PDP_ERR_RETRY);
    }
    CellularDataHiSysEvent::WriteDataActivateFaultEvent(INVALID_PARAMETER, SWITCH_ON,
        CellularDataErrorCode::DATA_ERROR_RADIO_RESPONSEINFO_ERROR,
        "ErrType " + std::to_string(static_cast<int32_t>(rilInfo->error)));
    inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
    stateMachine->TransitionTo(stateMachine->inActiveState_);
    return true;
}

void Activating::ProcessConnectTimeout(const AppExecFwk::InnerEvent::Pointer &event)
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
    int64_t currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    if ((currentTime - stateMachine->startTimeConnectTimeoutTask_) < CONNECTION_TASK_TIME) {
        TELEPHONY_LOGE("ProcessConnectTimeout error, delay: %{public}lld",
            static_cast<long long>(currentTime - stateMachine->startTimeConnectTimeoutTask_));
        return;
    }
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    if (inActive == nullptr) {
        TELEPHONY_LOGE("Inactive is null");
        return;
    }
    inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
    inActive->SetPdpErrorReason(PdpErrorReason::PDP_ERR_RETRY);
    stateMachine->TransitionTo(stateMachine->inActiveState_);
    std::string apnType = ApnManager::FindApnNameByApnId(stateMachine->apnId_);
    CellularDataHiSysEvent::WriteDataActivateFaultEvent(stateMachine->GetSlotId(), SWITCH_ON,
        CellularDataErrorCode::DATA_ERROR_DATA_ACTIVATE_TIME_OUT, apnType + " activate time out.");
    TELEPHONY_LOGI("ProcessConnectTimeout");
}

bool Activating::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
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
        case CellularDataEventCode::MSG_SM_DRS_OR_RAT_CHANGED:
            [[fallthrough]];
        case CellularDataEventCode::MSG_SM_CONNECT:
            TELEPHONY_LOGI("Activating::MSG_SM_CONNECT");
            stateMachine->DeferEvent(std::move(event));
            retVal = PROCESSED;
            break;
        case RadioEvent::RADIO_RIL_SETUP_DATA_CALL: {
            retVal = RilActivatePdpContextDone(event);
            break;
        }
        case CellularDataEventCode::MSG_SM_GET_LAST_FAIL_DONE:
            stateMachine->TransitionTo(stateMachine->inActiveState_);
            retVal = PROCESSED;
            break;
        case CellularDataEventCode::MSG_GET_RIL_BANDWIDTH:
            stateMachine->DeferEvent(std::move(event));
            break;
        case CellularDataEventCode::MSG_CONNECT_TIMEOUT_CHECK:
            ProcessConnectTimeout(event);
            retVal = PROCESSED;
            break;
        default:
            TELEPHONY_LOGE("eventCode:%{public}d goto default", eventCode);
            break;
    }
    return retVal;
}
} // namespace Telephony
} // namespace OHOS