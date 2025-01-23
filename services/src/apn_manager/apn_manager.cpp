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
#include "cellular_data_hisysevent.h"
#include "core_manager_inner.h"
#include "net_specifier.h"
#include "string_ex.h"
#include "tel_profile_util.h"
#include "telephony_log_wrapper.h"
#include "telephony_ext_wrapper.h"

namespace OHOS {
namespace Telephony {
const std::map<std::string, int32_t> ApnManager::apnIdApnNameMap_ {
    {DATA_CONTEXT_ROLE_ALL, DATA_CONTEXT_ROLE_ALL_ID},
    {DATA_CONTEXT_ROLE_DEFAULT, DATA_CONTEXT_ROLE_DEFAULT_ID},
    {DATA_CONTEXT_ROLE_MMS, DATA_CONTEXT_ROLE_MMS_ID},
    {DATA_CONTEXT_ROLE_SUPL, DATA_CONTEXT_ROLE_SUPL_ID},
    {DATA_CONTEXT_ROLE_DUN, DATA_CONTEXT_ROLE_DUN_ID},
    {DATA_CONTEXT_ROLE_IMS, DATA_CONTEXT_ROLE_IMS_ID},
    {DATA_CONTEXT_ROLE_IA, DATA_CONTEXT_ROLE_IA_ID},
    {DATA_CONTEXT_ROLE_EMERGENCY, DATA_CONTEXT_ROLE_EMERGENCY_ID},
    {DATA_CONTEXT_ROLE_INTERNAL_DEFAULT, DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID},
    {DATA_CONTEXT_ROLE_XCAP, DATA_CONTEXT_ROLE_XCAP_ID},
    {DATA_CONTEXT_ROLE_BIP, DATA_CONTEXT_ROLE_BIP_ID}
};
const std::map<std::string, ApnTypes> ApnManager::apnNameApnTypeMap_ {
    {DATA_CONTEXT_ROLE_ALL, ApnTypes::ALL},
    {DATA_CONTEXT_ROLE_DEFAULT, ApnTypes::DEFAULT},
    {DATA_CONTEXT_ROLE_MMS, ApnTypes::MMS},
    {DATA_CONTEXT_ROLE_SUPL, ApnTypes::SUPL},
    {DATA_CONTEXT_ROLE_DUN, ApnTypes::DUN},
    {DATA_CONTEXT_ROLE_IMS, ApnTypes::IMS},
    {DATA_CONTEXT_ROLE_IA, ApnTypes::IA},
    {DATA_CONTEXT_ROLE_EMERGENCY, ApnTypes::EMERGENCY},
    {DATA_CONTEXT_ROLE_XCAP, ApnTypes::XCAP},
    {DATA_CONTEXT_ROLE_BIP, ApnTypes::BIP},
    {DATA_CONTEXT_ROLE_INTERNAL_DEFAULT, ApnTypes::INTERNAL_DEFAULT}
};
const std::vector<ApnProfileState> ApnManager::apnStateArr_ = {
    PROFILE_STATE_CONNECTED,
    PROFILE_STATE_DISCONNECTING,
    PROFILE_STATE_CONNECTING,
    PROFILE_STATE_IDLE,
    PROFILE_STATE_RETRYING,
    PROFILE_STATE_FAILED
};
constexpr const char *CT_MCC_MNC_1 = "46003";
constexpr const char *CT_MCC_MNC_2 = "46011";
constexpr const char *GC_ICCID = "8985231";
constexpr const char *GC_MCC_MNC = "45431";
constexpr const char *GC_SPN = "CTExcel";
constexpr const char *MO_ICCID_1 = "8985302";
constexpr const char *MO_ICCID_2 = "8985307";
constexpr const char *MO_UNICOM_MCCMNC = "46001";
constexpr int32_t ICCID_LEN_MINIMUM = 7;

ApnManager::ApnManager() = default;

ApnManager::~ApnManager() = default;

void ApnManager::InitApnHolders()
{
    AddApnHolder(DATA_CONTEXT_ROLE_DEFAULT, static_cast<int32_t>(DataContextPriority::PRIORITY_LOW));
    AddApnHolder(DATA_CONTEXT_ROLE_MMS, static_cast<int32_t>(DataContextPriority::PRIORITY_NORMAL));
    AddApnHolder(DATA_CONTEXT_ROLE_INTERNAL_DEFAULT, static_cast<int32_t>(DataContextPriority::PRIORITY_NONE));
    AddApnHolder(DATA_CONTEXT_ROLE_XCAP, static_cast<int32_t>(DataContextPriority::PRIORITY_NORMAL));
    AddApnHolder(DATA_CONTEXT_ROLE_DUN, static_cast<int32_t>(DataContextPriority::PRIORITY_NORMAL));
    AddApnHolder(DATA_CONTEXT_ROLE_IA, static_cast<int32_t>(DataContextPriority::PRIORITY_HIGH));
    AddApnHolder(DATA_CONTEXT_ROLE_SUPL, static_cast<int32_t>(DataContextPriority::PRIORITY_NORMAL));
    AddApnHolder(DATA_CONTEXT_ROLE_BIP, static_cast<int32_t>(DataContextPriority::PRIORITY_NORMAL));
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

int32_t ApnManager::FindApnTypeByApnName(const std::string &apnName)
{
    auto iter = apnNameApnTypeMap_.find(apnName);
    if (iter != apnNameApnTypeMap_.end()) {
        return static_cast<uint64_t>(iter->second);
    }
    TELEPHONY_LOGI("ApnName %{public}s is not exist!", apnName.c_str());
    return static_cast<uint64_t>(ApnTypes::NONETYPE);
}

int32_t ApnManager::FindApnIdByCapability(const uint64_t capability)
{
    switch (capability) {
        case NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET:
            return DATA_CONTEXT_ROLE_DEFAULT_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_MMS:
            return DATA_CONTEXT_ROLE_MMS_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_INTERNAL_DEFAULT:
            return DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_IA:
            return DATA_CONTEXT_ROLE_IA_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_XCAP:
            return DATA_CONTEXT_ROLE_XCAP_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_SUPL:
            return DATA_CONTEXT_ROLE_SUPL_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_DUN:
            return DATA_CONTEXT_ROLE_DUN_ID;
        case NetManagerStandard::NetCap::NET_CAPABILITY_BIP:
            return DATA_CONTEXT_ROLE_BIP_ID;
        default:
            return DATA_CONTEXT_ROLE_INVALID_ID;
    }
}

static bool HasNetCap(const uint64_t capabilities, const NetManagerStandard::NetCap netCap)
{
    return (capabilities & (1L << netCap)) != 0;
}

NetManagerStandard::NetCap ApnManager::FindBestCapability(const uint64_t capabilities)
{
    NetManagerStandard::NetCap netCap = NetManagerStandard::NetCap::NET_CAPABILITY_END;
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET;
    }
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_INTERNAL_DEFAULT)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_INTERNAL_DEFAULT;
    }
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_MMS)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_MMS;
    }
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_SUPL)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_SUPL;
    }
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_DUN)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_DUN;
    }
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_XCAP)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_XCAP;
    }
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_IA)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_IA;
    }
    if (HasNetCap(capabilities, NetManagerStandard::NetCap::NET_CAPABILITY_BIP)) {
        netCap = NetManagerStandard::NetCap::NET_CAPABILITY_BIP;
    }
    return netCap;
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
            if (c1 == nullptr || c2 == nullptr) {
                return false;
            }
            return c2->GetPriority() < c1->GetPriority();
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
    TELEPHONY_LOGE("apnType %{public}s is not exist!", apnType.c_str());
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

