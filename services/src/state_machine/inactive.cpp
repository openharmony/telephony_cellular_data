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

#include "inactive.h"

#include "telephony_log_wrapper.h"

#include "apn_manager.h"
#include "cellular_data_event_code.h"

namespace OHOS {
namespace Telephony {
void Inactive::StateBegin()
{
    TELEPHONY_LOGI("Enter inactive state");
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    stateMachine->connectId_++;
    isActive_ = true;
    if (deActiveApnTypeId_ != ERROR_APN_ID) {
        // set net manager connection false
        if (stateMachine->netSupplierInfo_ != nullptr) {
            stateMachine->netSupplierInfo_->isAvailable_ = false;
        }
        // send MSG_DISCONNECT_DATA_COMPLETE to CellularDataHandler
        auto netInfo = std::make_shared<SetupDataCallResultInfo>();
        if (netInfo == nullptr) {
            TELEPHONY_LOGE("Create data disconnect params failed");
            return;
        }
        netInfo->cid = stateMachine->cid_;
        netInfo->flag = deActiveApnTypeId_;
        netInfo->active = 0;
        if (resultInfo_ != nullptr) {
            netInfo->reason = resultInfo_->reason;
            netInfo->retryTime = resultInfo_->retryTime;
            netInfo->retryScene = resultInfo_->retryScene;
        }
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_DISCONNECT_DATA_COMPLETE, netInfo);
        if (event == nullptr) {
            TELEPHONY_LOGE("Create event failed");
            return;
        }
        if (stateMachine->cellularDataHandler_ != nullptr) {
            stateMachine->cellularDataHandler_->SendEvent(event);
        }
        deActiveApnTypeId_ = ERROR_APN_ID;
        resultInfo_ = nullptr;
    }
    stateMachine->SetCurrentState(sptr<State>(this));
}

void Inactive::StateEnd()
{
    TELEPHONY_LOGI("Exit inactive state");
    isActive_ = false;
}

bool Inactive::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
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
        case CellularDataEventCode::MSG_SM_CONNECT: {
            TELEPHONY_LOGD("Inactive::MSG_SM_CONNECT");
            std::unique_ptr<DataConnectionParams> params = event->GetUniqueObject<DataConnectionParams>();
            if (params == nullptr) {
                TELEPHONY_LOGE("Failed to get DataConnectionParams");
                return false;
            }
            stateMachine->DoConnect(*params);
            stateMachine->TransitionTo(stateMachine->activatingState_);
            retVal = PROCESSED;
            break;
        }
        case CellularDataEventCode::MSG_SM_DISCONNECT: {
            TELEPHONY_LOGI("Inactive::MSG_SM_DISCONNECT");
            retVal = PROCESSED;
            break;
        }
        case CellularDataEventCode::MSG_SM_DISCONNECT_ALL: {
            TELEPHONY_LOGI("Inactive::MSG_SM_DISCONNECT_ALL");
            retVal = PROCESSED;
            break;
        }
        default:
            break;
    }
    return retVal;
}

void Inactive::SetStateMachine(const std::weak_ptr<CellularDataStateMachine> &stateMachine)
{
    stateMachine_ = stateMachine;
}

void Inactive::SetDeActiveApnTypeId(int32_t deActiveApnTypeId)
{
    deActiveApnTypeId_ = deActiveApnTypeId;
}

void Inactive::SetDataCallResultInfo(std::shared_ptr<SetupDataCallResultInfo> resultInfo)
{
    resultInfo_ = resultInfo;
}

void Inactive::SetPdpErrorReason(PdpErrorReason reason)
{
    resultInfo_ = std::make_shared<SetupDataCallResultInfo>();
    resultInfo_->reason = static_cast<int32_t>(reason);
}
} // namespace Telephony
} // namespace OHOS
