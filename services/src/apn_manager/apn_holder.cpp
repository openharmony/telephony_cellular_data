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

#include "apn_holder.h"

#include "cellular_data_event_code.h"
#include "cellular_data_state_machine.h"
#include "data_disconnect_params.h"

namespace OHOS {
namespace Telephony {
const std::map<std::string, int32_t> ApnHolder::apnTypeDataProfileMap_ {
    {DATA_CONTEXT_ROLE_DEFAULT, DATA_PROFILE_DEFAULT},
    {DATA_CONTEXT_ROLE_MMS,     DATA_PROFILE_MMS}
};

ApnHolder::ApnHolder(const std::string &apnType, const int32_t priority) : apnType_(apnType), priority_(priority) {}

ApnHolder::~ApnHolder() = default;

sptr<ApnItem> ApnHolder::GetNextRetryApn() const
{
    return retryPolicy_.GetNextRetryApnItem();
}

void ApnHolder::SetAllMatchedApns(std::vector<sptr<ApnItem>> &matchedApns)
{
    retryPolicy_.SetMatchedApns(matchedApns);
}

int64_t ApnHolder::GetRetryDelay() const
{
    return retryPolicy_.GetNextRetryDelay();
}

void ApnHolder::SetCurrentApn(sptr<ApnItem> &apnItem)
{
    apnItem_ = apnItem;
}

sptr<ApnItem> ApnHolder::GetCurrentApn() const
{
    return apnItem_;
}

void ApnHolder::SetApnState(ApnProfileState state)
{
    if (apnState_ != state) {
        apnState_ = state;
    }
    if (apnState_ == PROFILE_STATE_FAILED) {
        retryPolicy_.ClearRetryApns();
    }
}

ApnProfileState ApnHolder::GetApnState() const
{
    return apnState_;
}

bool ApnHolder::IsDataCallEnabled() const
{
    return dataCallEnabled_;
}

bool ApnHolder::IsDataCallConnectable() const
{
    return dataCallEnabled_ && ((apnState_ == PROFILE_STATE_IDLE)
        || (apnState_ == PROFILE_STATE_RETRYING) || (apnState_ == PROFILE_STATE_FAILED));
}

std::string ApnHolder::GetApnType() const
{
    return apnType_;
}

void ApnHolder::ReleaseDataConnection()
{
    if (cellularDataStateMachine_ == nullptr) {
        TELEPHONY_LOGE("cellularDataStateMachine_ is null");
        return;
    }
    std::unique_ptr<DataDisconnectParams> object =
        std::make_unique<DataDisconnectParams>(apnType_, DisConnectionReason::REASON_CLEAR_CONNECTION);
    if (object == nullptr) {
        TELEPHONY_LOGE("ClearConnection fail, object is null");
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, object);
    cellularDataStateMachine_->SendEvent(event);
    apnState_ = PROFILE_STATE_IDLE;
    cellularDataStateMachine_ = nullptr;
}

int32_t ApnHolder::GetProfileId(const std::string &apnType) const
{
    std::map<std::string, int32_t>::const_iterator it = apnTypeDataProfileMap_.find(apnType);
    if (it != apnTypeDataProfileMap_.end()) {
        return it->second;
    }
    TELEPHONY_LOGI("this apnType is not in apnTypeDataProfileMap.");
    return DATA_PROFILE_DEFAULT;
}

void ApnHolder::SetCellularDataStateMachine(const std::shared_ptr<CellularDataStateMachine> &stateMachine)
{
    cellularDataStateMachine_ = stateMachine;
}

std::shared_ptr<CellularDataStateMachine> ApnHolder::GetCellularDataStateMachine() const
{
    return cellularDataStateMachine_;
}

uint32_t ApnHolder::GetCapability() const
{
    return capability_;
}

int32_t ApnHolder::GetPriority() const
{
    return priority_;
}

void ApnHolder::RequestCellularData(const NetRequest &netRequest)
{
    for (const NetRequest &request : netRequests_) {
        if ((netRequest.capability == request.capability) && (netRequest.ident == request.ident)) {
            return;
        }
    }
    netRequests_.push_back(netRequest);
    capability_ = netRequest.capability;
    dataCallEnabled_ = true;
}

void ApnHolder::ReleaseCellularData(const NetRequest &netRequest)
{
    for (std::vector<NetRequest>::const_iterator it = netRequests_.begin(); it != netRequests_.end(); it++) {
        if ((netRequest.capability == it->capability) && (netRequest.ident == it->ident)) {
            netRequests_.erase(it);
            if (netRequests_.empty()) {
                dataCallEnabled_ = false;
            }
            return;
        }
    }
}

bool ApnHolder::IsEmergencyType() const
{
    return apnType_ == DATA_CONTEXT_ROLE_EMERGENCY;
}

void ApnHolder::InitialApnRetryCount()
{
    retryPolicy_.InitialRetryCountValue();
}
} // namespace Telephony
} // namespace OHOS