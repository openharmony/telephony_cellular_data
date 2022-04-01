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

#include <string>
#include <vector>

#include "apn_holder.h"
#include "apn_item.h"
#include "cellular_data_rdb_helper.h"

namespace OHOS {
namespace Telephony {
class ApnManager : public RefBase {
public:
    ApnManager();
    ~ApnManager();
    sptr<ApnHolder> GetApnHolder(const std::string &apnType) const;
    std::vector<sptr<ApnHolder>> GetAllApnHolder() const;
    std::vector<sptr<ApnHolder>> GetSortApnHolder() const;
    void CreateAllApnItem();
    std::vector<sptr<ApnItem>> FilterMatchedApns(const std::string &requestApnType) const;
    void InitApnHolders();
    sptr<ApnHolder> FindApnHolderById(const int32_t id) const;
    static int32_t FindApnIdByApnName(const std::string &type);
    static std::string FindApnNameByApnId(const int32_t id);
    static int32_t FindApnIdByCapability(const uint64_t capability);
    bool IsDataConnectionNotUsed(const std::shared_ptr<CellularDataStateMachine> &stateMachine) const;
    int32_t CreateAllApnItemByDatabase(const std::string &numeric);
    bool HasAnyConnectedState() const;
    ApnProfileState GetOverallApnState() const;
    sptr<ApnItem> GetRilAttachApn() const;

private:
    void AddApnHolder(const std::string &apnType, const int32_t priority);

private:
    static const std::map<std::string, int32_t> apnIdApnNameMap_;
    std::vector<sptr<ApnItem>> allApnItem_;
    std::vector<sptr<ApnHolder>> apnHolders_;
    std::map<int32_t, sptr<ApnHolder>> apnIdApnHolderMap_;
    std::vector<sptr<ApnHolder>> sortedApnHolders_;
};
} // namespace Telephony
} // namespace OHOS
#endif // APN_MANAGER_H