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
#include "apn_manager.h"
namespace OHOS {
namespace Telephony {
namespace {
    constexpr int32_t SYSTEM_UID = 1e4;
    std::shared_mutex reqUidsMutex_;
    std::mutex netRequestMutex_;
}
const std::map<std::string, int32_t> ApnHolder::apnTypeDataProfileMap_ {
    {DATA_CONTEXT_ROLE_DEFAULT, DATA_PROFILE_DEFAULT},
    {DATA_CONTEXT_ROLE_MMS, DATA_PROFILE_MMS},
    {DATA_CONTEXT_ROLE_SUPL, DATA_PROFILE_SUPL},
    {DATA_CONTEXT_ROLE_DUN, DATA_PROFILE_DUN},
    {DATA_CONTEXT_ROLE_IA, DATA_PROFILE_IA},
    {DATA_CONTEXT_ROLE_XCAP, DATA_PROFILE_XCAP}
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

int64_t ApnHolder::GetRetryDelay(int32_t cause, int32_t suggestTime, RetryScene scene)
{
    return retryPolicy_.GetNextRetryDelay(apnType_, cause, suggestTime, scene);
}

void ApnHolder::SetCurrentApn(sptr<ApnItem> &apnItem)
{
    apnItem_ = apnItem;
}

sptr<ApnItem> ApnHolder::GetCurrentApn() const
{
    return apnItem_;
}

void ApnHolder::AddUid(uint32_t uid)
{
    std::unique_lock<std::shared_mutex> lock(reqUidsMutex_);
    if (reqUids_.find(uid) != reqUids_.end()) {
        return;
    }
    reqUids_.insert(uid);
}

void ApnHolder::RemoveUid(uint32_t uid)
{
    std::unique_lock<std::shared_mutex> lock(reqUidsMutex_);
    auto it = reqUids_.find(uid);
    if (it != reqUids_.end()) {
        reqUids_.erase(it);
        return;
    }
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
    apnState_ = PROFILE_STATE_DISCONNECTING;
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, object);
    cellularDataStateMachine_->SendEvent(event);
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

uint64_t ApnHolder::GetCapability() const
{
    return capability_;
}

int32_t ApnHolder::GetPriority() const
{
    return priority_;
}

HasSystemUse ApnHolder::GetUidStatus() const
{
    std::unique_lock<std::shared_mutex> lock(reqUidsMutex_);
    for (auto item : reqUids_) {
        if (item < SYSTEM_UID) {
            return HasSystemUse::HAS;
        }
    }
    return HasSystemUse::NOT_HAS;
}

void ApnHolder::RequestCellularData(const NetRequest &netRequest)
{
    std::unique_lock<std::mutex> lock(netRequestMutex_);
    for (const NetRequest &request : netRequests_) {
        if ((netRequest.capability == request.capability) && (netRequest.ident == request.ident)) {
            return;
        }
    }
    netRequests_.push_back(netRequest);
    capability_ = netRequest.capability;
    dataCallEnabled_ = true;
}

bool ApnHolder::ReleaseCellularData(const NetRequest &netRequest)
{
    std::unique_lock<std::mutex> lock(netRequestMutex_);
    for (std::vector<NetRequest>::const_iterator it = netRequests_.begin(); it != netRequests_.end();) {
        if ((netRequest.capability == it->capability) && (netRequest.ident == it->ident)) {
            it = netRequests_.erase(it);
            if (netRequests_.empty()) {
                dataCallEnabled_ = false;
                return true;
            }
        } else {
            it++;
        }
    }
    return false;
}

void ApnHolder::ReleaseAllCellularData()
{
    std::unique_lock<std::mutex> lock(netRequestMutex_);
    TELEPHONY_LOGI("clear all cellular data");
    netRequests_.clear();
    if (netRequests_.empty()) {
        dataCallEnabled_ = false;
    }
}

bool ApnHolder::IsEmergencyType() const
{
    return apnType_ == DATA_CONTEXT_ROLE_EMERGENCY;
}

bool ApnHolder::IsMmsType() const
{
    return apnType_ == DATA_CONTEXT_ROLE_MMS;
}

void ApnHolder::InitialApnRetryCount()
{
    retryPolicy_.InitialRetryCountValue();
}

bool ApnHolder::IsSameMatchedApns(std::vector<sptr<ApnItem>> newMatchedApns, bool roamingState)
{
    std::vector<sptr<ApnItem>> currentMatchedApns = retryPolicy_.GetMatchedApns();
    if (currentMatchedApns.empty() || newMatchedApns.empty()) {
        TELEPHONY_LOGE("newMatchedApns or oldMatchedApns is empty");
        return false;
    }
    if (currentMatchedApns.size() != newMatchedApns.size()) {
        TELEPHONY_LOGI("newMatchedApns and oldMatchedApns are not equal in size");
        return false;
    }
    for (const sptr<ApnItem> &newApnItem : newMatchedApns) {
        bool canHandle = false;
        for (const sptr<ApnItem> &oldApnItem : currentMatchedApns) {
            if (IsSameApnItem(newApnItem, oldApnItem, roamingState)) {
                canHandle = true;
                break;
            }
        }
        if (!canHandle) {
            return false;
        }
    }
    return true;
}

bool ApnHolder::IsSameApnItem(const sptr<ApnItem> &newApnItem,
                              const sptr<ApnItem> &oldApnItem,
                              bool roamingState)
{
    if (newApnItem == nullptr || oldApnItem == nullptr) {
        TELEPHONY_LOGE("newApnItem or oldApnItem is null");
        return false;
    }
    bool isSameProtocol = false;
    if (roamingState) {
        isSameProtocol = std::strcmp(newApnItem->attr_.roamingProtocol_, oldApnItem->attr_.roamingProtocol_) == 0;
    } else {
        isSameProtocol = std::strcmp(newApnItem->attr_.protocol_, oldApnItem->attr_.protocol_) == 0;
    }
    return isSameProtocol && newApnItem->attr_.profileId_ == oldApnItem->attr_.profileId_ &&
        newApnItem->attr_.authType_ == oldApnItem->attr_.authType_ &&
        newApnItem->attr_.isRoamingApn_ == oldApnItem->attr_.isRoamingApn_ &&
        newApnItem->attr_.isEdited_ == oldApnItem->attr_.isEdited_ &&
        std::strcmp(newApnItem->attr_.types_, oldApnItem->attr_.types_) == 0 &&
        std::strcmp(newApnItem->attr_.numeric_, oldApnItem->attr_.numeric_) == 0 &&
        std::strcmp(newApnItem->attr_.apn_, oldApnItem->attr_.apn_) == 0 &&
        std::strcmp(newApnItem->attr_.apnName_, oldApnItem->attr_.apnName_) == 0 &&
        std::strcmp(newApnItem->attr_.user_, oldApnItem->attr_.user_) == 0 &&
        std::strcmp(newApnItem->attr_.password_, oldApnItem->attr_.password_) == 0 &&
        std::strcmp(newApnItem->attr_.homeUrl_, oldApnItem->attr_.homeUrl_) == 0 &&
        std::strcmp(newApnItem->attr_.proxyIpAddress_, oldApnItem->attr_.proxyIpAddress_) == 0 &&
        std::strcmp(newApnItem->attr_.mmsIpAddress_, oldApnItem->attr_.mmsIpAddress_) == 0;
}

bool ApnHolder::IsCompatibleApnItem(const sptr<ApnItem> &newApnItem, const sptr<ApnItem> &oldApnItem,
    bool roamingState)
{
    if (newApnItem == nullptr || oldApnItem == nullptr) {
        TELEPHONY_LOGE("newApnItem or oldApnItem is null");
        return false;
    }
    bool isSameProtocol = false;
    if (roamingState) {
        isSameProtocol = std::strcmp(newApnItem->attr_.roamingProtocol_, oldApnItem->attr_.roamingProtocol_) == 0;
    } else {
        isSameProtocol = std::strcmp(newApnItem->attr_.protocol_, oldApnItem->attr_.protocol_) == 0;
    }
    return isSameProtocol && newApnItem->attr_.profileId_ == oldApnItem->attr_.profileId_ &&
        newApnItem->attr_.authType_ == oldApnItem->attr_.authType_ &&
        std::strcmp(newApnItem->attr_.types_, oldApnItem->attr_.types_) == 0 &&
        std::strcmp(newApnItem->attr_.numeric_, oldApnItem->attr_.numeric_) == 0 &&
        std::strcmp(newApnItem->attr_.apn_, oldApnItem->attr_.apn_) == 0 &&
        std::strcmp(newApnItem->attr_.apnName_, oldApnItem->attr_.apnName_) == 0 &&
        std::strcmp(newApnItem->attr_.user_, oldApnItem->attr_.user_) == 0 &&
        std::strcmp(newApnItem->attr_.password_, oldApnItem->attr_.password_) == 0 &&
        std::strcmp(newApnItem->attr_.proxyIpAddress_, oldApnItem->attr_.proxyIpAddress_) == 0 &&
        std::strcmp(newApnItem->attr_.mmsIpAddress_, oldApnItem->attr_.mmsIpAddress_) == 0;
}

void ApnHolder::SetApnBadState(bool isBad)
{
    if (apnItem_ != nullptr) {
        apnItem_->MarkBadApn(isBad);
    }
}
} // namespace Telephony
} // namespace OHOS