int32_t ApnManager::CreateAllApnItemByDatabase(int32_t slotId)
{
    int32_t count = 0;
    if (TELEPHONY_EXT_WRAPPER.createAllApnItemExt_) {
        sptr<ApnItem> extraApnItem = ApnItem::MakeDefaultApn("default");
        if (TELEPHONY_EXT_WRAPPER.createAllApnItemExt_(slotId, extraApnItem)) {
            std::lock_guard<std::mutex> lock(mutex_);
            allApnItem_.clear();
            allApnItem_.push_back(extraApnItem);
            return ++count;
        }
    }
    std::u16string operatorNumeric;
    CoreManagerInner::GetInstance().GetSimOperatorNumeric(slotId, operatorNumeric);
    std::string numeric = Str16ToStr8(operatorNumeric);
    GetCTOperator(slotId, numeric);
    if (numeric.empty()) {
        TELEPHONY_LOGE("numeric is empty!!!");
        return count;
    }
    TELEPHONY_LOGI("current slotId = %{public}d, numeric = %{public}s", slotId, numeric.c_str());
    auto helper = CellularDataRdbHelper::GetInstance();
    if (helper == nullptr) {
        TELEPHONY_LOGE("get cellularDataRdbHelper failed");
        return count;
    }
    preferId_ = INVALID_PROFILE_ID;
    std::vector<PdpProfile> preferApnVec;
    if (helper->QueryPreferApn(slotId, preferApnVec)) {
        preferId_ = preferApnVec[0].profileId;
        TELEPHONY_LOGI("query preferId_ = %{public}d", preferId_);
    }
    std::string mcc = numeric.substr(0, DEFAULT_MCC_SIZE);
    std::string mnc = numeric.substr(mcc.size(), numeric.size() - mcc.size());
    int32_t mvnoCount = CreateMvnoApnItems(slotId, mcc, mnc);
    if (mvnoCount > 0) {
        return mvnoCount;
    }
    std::vector<PdpProfile> apnVec;
    if (!helper->QueryApns(mcc, mnc, apnVec, slotId)) {
        TELEPHONY_LOGE("query apns from data ability fail");
        return count;
    }
    return MakeSpecificApnItem(apnVec, slotId);
}

