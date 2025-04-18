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

#ifndef APN_HOLDER_H
#define APN_HOLDER_H

#include <map>
#include <memory>
#include <set>

#include "apn_item.h"
#include "cellular_data_constant.h"
#include "connection_retry_policy.h"
#include "net_supplier_callback_base.h"

namespace OHOS {
namespace Telephony {
class CellularDataStateMachine;
enum class HasSystemUse : int32_t {
    UNKOWN = -1,
    NOT_HAS = 0,
    HAS = 1,
};

class ApnHolder : public RefBase {
public:
    ApnHolder(const std::string &apnType, const int32_t priority);
    ~ApnHolder();
    sptr<ApnItem> GetNextRetryApn() const;
    void SetAllMatchedApns(std::vector<sptr<ApnItem>> &matchedApns);
    int64_t GetRetryDelay(int32_t cause, int32_t suggestTime, RetryScene scene, bool isDefaultApnRetrying);
    void SetCurrentApn(sptr<ApnItem> &apnItem);
    sptr<ApnItem> GetCurrentApn() const;
    void SetApnState(ApnProfileState state);
    ApnProfileState GetApnState() const;
    bool IsDataCallEnabled() const;
    std::string GetApnType() const;
    void ReleaseDataConnection();
    int32_t GetProfileId(const std::string &apnType) const;
    void SetCellularDataStateMachine(const std::shared_ptr<CellularDataStateMachine> &stateMachine);
    std::shared_ptr<CellularDataStateMachine> GetCellularDataStateMachine() const;
    uint64_t GetCapability() const;
    int32_t GetPriority() const;
    void RequestCellularData(const NetRequest &netRequest);
    bool ReleaseCellularData(const NetRequest &netRequest);
    void ReleaseAllCellularData();
    bool IsEmergencyType() const;
    bool IsMmsType() const;
    bool IsBipType() const;
    void InitialApnRetryCount();
    bool IsSameMatchedApns(std::vector<sptr<ApnItem>> newMatchedApns, bool roamingState);
    static bool IsSameApnItem(const sptr<ApnItem> &newApnItem, const sptr<ApnItem> &oldApnItem, bool roamingState);
    static bool IsCompatibleApnItem(const sptr<ApnItem> &newApnItem, const sptr<ApnItem> &oldApnItem,
        bool roamingState);
    void AddUid(uint32_t uid);
    void RemoveUid(uint32_t uid);
    HasSystemUse GetUidStatus() const;
    void SetApnBadState(bool isBad);
    bool IsReqUidsEmpty();

private:
    ApnHolder(ApnHolder &apnHolder) = default;
    ApnHolder &operator=(ApnHolder &apnHolder) = default;

private:
    static const std::map<std::string, int32_t> apnTypeDataProfileMap_;
    bool dataCallEnabled_ = false;
    uint64_t capability_ = 0;
    ConnectionRetryPolicy retryPolicy_;
    ApnProfileState apnState_ = ApnProfileState::PROFILE_STATE_IDLE;
    sptr<ApnItem> apnItem_;
    std::string apnType_;
    int32_t priority_;
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine_;
    std::vector<NetRequest> netRequests_;
    std::set<uint32_t> reqUids_;
};
} // namespace Telephony
} // namespace OHOS
#endif // APN_HOLDER_H
