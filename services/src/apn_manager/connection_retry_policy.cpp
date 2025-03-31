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

#include <charconv>
#include <random>
#include "parameter.h"

#include "telephony_log_wrapper.h"
#include "cellular_data_utils.h"
#include "telephony_ext_wrapper.h"

namespace OHOS {
namespace Telephony {
static const char* PROP_RETRY_STRATEGY_ALLOW = "persist.telephony.retrystrategy.allow";
static const char* PROP_SETUP_FAIL_DELAY = "persist.telephony.setupfail.delay";
static const char* PROP_MODEM_DEND_DELAY = "persist.telephony.modemdend.delay";
static const char* DEFAULT_DELAY = "3000";
static const char* DEFAULT_RETRY_STRATEGY_ALLOW = "false";
static constexpr int16_t BASE_10 = 10;
static constexpr int32_t SYSPARA_SIZE = 8;
static constexpr int64_t DEFAULT_DELAY_FOR_INTERNAL_DEFAULT_APN_L = 60 * 1000;
static constexpr int64_t DEFAULT_DELAY_FOR_INTERNAL_DEFAULT_APN_S = 5 * 1000;
static constexpr int64_t DEFAULT_DELAY_FOR_OTHER_APN = 2 * 1000;
static constexpr int32_t MIN_RANDOM_DELAY = 0;
static constexpr int32_t MAX_RANDOM_DELAY = 2000;

ConnectionRetryPolicy::ConnectionRetryPolicy()
{
    char retryStrategyAllow[SYSPARA_SIZE] = { 0 };
    GetParameter(PROP_RETRY_STRATEGY_ALLOW, DEFAULT_RETRY_STRATEGY_ALLOW, retryStrategyAllow, SYSPARA_SIZE);
    isPropOn_ = (strcmp(retryStrategyAllow, "true") == 0);
    char setupFailDelay[SYSPARA_SIZE] = { 0 };
    GetParameter(PROP_SETUP_FAIL_DELAY, DEFAULT_DELAY, setupFailDelay, SYSPARA_SIZE);
    if (!ConvertStrToInt(setupFailDelay, defaultSetupFailDelay_)) {
        TELEPHONY_LOGE("setupFailDelay is invalid: %{public}s", setupFailDelay);
    }
    char modemDendDelay[SYSPARA_SIZE] = { 0 };
    GetParameter(PROP_MODEM_DEND_DELAY, DEFAULT_DELAY, modemDendDelay, SYSPARA_SIZE);
    if (!ConvertStrToInt(modemDendDelay, defaultModemDendDelay_)) {
        TELEPHONY_LOGE("modemDendDelay is invalid: %{public}s", modemDendDelay);
    }
    WatchParameter(PROP_RETRY_STRATEGY_ALLOW, OnPropChanged, this);
    WatchParameter(PROP_SETUP_FAIL_DELAY, OnPropChanged, this);
    WatchParameter(PROP_MODEM_DEND_DELAY, OnPropChanged, this);
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
    if (matchedApns_.size() != apns.size()) {
        TELEPHONY_LOGI("reset currentApnIndex");
        currentApnIndex_ = 0;
    } else {
        for (int32_t i = 0; i < apns.size(); i++) {
            if (apns[i]->attr_.profileId_ != matchedApns_[i]->attr_.profileId_) {
                TELEPHONY_LOGI("reset currentApnIndex");
                currentApnIndex_ = 0;
                break;
            }
        }
    }
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
    RetryScene scene, bool isDefaultApnRetrying)
{
    int64_t retryDelay = GetRandomDelay();
    if (apnType == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
        retryDelay += isDefaultApnRetrying ? DEFAULT_DELAY_FOR_INTERNAL_DEFAULT_APN_L :
            DEFAULT_DELAY_FOR_INTERNAL_DEFAULT_APN_S;
    } else if (apnType == DATA_CONTEXT_ROLE_DEFAULT) {
        if (scene == RetryScene::RETRY_SCENE_MODEM_DEACTIVATE) {
            retryDelay += defaultModemDendDelay_;
        } else {
            retryDelay += defaultSetupFailDelay_;
        }
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
        int64_t updatedDelay = 0;
        if (isPropOn_ && TELEPHONY_EXT_WRAPPER.handleDendFailcause_) {
            updatedDelay = TELEPHONY_EXT_WRAPPER.handleDendFailcause_(cause, suggestTime);
        }
        if (updatedDelay > 0) {
            retryDelay = updatedDelay;
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
    if ((key == nullptr) || (value == nullptr)) {
        return;
    }
    if (strcmp(key, PROP_RETRY_STRATEGY_ALLOW) == 0) {
        isPropOn_ = (strcmp(value, "true") == 0);
    } else if ((strcmp(key, PROP_SETUP_FAIL_DELAY) == 0)) {
        if (!ConvertStrToInt(value, defaultSetupFailDelay_)) {
            TELEPHONY_LOGE("invalid value: %{public}s", value);
        }
    } else if ((strcmp(key, PROP_MODEM_DEND_DELAY) == 0)) {
        if (!ConvertStrToInt(value, defaultModemDendDelay_)) {
            TELEPHONY_LOGE("invalid value: %{public}s", value);
        }
    } else {
        TELEPHONY_LOGI("invalid key: %{public}s", key);
    }
    TELEPHONY_LOGI("prop change: allow=%{public}d, delay=%{public}d,%{public}d", isPropOn_, defaultSetupFailDelay_,
        defaultModemDendDelay_);
}

DisConnectionReason ConnectionRetryPolicy::ConvertPdpErrorToDisconnReason(int32_t reason)
{
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (isPropOn_ && TELEPHONY_EXT_WRAPPER.convertPdpError_) {
        reason = TELEPHONY_EXT_WRAPPER.convertPdpError_(reason);
    }
#endif
    switch (reason) {
        case PdpErrorReason::PDP_ERR_TO_NORMAL:
            return DisConnectionReason::REASON_NORMAL;
        case PdpErrorReason::PDP_ERR_TO_GSM_AND_CALLING_ONLY:
            return DisConnectionReason::REASON_GSM_AND_CALLING_ONLY;
        case PdpErrorReason::PDP_ERR_TO_CLEAR_CONNECTION:
            return DisConnectionReason::REASON_CLEAR_CONNECTION;
        case PdpErrorReason::PDP_ERR_TO_CHANGE_CONNECTION:
            return DisConnectionReason::REASON_CHANGE_CONNECTION;
        case PdpErrorReason::PDP_ERR_TO_PERMANENT_REJECT:
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

bool ConnectionRetryPolicy::ConvertStrToInt(const std::string& str, int32_t& value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, BASE_10);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

void ConnectionRetryPolicy::RestartRadioIfRequired(int32_t failCause, int32_t slotId)
{
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (isPropOn_ && TELEPHONY_EXT_WRAPPER.restartRadioIfRequired_) {
        TELEPHONY_EXT_WRAPPER.restartRadioIfRequired_(failCause, slotId);
    }
#endif
}
} // namespace Telephony
} // namespace OHOS