void ApnManager::GetCTOperator(int32_t slotId, std::string &numeric)
{
    bool isCTSimCard = false;
    CoreManagerInner::GetInstance().IsCTSimCard(slotId, isCTSimCard);
    if (isCTSimCard && numeric.compare(CT_MCC_MNC_2)) {
        numeric = CT_MCC_MNC_1;
    }
    if (!numeric.compare(CT_MCC_MNC_1)) {
        std::u16string tempIccId;
        CoreManagerInner::GetInstance().GetSimIccId(slotId, tempIccId);
        std::string iccId = Str16ToStr8(tempIccId);
        std::u16string tempSpn;
        CoreManagerInner::GetInstance().GetSimSpn(slotId, tempSpn);
        std::string spn = Str16ToStr8(tempSpn);
        if (!iccId.compare(0, ICCID_LEN_MINIMUM, GC_ICCID) || !spn.compare(GC_SPN)) {
            numeric = GC_MCC_MNC;
        } else if (!iccId.compare(0, ICCID_LEN_MINIMUM, MO_ICCID_1) ||
            !iccId.compare(0, ICCID_LEN_MINIMUM, MO_ICCID_2)) {
            std::u16string tempPlmn;
            CoreManagerInner::GetInstance().GetSimOperatorNumeric(slotId, tempPlmn);
            std::string plmn = Str16ToStr8(tempPlmn);
            if (!plmn.empty() && !plmn.compare(MO_UNICOM_MCCMNC)) {
                numeric = MO_UNICOM_MCCMNC;
            }
        }
    }
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
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::SPN, Str16ToStr8(spn), mvnoApnVec, slotId)) {
        TELEPHONY_LOGE("query mvno apns by spn fail");
        return count;
    }
    std::u16string imsi;
    CoreManagerInner::GetInstance().GetIMSI(slotId, imsi);
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::IMSI, Str16ToStr8(imsi), mvnoApnVec, slotId)) {
        TELEPHONY_LOGE("query mvno apns by imsi fail");
        return count;
    }
    std::u16string gid1;
    CoreManagerInner::GetInstance().GetSimGid1(slotId, gid1);
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::GID1, Str16ToStr8(gid1), mvnoApnVec, slotId)) {
        TELEPHONY_LOGE("query mvno apns by gid1 fail");
        return count;
    }
    std::u16string iccId;
    CoreManagerInner::GetInstance().GetSimIccId(slotId, iccId);
    if (!helper->QueryMvnoApnsByType(mcc, mnc, MvnoType::ICCID, Str16ToStr8(iccId), mvnoApnVec, slotId)) {
        TELEPHONY_LOGE("query mvno apns by iccId fail");
        return count;
    }
    return MakeSpecificApnItem(mvnoApnVec, slotId);
}

void ApnManager::ReportApnInfo(int32_t slotId, PdpProfile &apnData)
{
    if (apnData.apnTypes.empty()) {
        return;
    }
    if (apnData.apnTypes.find(DATA_CONTEXT_ROLE_DEFAULT) == std::string::npos) {
        return;
    }
    CellularDataHiSysEvent::WriteApnInfoBehaviorEvent(slotId, apnData);
}

