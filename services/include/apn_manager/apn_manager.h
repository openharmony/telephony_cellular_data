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

#ifndef APN_MANAGER_H
#define APN_MANAGER_H

#include <mutex>
#include <string>
#include <vector>

#include "apn_holder.h"
#include "apn_item.h"
#include "cellular_data_rdb_helper.h"
#include "net_all_capabilities.h"

namespace OHOS {
namespace Telephony {
class ApnManager : public RefBase {
public:
    ApnManager();
    ~ApnManager();
    sptr<ApnHolder> GetApnHolder(const std::string &apnType) const;
    std::vector<sptr<ApnHolder>> GetAllApnHolder() const;
    std::vector<sptr<ApnHolder>> GetSortApnHolder() const;
    std::vector<sptr<ApnItem>> FilterMatchedApns(const std::string &requestApnType, const int32_t slotId);
    void InitApnHolders();
    sptr<ApnHolder> FindApnHolderById(const int32_t id) const;
    static int32_t FindApnIdByApnName(const std::string &type);
    static std::string FindApnNameByApnId(const int32_t id);
    static int32_t FindApnIdByCapability(const uint64_t capabilities);
    static NetManagerStandard::NetCap FindBestCapability(const uint64_t capabilities);
    bool IsDataConnectionNotUsed(const std::shared_ptr<CellularDataStateMachine> &stateMachine) const;
    int32_t CreateAllApnItemByDatabase(int32_t slotId);
    bool HasAnyConnectedState() const;
    ApnProfileState GetOverallApnState() const;
    ApnProfileState GetOverallDefaultApnState() const;
    sptr<ApnItem> GetRilAttachApn();
    sptr<ApnItem> GetApnItemById(const int32_t id);
    bool ResetApns(int32_t slotId);
    void FetchDunApns(std::vector<sptr<ApnItem>> &matchApnItemList, const int32_t slotId);
    void FetchBipApns(std::vector<sptr<ApnItem>> &matchApnItemList);
    bool IsPreferredApnUserEdited();
    static int32_t FindApnTypeByApnName(const std::string &apnName);
    void ClearAllApnBad();
    static uint64_t FindCapabilityByApnId(int32_t apnId);

private:
    void ReportApnInfo(int32_t slotId, PdpProfile &apnData);
    void AddApnHolder(const std::string &apnType, const int32_t priority);
    int32_t CreateMvnoApnItems(int32_t slotId, const std::string &mcc, const std::string &mnc);
    int32_t MakeSpecificApnItem(std::vector<PdpProfile> &apnVec, int32_t slotId);
    void GetCTOperator(int32_t slotId, std::string &numeric);
    void TryMergeSimilarPdpProfile(std::vector<PdpProfile> &apnVec);
    void MergePdpProfile(PdpProfile &newProfile, PdpProfile &oldProfile);
    bool GetPreferId(int32_t slotId);
    int32_t PushApnItem(int32_t count, sptr<ApnItem> extraApnItem);

private:
    static const std::map<std::string, int32_t> apnIdApnNameMap_;
    static const std::map<std::string, ApnTypes> apnNameApnTypeMap_;
    static const std::vector<ApnProfileState> apnStateArr_;
    std::vector<sptr<ApnItem>> allApnItem_;
    std::vector<sptr<ApnHolder>> apnHolders_;
    std::map<int32_t, sptr<ApnHolder>> apnIdApnHolderMap_;
    std::vector<sptr<ApnHolder>> sortedApnHolders_;
    std::mutex mutex_;
    int32_t preferId_ = -1;
};
} // namespace Telephony
} // namespace OHOS
#endif // APN_MANAGER_H