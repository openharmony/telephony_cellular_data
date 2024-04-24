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

#include "cellular_data_event_code.h"
#include "cellular_data_hisysevent.h"
#include "tel_ril_data_parcel.h"
#include "inactive.h"
#include "radio_event.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
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
        return RilErrorResponse(event);
    }
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
    if (resultInfo->reason != 0) {
        DisConnectionReason disReason = DataCallPdpError(resultInfo->reason);
        inActive->SetReason(disReason);
        inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
        stateMachine->TransitionTo(stateMachine->inActiveState_);
        return true;
    }
    if (resultInfo->active == 0) {
        inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
        inActive->SetReason(DisConnectionReason::REASON_RETRY_CONNECTION);
        stateMachine->TransitionTo(stateMachine->inActiveState_);
        return true;
    }
    stateMachine->SetCid(resultInfo->cid);
    if (stateMachine->cdConnectionManager_ != nullptr) {
        stateMachine->cdConnectionManager_->AddActiveConnectionByCid(stateMachine_.lock());
    } else {
        TELEPHONY_LOGE("cdConnectionManager is null");
    }
    stateMachine->DeferEvent(std::move(event));
    stateMachine->TransitionTo(stateMachine->activeState_);
    return true;
}

DisConnectionReason Activating::DataCallPdpError(int32_t reason)
{
    switch (reason) {
        case PdpErrorReason::PDP_ERR_RETRY:
        case PdpErrorReason::PDP_ERR_UNKNOWN:
        case PdpErrorReason::PDP_ERR_SHORTAGE_RESOURCES:
        case PdpErrorReason::PDP_ERR_ACTIVATION_REJECTED_UNSPECIFIED:
        case PdpErrorReason::PDP_ERR_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER:
        case PdpErrorReason::PDP_ERR_APN_NOT_SUPPORTED_IN_CURRENT_RAT_PLMN:
        case PdpErrorReason::PDP_ERR_APN_RESTRICTION_VALUE_INCOMPATIBLE: {
            TELEPHONY_LOGE("DataCall: The connection failed, try again");
            return DisConnectionReason::REASON_RETRY_CONNECTION;
        }
        case PdpErrorReason::PDP_ERR_MULT_ACCESSES_PDN_NOT_ALLOWED:
        case PdpErrorReason::PDP_ERR_OPERATOR_DETERMINED_BARRING:
        case PdpErrorReason::PDP_ERR_MISSING_OR_UNKNOWN_APN:
        case PdpErrorReason::PDP_ERR_UNKNOWN_PDP_ADDR_OR_TYPE:
        case PdpErrorReason::PDP_ERR_USER_VERIFICATION:
        case PdpErrorReason::PDP_ERR_ACTIVATION_REJECTED_GGSN:
        case PdpErrorReason::PDP_ERR_SERVICE_OPTION_NOT_SUPPORTED:
        case PdpErrorReason::PDP_ERR_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED:
        case PdpErrorReason::PDP_ERR_NSAPI_ALREADY_USED:
        case PdpErrorReason::PDP_ERR_IPV4_ONLY_ALLOWED:
        case PdpErrorReason::PDP_ERR_IPV6_ONLY_ALLOWED:
        case PdpErrorReason::PDP_ERR_IPV4V6_ONLY_ALLOWED:
        case PdpErrorReason::PDP_ERR_NON_IP_ONLY_ALLOWED:
        case PdpErrorReason::PDP_ERR_MAX_NUM_OF_PDP_CONTEXTS:
        case PdpErrorReason::PDP_ERR_PROTOCOL_ERRORS: {
            TELEPHONY_LOGE("DataCall: The connection failed, not try again");
            return DisConnectionReason::REASON_CLEAR_CONNECTION;
        }
        default: {
            if (reason > PDP_ERR_PROTOCOL_ERRORS && reason < PDP_ERR_APN_RESTRICTION_VALUE_INCOMPATIBLE) {
                TELEPHONY_LOGE("DataCall: The protocol error, not try again");
                return DisConnectionReason::REASON_CLEAR_CONNECTION;
            }
            break;
        }
    }
    TELEPHONY_LOGE("DataCall: Connection failed for an unsupported reason, not try again");
    return DisConnectionReason::REASON_CLEAR_CONNECTION;
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
    switch (rilInfo->error) {
        case ErrType::ERR_GENERIC_FAILURE:
        case ErrType::ERR_CMD_SEND_FAILURE:
        case ErrType::ERR_NULL_POINT: {
            inActive->SetReason(DisConnectionReason::REASON_RETRY_CONNECTION);
            CellularDataHiSysEvent::WriteDataActivateFaultEvent(INVALID_PARAMETER, SWITCH_ON,
                CellularDataErrorCode::DATA_ERROR_RADIO_RESPONSEINFO_ERROR,
                "ErrType " + std::to_string(static_cast<int32_t>(rilInfo->error)));
            TELEPHONY_LOGD("Handle supported error responses and retry the connection.");
            break;
        }
        case ErrType::ERR_INVALID_RESPONSE:
        case ErrType::ERR_CMD_NO_CARRIER:
        case ErrType::ERR_HDF_IPC_FAILURE:
            inActive->SetReason(DisConnectionReason::REASON_CLEAR_CONNECTION);
            TELEPHONY_LOGI("Handle the supported error response and clear the connection.");
            break;
        default: {
            inActive->SetReason(DisConnectionReason::REASON_CLEAR_CONNECTION);
            TELEPHONY_LOGE("Handle the unsupported error response");
            break;
        }
    }
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
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    if (inActive == nullptr) {
        TELEPHONY_LOGE("Inactive is null");
        return;
    }
    inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
    inActive->SetReason(DisConnectionReason::REASON_RETRY_CONNECTION);
    stateMachine->TransitionTo(stateMachine->inActiveState_);
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