int32_t ApnManager::MakeSpecificApnItem(std::vector<PdpProfile> &apnVec, int32_t slotId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    allApnItem_.clear();
    TryMergeSimilarPdpProfile(apnVec);
    int32_t count = 0;
    for (PdpProfile &apnData : apnVec) {
        TELEPHONY_LOGI("profileId = %{public}d, profileName = %{public}s, mvnoType = %{public}s",
            apnData.profileId, apnData.profileName.c_str(), apnData.mvnoType.c_str());
        if (apnData.profileId == preferId_ && apnData.apnTypes.empty()) {
            apnData.apnTypes = DATA_CONTEXT_ROLE_DEFAULT;
        }
        ReportApnInfo(slotId, apnData);
        sptr<ApnItem> apnItem = ApnItem::MakeApn(apnData);
        if (apnItem != nullptr) {
            allApnItem_.push_back(apnItem);
            count++;
        }
    }
    int32_t preferId = preferId_;
    auto it = std::find_if(allApnItem_.begin(), allApnItem_.end(),
        [preferId](auto &apn) { return apn != nullptr && apn->attr_.profileId_ == preferId; });
    if (it != allApnItem_.end()) {
        sptr<ApnItem> apnItem = *it;
        allApnItem_.erase(it);
        allApnItem_.insert(allApnItem_.begin(), apnItem);
    }
    return count;
}

std::vector<sptr<ApnItem>> ApnManager::FilterMatchedApns(const std::string &requestApnType, const int32_t slotId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<sptr<ApnItem>> matchApnItemList;
    if (requestApnType == DATA_CONTEXT_ROLE_DUN) {
        FetchDunApns(matchApnItemList, slotId);
        return matchApnItemList;
    } else if (requestApnType == DATA_CONTEXT_ROLE_BIP) {
        FetchBipApns(matchApnItemList);
        return matchApnItemList;
    }

    for (const sptr<ApnItem> &apnItem : allApnItem_) {
        if (apnItem->CanDealWithType(requestApnType)) {
            matchApnItemList.push_back(apnItem);
        }
    }
    TELEPHONY_LOGD("apn size is :%{public}zu", matchApnItemList.size());
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
        TELEPHONY_LOGD("apn overall state is STATE_IDLE");
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    TELEPHONY_LOGI("apn overall state is STATE_FAILED");
    return ApnProfileState::PROFILE_STATE_FAILED;
}

ApnProfileState ApnManager::GetOverallDefaultApnState() const
{
    if (apnHolders_.empty()) {
        TELEPHONY_LOGE("apn overall state is STATE_IDLE");
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    ApnProfileState defaultApnState = ApnProfileState::PROFILE_STATE_IDLE;
    ApnProfileState internalApnState = ApnProfileState::PROFILE_STATE_IDLE;

    for (const sptr<ApnHolder> &apnHolder : apnHolders_) {
        if (apnHolder == nullptr) {
            continue;
        }
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_DEFAULT) {
            defaultApnState = apnHolder->GetApnState();
        }
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
            internalApnState = apnHolder->GetApnState();
        }
    }
    TELEPHONY_LOGI("defaultApnState is %{public}d, internalApnState is %{public}d", defaultApnState,
        internalApnState);
    for (auto apnState : apnStateArr_) {
        if (defaultApnState == apnState || internalApnState == apnState) {
            return apnState;
        }
    }
    return ApnProfileState::PROFILE_STATE_IDLE;
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

sptr<ApnItem> ApnManager::GetApnItemById(const int32_t id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (allApnItem_.empty()) {
        TELEPHONY_LOGE("apn item is null");
        return nullptr;
    }
    sptr<ApnItem> attachApn = nullptr;
    for (const sptr<ApnItem> &apnItem : allApnItem_) {
        if (id == apnItem->attr_.profileId_) {
            attachApn = apnItem;
            break;
        }
    }
    return attachApn;
}

bool ApnManager::ResetApns(int32_t slotId)
{
    auto helper = CellularDataRdbHelper::GetInstance();
    if (helper == nullptr) {
        TELEPHONY_LOGE("get cellularDataRdbHelper failed");
        return false;
    }
    return helper->ResetApns(slotId);
}

void ApnManager::FetchBipApns(std::vector<sptr<ApnItem>> &matchApnItemList)
{
    sptr<ApnItem> defaultApn = nullptr;
    for (const sptr<ApnItem> &apnItem : allApnItem_) {
        if (apnItem->CanDealWithType(DATA_CONTEXT_ROLE_BIP)) {
            matchApnItemList.push_back(apnItem);
            return;
        }
        if (apnItem->CanDealWithType(DATA_CONTEXT_ROLE_DEFAULT)) {
            defaultApn = apnItem;
        }
    }

    if (defaultApn != nullptr) {
        matchApnItemList.push_back(defaultApn);
    }
}

