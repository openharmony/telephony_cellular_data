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

#include "apn_manager.h"

#include "net_specifier.h"
#include "telephony_log_wrapper.h"

#include "cellular_data_utils.h"

namespace OHOS {
namespace Telephony {
const std::map<std::string, int32_t> ApnManager::apnIdApnNameMap_ {
    {DATA_CONTEXT_ROLE_ALL,       DATA_CONTEXT_ROLE_ALL_ID},
    {DATA_CONTEXT_ROLE_DEFAULT,   DATA_CONTEXT_ROLE_DEFAULT_ID},
    {DATA_CONTEXT_ROLE_MMS,       DATA_CONTEXT_ROLE_MMS_ID},
    {DATA_CONTEXT_ROLE_SUPL,      DATA_CONTEXT_ROLE_SUPL_ID},
    {DATA_CONTEXT_ROLE_DUN,       DATA_CONTEXT_ROLE_DUN_ID},
    {DATA_CONTEXT_ROLE_IMS,       DATA_CONTEXT_ROLE_IMS_ID},
    {DATA_CONTEXT_ROLE_IA,        DATA_CONTEXT_ROLE_IA_ID},
    {DATA_CONTEXT_ROLE_EMERGENCY, DATA_CONTEXT_ROLE_EMERGENCY_ID}
};

ApnManager::ApnManager() = default;

ApnManager::~ApnManager() = default;

void ApnManager::InitApnHolders()
{
    AddApnHolder(DATA_CONTEXT_ROLE_DEFAULT, static_cast<int32_t>(DataContextPriority::PRIORITY_LOW));
    AddApnHolder(DATA_CONTEXT_ROLE_MMS, static_cast<int32_t>(DataContextPriority::PRIORITY_NORMAL));
}

sptr<ApnHolder> ApnManager::FindApnHolderById(const int32_t id) const
{
    if (apnIdApnHolderMap_.empty()) {
        TELEPHONY_LOGE("apnIdApnHolderMap_ empty");
        return nullptr;
    }
    if (id == DATA_CONTEXT_ROLE_INVALID_ID) {
        TELEPHONY_LOGE("find a invalid capability!");
        return nullptr;
    }
    std::map<int32_t, sptr<ApnHolder>>::const_iterator it = apnIdApnHolderMap_.find(id);
    if (it != apnIdApnHolderMap_.end()) {
        return it->second;
    }
    return nullptr;
}

int32_t ApnManager::FindApnIdByApnName(const std::string &type)
{
    std::map<std::string, int32_t>::const_iterator it = apnIdApnNameMap_.find(type);
    if (it != apnIdApnNameMap_.end()) {
        return it->second;
    }
    TELEPHONY_LOGE("apnType %{public}s is not exist!", type.c_str());
    return DATA_CONTEXT_ROLE_INVALID_ID;
}

std::string ApnManager::FindApnNameByApnId(const int32_t id)
{
    for (std::pair<const std::string, int32_t> it : apnIdApnNameMap_) {
        if (it.second == id) {
            return it.first;
        }
    }
    TELEPHONY_LOGI("apnId %{public}d is not exist!", id);
    return DATA_CONTEXT_ROLE_DEFAULT;
}

int32_t ApnManager::FindApnIdByCapability(const uint64_t capability)
{
    switch (capability) {
        case NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET:
            return DATA_CONTEXT_ROLE_DEFAULT_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_MMS:
            return DATA_CONTEXT_ROLE_MMS_ID;
        default:
            return DATA_CONTEXT_ROLE_INVALID_ID;
    }
}

void ApnManager::AddApnHolder(const std::string &apnType, const int32_t priority)
{
    int32_t apnId = FindApnIdByApnName(apnType);
    if (apnId == DATA_CONTEXT_ROLE_INVALID_ID) {
        TELEPHONY_LOGE("APN INVALID ID");
        return;
    }
    sptr<ApnHolder> apnHolder = std::make_unique<ApnHolder>(apnType, priority).release();
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is null, type: %{public}s", apnType.c_str());
        return;
    }
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    apnHolders_.push_back(apnHolder);
    apnIdApnHolderMap_.insert(std::pair<int32_t, sptr<ApnHolder>>(apnId, apnHolder));
    sortedApnHolders_.emplace_back(apnHolder);
    sort(sortedApnHolders_.begin(), sortedApnHolders_.end(),
        [](const sptr<ApnHolder> &c1, const sptr<ApnHolder> &c2) {
            return c2->GetPriority() - c1->GetPriority();
        });
    TELEPHONY_LOGI("The Apn holder type:%{public}s, size:%{public}zu", apnType.c_str(), sortedApnHolders_.size());
}

sptr<ApnHolder> ApnManager::GetApnHolder(const std::string &apnType) const
{
    int32_t apnId = FindApnIdByApnName(apnType);
    if (DATA_CONTEXT_ROLE_INVALID_ID == apnId) {
        TELEPHONY_LOGE("APN INVALID ID");
        return nullptr;
    }
    std::map<int32_t, sptr<ApnHolder>>::const_iterator it = apnIdApnHolderMap_.find(apnId);
    if (it != apnIdApnHolderMap_.end()) {
        return it->second;
    }
    TELEPHONY_LOGE("ApnManager::GetApnHolder:apnType %{public}s is not exist!", apnType.c_str());
    return nullptr;
}

std::vector<sptr<ApnHolder>> ApnManager::GetAllApnHolder() const
{
    return apnHolders_;
}

std::vector<sptr<ApnHolder>> ApnManager::GetSortApnHolder() const
{
    return sortedApnHolders_;
}

void ApnManager::CreateAllApnItem()
{
    allApnItem_.clear();
    allApnItem_.push_back(ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT));
    allApnItem_.push_back(ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_MMS));
    allApnItem_.push_back(ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_SUPL));
    allApnItem_.push_back(ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DUN));
    allApnItem_.push_back(ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_IMS));
    allApnItem_.push_back(ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_IA));
    allApnItem_.push_back(ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_EMERGENCY));
}

