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

#include "connection_retry_policy.h"

#include <random>
#include "parameter.h"

#include "telephony_log_wrapper.h"
#include "cellular_data_utils.h"
#include "telephony_ext_wrapper.h"

namespace OHOS {
namespace Telephony {
static const char* PROP_RETRY_STRATEGY_ALLOW = "persist.telephony.retrystrategy.allow";
static const char* DEFAULT_RETRY_STRATEGY_ALLOW = "false";
static constexpr int32_t SYSPARA_SIZE = 8;
static constexpr int64_t DEFAULT_DELAY_FOR_INTERNAL_DEFAULT_APN = 30 * 1000;
static constexpr int64_t DEFAULT_DELAY_DATA_SETUP_FAIL = 3000;
static constexpr int64_t DEFAULT_DELAY_MODEM_DEACTIVATE_FAIL = 1000;
static constexpr int64_t DEFAULT_DELAY_FOR_OTHER_APN = 2 * 1000;
static constexpr int32_t MIN_RANDOM_DELAY = 0;
static constexpr int32_t MAX_RANDOM_DELAY = 2000;

ConnectionRetryPolicy::ConnectionRetryPolicy()
{
    char retryStrategyAllow[SYSPARA_SIZE] = { 0 };
    GetParameter(PROP_RETRY_STRATEGY_ALLOW, DEFAULT_RETRY_STRATEGY_ALLOW, retryStrategyAllow, SYSPARA_SIZE);
    isPropOn_ = (strcmp(retryStrategyAllow, "true") == 0);
    WatchParameter(PROP_RETRY_STRATEGY_ALLOW, OnPropChanged, this);
}

sptr<ApnItem> ConnectionRetryPolicy::GetNextRetryApnItem() const
{
    if ((matchedApns_.empty()) || IsAllBadApn()) {
        TELEPHONY_LOGE("matchedApns is null, or all bad apns");
        return nullptr;
    }
    if (currentApnIndex_ >= static_cast<int32_t>(matchedApns_.size()) || currentApnIndex_ < 0) {
        currentApnIndex_ = 0;
        tryCount_ = 0;
    }
    sptr<ApnItem> apnItem = matchedApns_[currentApnIndex_];
    tryCount_++;
    if ((apnItem != nullptr && apnItem->IsBadApn()) || (tryCount_ > maxCount_)) {
        TELEPHONY_LOGI("try next non-bad apn");
        tryCount_ = 0;
        currentApnIndex_++;
        return GetNextRetryApnItem();
    }
    return apnItem;
}

void ConnectionRetryPolicy::SetMatchedApns(std::vector<sptr<ApnItem>> &apns)
{
    matchedApns_ = apns;
}

void ConnectionRetryPolicy::ClearRetryApns()
{
    matchedApns_.clear();
}

void ConnectionRetryPolicy::MarkBadApn(ApnItem &apn)
{
    // The APN that fails after multiple retries is true
    apn.MarkBadApn(true);
}

int64_t ConnectionRetryPolicy::GetNextRetryDelay(std::string apnType, int32_t cause, int64_t suggestTime,
    RetryScene scene)
{
    int64_t retryDelay = GetRandomDelay();
    if (apnType == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
        retryDelay += DEFAULT_DELAY_FOR_INTERNAL_DEFAULT_APN;
    } else if (apnType == DATA_CONTEXT_ROLE_DEFAULT) {
        if (scene == RetryScene::RETRY_SCENE_MODEM_DEACTIVATE) {
            retryDelay += DEFAULT_DELAY_MODEM_DEACTIVATE_FAIL;
        } else {
            retryDelay += DEFAULT_DELAY_DATA_SETUP_FAIL;
        }
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
        if (isPropOn_ && TELEPHONY_EXT_WRAPPER.dataEndRetryStrategy_) {
            TELEPHONY_EXT_WRAPPER.dataEndRetryStrategy_(retryDelay, cause, suggestTime, maxCount_);
        }
#endif
    } else {
        retryDelay += DEFAULT_DELAY_FOR_OTHER_APN;
    }
    TELEPHONY_LOGI("%{public}s: cause=%{public}d, suggestTime=%{public}lld, tryCnt=%{public}d, delay=%{public}lld",
        apnType.c_str(), cause, static_cast<long long>(suggestTime), tryCount_, static_cast<long long>(retryDelay));
    return retryDelay;
}

void ConnectionRetryPolicy::InitialRetryCountValue()
{
    tryCount_ = 0;
}

std::vector<sptr<ApnItem>> ConnectionRetryPolicy::GetMatchedApns() const
{
    return matchedApns_;
}

void ConnectionRetryPolicy::OnPropChanged(const char *key, const char *value, void *context)
{
    if ((key == nullptr) || (value == nullptr) || (strcmp(key, PROP_RETRY_STRATEGY_ALLOW) != 0)) {
        return;
    }
    char retryStrategyAllow[SYSPARA_SIZE] = { 0 };
    GetParameter(PROP_RETRY_STRATEGY_ALLOW, DEFAULT_RETRY_STRATEGY_ALLOW, retryStrategyAllow, SYSPARA_SIZE);
    isPropOn_ = (strcmp(retryStrategyAllow, "true") == 0);
    TELEPHONY_LOGI("prop changes to %{public}s", retryStrategyAllow);
}

DisConnectionReason ConnectionRetryPolicy::ConvertPdpErrorToDisconnReason(int32_t reason)
{
    switch (reason) {
        case PdpErrorReason::PDP_ERR_OPERATOR_DETERMINED_BARRING:
        case PdpErrorReason::PDP_ERR_MISSING_OR_UNKNOWN_APN:
        case PdpErrorReason::PDP_ERR_UNKNOWN_PDP_ADDR_OR_TYPE:
        case PdpErrorReason::PDP_ERR_USER_VERIFICATION:
        case PdpErrorReason::PDP_ERR_ACTIVATION_REJECTED_GGSN:
        case PdpErrorReason::PDP_ERR_SERVICE_OPTION_NOT_SUPPORTED:
        case PdpErrorReason::PDP_ERR_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED:
        case PdpErrorReason::PDP_ERR_NSAPI_ALREADY_USED:
        case PdpErrorReason::PDP_ERR_IPV4_ONLY_ALLOWED:
        case PdpErrorReason::PDP_ERR_IPV6_ONLY_ALLOWED:
        case PdpErrorReason::PDP_ERR_PROTOCOL_ERRORS:
            return DisConnectionReason::REASON_PERMANENT_REJECT;
        case PdpErrorReason::PDP_ERR_UNKNOWN_TO_CLEAR_CONNECTION:
            return DisConnectionReason::REASON_CLEAR_CONNECTION;
        default:
            return DisConnectionReason::REASON_RETRY_CONNECTION;
    }
}

bool ConnectionRetryPolicy::IsAllBadApn() const
{
    for (const auto &apn : matchedApns_) {
        if (!apn->IsBadApn()) {
            return false;
        }
    }
    return true;
}

int64_t ConnectionRetryPolicy::GetRandomDelay()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(MIN_RANDOM_DELAY, MAX_RANDOM_DELAY);
    return dis(gen);
}
} // namespace Telephony
} // namespace OHOS