void ApnManager::FetchDunApns(std::vector<sptr<ApnItem>> &matchApnItemList, const int32_t slotId)
{
    bool roamingState = CoreManagerInner::GetInstance().GetPsRoamingState(slotId) > 0;
    if (roamingState && !IsPreferredApnUserEdited()) {
        TELEPHONY_LOGI("FetchDunApns: Dun apn is not used in roaming network");
        return;
    }
    int32_t preferId = preferId_;
    sptr<ApnItem> preferredApn = nullptr;
    auto it = std::find_if(allApnItem_.begin(), allApnItem_.end(), [preferId](auto &apn) {
        return apn != nullptr && apn->attr_.profileId_ == preferId;
    });
    if (it != allApnItem_.end()) {
        preferredApn = *it;
    }
    if (preferredApn != nullptr && preferredApn->CanDealWithType(DATA_CONTEXT_ROLE_DUN)) {
        matchApnItemList.insert(matchApnItemList.begin(), preferredApn);
    }
    if (matchApnItemList.empty()) {
        for (const auto &item : allApnItem_) {
            if (item->CanDealWithType(DATA_CONTEXT_ROLE_DUN)) {
                matchApnItemList.push_back(item);
            }
        }
    }
}

bool ApnManager::IsPreferredApnUserEdited()
{
    bool isUserEdited = false;
    int32_t preferId = preferId_;
    auto it = std::find_if(allApnItem_.begin(), allApnItem_.end(), [preferId](auto &apn) {
        return apn->attr_.profileId_ == preferId;
    });
    if (it != allApnItem_.end() && *it != nullptr) {
        isUserEdited = (*it)->attr_.isEdited_;
    }
    return isUserEdited;
}

void ApnManager::ClearAllApnBad()
{
    for (const sptr<ApnHolder> &apnHolder : apnHolders_) {
        if (apnHolder != nullptr) {
            apnHolder->SetApnBadState(false);
        }
    }
}

void ApnManager::TryMergeSimilarPdpProfile(std::vector<PdpProfile> &apnVec)
{
    // coalesce similar APNs to prevent bringing up two data calls with same interface
    std::vector<PdpProfile> newApnVec;
    for (size_t i = 0; i < apnVec.size(); i++) {
        if (apnVec[i].profileId == -1) {
            continue;
        }
        for (size_t j = i + 1; j < apnVec.size(); j++) {
            if (ApnItem::IsSimilarPdpProfile(apnVec[i], apnVec[j])) {
                MergePdpProfile(apnVec[i], apnVec[j]);
                apnVec[j].profileId = -1;
            }
        }
        newApnVec.push_back(apnVec[i]);
    }
    apnVec.assign(newApnVec.begin(), newApnVec.end());
}

void ApnManager::MergePdpProfile(PdpProfile &newProfile, PdpProfile &oldProfile)
{
    newProfile.apnTypes = newProfile.apnTypes + ',' + oldProfile.apnTypes;
    newProfile.pdpProtocol = (newProfile.pdpProtocol == PROTOCOL_IPV4V6) ?
        newProfile.pdpProtocol : oldProfile.pdpProtocol;
    newProfile.roamPdpProtocol = (newProfile.roamPdpProtocol == PROTOCOL_IPV4V6) ?
        newProfile.roamPdpProtocol : oldProfile.roamPdpProtocol;
    if (preferId_ == oldProfile.profileId) {
        TELEPHONY_LOGI("preferId change from %{public}d to %{public}d", oldProfile.profileId, newProfile.profileId);
        preferId_ = newProfile.profileId;
    }
    TELEPHONY_LOGI("merge %{public}d and %{public}d: apn[%{public}s], apnTypes[%{public}s]",
        newProfile.profileId, oldProfile.profileId, newProfile.apn.c_str(), newProfile.apnTypes.c_str());
}

uint64_t ApnManager::FindCapabilityByApnId(int32_t apnId)
{
    switch (apnId) {
        case DATA_CONTEXT_ROLE_DEFAULT_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET;
        case DATA_CONTEXT_ROLE_MMS_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_MMS;
        case DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_INTERNAL_DEFAULT;
        case DATA_CONTEXT_ROLE_IA_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_IA;
        case DATA_CONTEXT_ROLE_XCAP_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_XCAP;
        case DATA_CONTEXT_ROLE_SUPL_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_SUPL;
        case DATA_CONTEXT_ROLE_DUN_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_DUN;
        case DATA_CONTEXT_ROLE_BIP_ID:
            return NetManagerStandard::NetCap::NET_CAPABILITY_BIP;
        default:
            return NetManagerStandard::NetCap::NET_CAPABILITY_END;
    }
}
}  // namespace Telephony
}  // namespace OHOS
