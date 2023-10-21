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

#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "net_specifier.h"
#include "string_ex.h"
#include "telephony_log_wrapper.h"

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
    std::lock_guard<std::mutex> lock(mutex_);
    allApnItem_.clear();
    sptr<ApnItem> defaultApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DEFAULT);
    if (defaultApnItem != nullptr) {
        allApnItem_.push_back(defaultApnItem);
    }
    sptr<ApnItem> mmsApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_MMS);
    if (mmsApnItem != nullptr) {
        allApnItem_.push_back(mmsApnItem);
    }
    sptr<ApnItem> suplApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_SUPL);
    if (suplApnItem != nullptr) {
        allApnItem_.push_back(suplApnItem);
    }
    sptr<ApnItem> dunApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_DUN);
    if (dunApnItem != nullptr) {
        allApnItem_.push_back(dunApnItem);
    }
    sptr<ApnItem> imsApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_IMS);
    if (imsApnItem != nullptr) {
        allApnItem_.push_back(imsApnItem);
    }
    sptr<ApnItem> iaApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_IA);
    if (iaApnItem != nullptr) {
        allApnItem_.push_back(iaApnItem);
    }
    sptr<ApnItem> emergencyApnItem = ApnItem::MakeDefaultApn(DATA_CONTEXT_ROLE_EMERGENCY);
    if (emergencyApnItem != nullptr) {
        allApnItem_.push_back(emergencyApnItem);
    }
}

int32_t ApnManager::CreateAllApnItemByDatabase(int32_t slotId)
{
    int32_t count = 0;
    std::u16string operatorNumeric;
    CoreManagerInner::GetInstance().GetSimOperatorNumeric(slotId, operatorNumeric);
    std::string numeric = Str16ToStr8(operatorNumeric);
    if (numeric.empty()) {
        TELEPHONY_LOGE("numeric is empty!!!");
        return count;
    }
    std::string mcc = numeric.substr(0, DEFAULT_MCC_SIZE);
    std::string mnc = numeric.substr(mcc.size(), numeric.size() - mcc.size());
    TELEPHONY_LOGI("current slotId = %{public}d, mcc = %{public}s, mnc = %{public}s", slotId, mcc.c_str(), mnc.c_str());
    int32_t mvnoCount = CreateMvnoApnItems(slotId, mcc, mnc);
    if (mvnoCount > 0) {
        return mvnoCount;
    }
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
    return MakeSpecificApnItem(apnVec);
}

int32_t ApnManager::CreateMvnoApnItems(int32_t slotId, const std::string &mcc, const std::string &mnc)
{
    int32_t count = 0;
    auto helper = CellularDataRdbHelper::GetInstance();
    if (helper == nullptr) {
        TELEPHONY_LOGE("get cellularDataRdbHelper failed");
        return count;
    }
    std::vector<PdpProfile> mvnoApnVec;
    std::u16string spn;
    CoreManagerInner::GetInstance().GetSimSpn(slotId, spn);
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::SPN, Str16ToStr8(spn), mvnoApnVec)) {
        TELEPHONY_LOGE("query mvno apns by spn fail");
        return count;
    }
    std::u16string imsi;
    CoreManagerInner::GetInstance().GetIMSI(slotId, imsi);
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::IMSI, Str16ToStr8(imsi), mvnoApnVec)) {
        TELEPHONY_LOGE("query mvno apns by imsi fail");
        return count;
    }
    std::u16string gid1;
    CoreManagerInner::GetInstance().GetSimGid1(slotId, gid1);
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::GID1, Str16ToStr8(gid1), mvnoApnVec)) {
        TELEPHONY_LOGE("query mvno apns by gid1 fail");
        return count;
    }
    std::u16string iccId;
    CoreManagerInner::GetInstance().GetSimIccId(slotId, iccId);
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::ICCID, Str16ToStr8(iccId), mvnoApnVec)) {
        TELEPHONY_LOGE("query mvno apns by iccId fail");
        return count;
    }
    return MakeSpecificApnItem(mvnoApnVec);
}

int32_t ApnManager::MakeSpecificApnItem(const std::vector<PdpProfile> &apnVec)
{
    std::lock_guard<std::mutex> lock(mutex_);
    allApnItem_.clear();
    int32_t count = 0;
    for (const PdpProfile &apnData : apnVec) {
        TELEPHONY_LOGI("profileId = %{public}d, profileName = %{public}s, mvnoType = %{public}s", apnData.profileId,
            apnData.profileName.c_str(), apnData.mvnoType.c_str());
        sptr<ApnItem> apnItem = ApnItem::MakeApn(apnData);
        if (apnItem != nullptr) {
            allApnItem_.push_back(apnItem);
            count++;
        }
    }
    return count;
}

std::vector<sptr<ApnItem>> ApnManager::FilterMatchedApns(const std::string &requestApnType)
{
    std::lock_guard<std::mutex> lock(mutex_);
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

sptr<ApnItem> ApnManager::GetRilAttachApn()
{
    std::lock_guard<std::mutex> lock(mutex_);
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
