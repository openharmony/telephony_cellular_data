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
#include "hril_data_parcel.h"
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
        TELEPHONY_LOGE("result info is null");
        return RilErrorResponse(event);
    }
    TELEPHONY_LOGI("callDone active: %{public}d flag: %{public}d, cid: %{public}d, reason: %{public}d",
        resultInfo->active, resultInfo->flag, resultInfo->cid, resultInfo->reason);
    if (stateMachine->connectId_ != resultInfo->flag) {
        TELEPHONY_LOGE("connectId is %{public}d, flag is %{public}d", stateMachine->connectId_, resultInfo->flag);
        return false;
    }
    if (resultInfo->reason != 0) {
        Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
        DisConnectionReason disReason = DataCallPdpError(resultInfo->reason);
        inActive->SetReason(disReason);
        inActive->SetDeActiveApnTypeId(stateMachine->apnId_);
        stateMachine->TransitionTo(stateMachine->inActiveState_);
        return true;
    }
    if (resultInfo->active == 0) {
        Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
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
        case HRilPdpErrorReason::HRIL_PDP_ERR_RETRY:
        case HRilPdpErrorReason::HRIL_PDP_ERR_UNKNOWN:
        case HRilPdpErrorReason::HRIL_PDP_ERR_SHORTAGE_RESOURCES:
        case HRilPdpErrorReason::HRIL_PDP_ERR_ACTIVATION_REJECTED_UNSPECIFIED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER:
        case HRilPdpErrorReason::HRIL_PDP_ERR_APN_NOT_SUPPORTED_IN_CURRENT_RAT_PLMN:
        case HRilPdpErrorReason::HRIL_PDP_ERR_APN_RESTRICTION_VALUE_INCOMPATIBLE: {
            TELEPHONY_LOGE("DataCall: The connection failed, try again");
            return DisConnectionReason::REASON_RETRY_CONNECTION;
        }
        case HRilPdpErrorReason::HRIL_PDP_ERR_MULT_ACCESSES_PDN_NOT_ALLOWED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_OPERATOR_DETERMINED_BARRING:
        case HRilPdpErrorReason::HRIL_PDP_ERR_MISSING_OR_UNKNOWN_APN:
        case HRilPdpErrorReason::HRIL_PDP_ERR_UNKNOWN_PDP_ADDR_OR_TYPE:
        case HRilPdpErrorReason::HRIL_PDP_ERR_USER_VERIFICATION:
        case HRilPdpErrorReason::HRIL_PDP_ERR_ACTIVATION_REJECTED_GGSN:
        case HRilPdpErrorReason::HRIL_PDP_ERR_SERVICE_OPTION_NOT_SUPPORTED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_NSAPI_ALREADY_USED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_IPV4_ONLY_ALLOWED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_IPV6_ONLY_ALLOWED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_IPV4V6_ONLY_ALLOWED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_NON_IP_ONLY_ALLOWED:
        case HRilPdpErrorReason::HRIL_PDP_ERR_MAX_NUM_OF_PDP_CONTEXTS:
        case HRilPdpErrorReason::HRIL_PDP_ERR_PROTOCOL_ERRORS: {
            TELEPHONY_LOGE("DataCall: The connection failed, not try again");
            return DisConnectionReason::REASON_CLEAR_CONNECTION;
        }
        default: {
            if (reason > HRIL_PDP_ERR_PROTOCOL_ERRORS && reason < HRIL_PDP_ERR_APN_RESTRICTION_VALUE_INCOMPATIBLE) {
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
    std::shared_ptr<HRilRadioResponseInfo> rilInfo = event->GetSharedObject<HRilRadioResponseInfo>();
    if (rilInfo == nullptr) {
        TELEPHONY_LOGE("SetupDataCallResultInfo and HRilRadioResponseInfo is null");
        return false;
    }
    if (stateMachine->connectId_ != rilInfo->flag) {
        TELEPHONY_LOGE("connectId is %{public}d, flag is %{public}d", stateMachine->connectId_, rilInfo->flag);
        return false;
    }
    TELEPHONY_LOGI("HRilRadioResponseInfo flag:%{public}d error:%{public}d", rilInfo->flag, rilInfo->error);
    Inactive *inActive = static_cast<Inactive *>(stateMachine->inActiveState_.GetRefPtr());
    switch (rilInfo->error) {
        case HRilErrType::HRIL_ERR_GENERIC_FAILURE:
        case HRilErrType::HRIL_ERR_CMD_SEND_FAILURE:
        case HRilErrType::HRIL_ERR_NULL_POINT: {
            inActive->SetReason(DisConnectionReason::REASON_RETRY_CONNECTION);
            CellularDataHiSysEvent::WriteDataActivateFaultEvent(INVALID_PARAMETER, SWITCH_ON,
                CellularDataErrorCode::DATA_ERROR_RADIO_RESPONSEINFO_ERROR,
                "HRilErrType " + std::to_string(static_cast<int32_t>(rilInfo->error)));
            TELEPHONY_LOGI("Handle supported error responses and retry the connection.");
            break;
        }
        case HRilErrType::HRIL_ERR_INVALID_RESPONSE:
        case HRilErrType::HRIL_ERR_CMD_NO_CARRIER:
        case HRilErrType::HRIL_ERR_HDF_IPC_FAILURE:
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