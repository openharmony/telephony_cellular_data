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

#ifndef CONNECTION_RETRY_POLICY_H
#define CONNECTION_RETRY_POLICY_H

#include <memory>
#include <string>
#include <vector>

#include "apn_item.h"

namespace OHOS {
namespace Telephony {
class ConnectionRetryPolicy {
public:
    ConnectionRetryPolicy();
    ~ConnectionRetryPolicy();
    sptr<ApnItem> GetNextRetryApnItem() const;
    void SetMatchedApns(std::vector<sptr<ApnItem>> &apns);
    void ClearRetryApns();
    void MarkBadApn(ApnItem &apn);
    int64_t GetNextRetryDelay() const;
    void InitialRetryCountValue();

private:
    std::vector<sptr<ApnItem>> matchedApns_;
    mutable int32_t tryCount_ = 0;
    int32_t maxCount_ = 5;
    mutable int32_t currentApnIndex_ = 0;
};
} // namespace Telephony
} // namespace OHOS
#endif // CONNECTION_RETRY_POLICY_H