int32_t ApnManager::CreateAllApnItemByDatabase(const std::string &numeric)
{
    int32_t count = 0;
    if (numeric.empty()) {
        TELEPHONY_LOGE("numeric is empty!!!");
        return count;
    }
    allApnItem_.clear();
    std::string mcc = numeric.substr(0, DEFAULT_MCC_SIZE);
    std::string mnc = numeric.substr(mcc.size(), numeric.size() - mcc.size());
    TELEPHONY_LOGI("mcc = %{public}s, mnc = %{public}s", mcc.c_str(), mnc.c_str());
    std::vector<PdpProfile> apnVec;
    auto helper = CellularDataRdbHelper::GetInstance();
    if (helper == nullptr) {
        TELEPHONY_LOGE("get cellularDataRdbHelper failed");
        return count;
    }
    if (!helper->QueryApns(mcc, mnc, apnVec)) {
        TELEPHONY_LOGE("query apns from data ability fail");
        return count;
    }
    for (const PdpProfile &apnData : apnVec) {
        TELEPHONY_LOGI("profileId = %{public}d, profileName = %{public}s",
            apnData.profileId, apnData.profileName.c_str());
        sptr<ApnItem> apnItem = ApnItem::MakeApn(apnData);
        if (apnItem != nullptr) {
            allApnItem_.push_back(apnItem);
            count++;
        }
    }
    return count;
}

std::vector<sptr<ApnItem>> ApnManager::FilterMatchedApns(const std::string &requestApnType) const
{
    std::vector<sptr<ApnItem>> matchApnItemList;
    for (const sptr<ApnItem> &apnItem : allApnItem_) {
        if (apnItem->CanDealWithType(requestApnType)) {
            matchApnItemList.push_back(apnItem);
        }
    }
    TELEPHONY_LOGI("FilterMatchedApns,apn size is :%{public}zu", matchApnItemList.size());
    return matchApnItemList;
}

bool ApnManager::IsDataConnectionNotUsed(const std::shared_ptr<CellularDataStateMachine> &stateMachine) const
{
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("CellularDataHandler:stateMachine is null");
        return false;
    }
    for (const sptr<ApnHolder> &apnHolder : apnHolders_) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("apn holder is null");
            continue;
        }
        std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine = apnHolder->GetCellularDataStateMachine();
        if (cellularDataStateMachine != nullptr && stateMachine == cellularDataStateMachine) {
            TELEPHONY_LOGE("cellularDataStateMachine in use");
            return false;
        }
    }
    return true;
}

bool ApnManager::HasAnyConnectedState() const
{
    for (const sptr<ApnHolder> &apnHolder : apnHolders_) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("apn holder is null");
            continue;
        }
        if (apnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTED ||
            apnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_DISCONNECTING) {
            return true;
        }
    }
    return false;
}

ApnProfileState ApnManager::GetOverallApnState() const
{
    if (apnHolders_.empty()) {
        TELEPHONY_LOGE("apn overall state is STATE_IDLE");
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    if (HasAnyConnectedState()) {
        TELEPHONY_LOGI("apn overall state is STATE_CONNECTED");
        return ApnProfileState::PROFILE_STATE_CONNECTED;
    }
    bool failState = false;
    for (const sptr<ApnHolder> &apnHolder : apnHolders_) {
        if (apnHolder == nullptr) {
            continue;
        }
        if (apnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTING ||
            apnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_RETRYING) {
            TELEPHONY_LOGI("apn overall state is STATE_CONNECTING");
            return ApnProfileState::PROFILE_STATE_CONNECTING;
        } else if (apnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_IDLE) {
            failState = true;
        }
    }
    if (failState) {
        TELEPHONY_LOGI("apn overall state is STATE_IDLE");
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    TELEPHONY_LOGI("apn overall state is STATE_FAILED");
    return ApnProfileState::PROFILE_STATE_FAILED;
}

sptr<ApnItem> ApnManager::GetRilAttachApn() const
{
    if (allApnItem_.empty()) {
        TELEPHONY_LOGE("apn item is null");
        return nullptr;
    }
    sptr<ApnItem> attachApn = nullptr;
    for (const sptr<ApnItem> &apnItem : allApnItem_) {
        if (apnItem->CanDealWithType(DATA_CONTEXT_ROLE_IA)) {
            attachApn = apnItem;
            break;
        }
        if (attachApn == nullptr && apnItem->CanDealWithType(DATA_CONTEXT_ROLE_DEFAULT)) {
            attachApn = apnItem;
        }
    }
    if (attachApn == nullptr) {
        attachApn = allApnItem_[0];
    }
    return attachApn;
}
} // namespace Telephony
} // namespace OHOS
