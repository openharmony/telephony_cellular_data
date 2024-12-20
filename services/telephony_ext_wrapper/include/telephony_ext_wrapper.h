/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TELEPHONY_EXT_WRAPPER_H
#define TELEPHONY_EXT_WRAPPER_H

#include "nocopyable.h"
#include "singleton.h"

#include "apn_item.h"
#include "apn_holder.h"
#include "net_manager_call_back.h"
#include "net_supplier_callback_base.h"

namespace OHOS {
namespace Telephony {
class TelephonyExtWrapper final {
DECLARE_DELAYED_REF_SINGLETON(TelephonyExtWrapper);

public:
    DISALLOW_COPY_AND_MOVE(TelephonyExtWrapper);
    void InitTelephonyExtWrapper();

    typedef void (*DATA_EDN_SELF_CURE)(int32_t&, int32_t&);
    typedef bool (*IS_APN_ALLOWED_ACTIVE)(int32_t, const char*, bool);
    /* add for vsim begin */
    typedef void (*GET_VSIM_SLOT_ID)(int32_t &slotId);
    typedef bool (*CREATE_ALL_APN_ITEM_EXT)(int32_t slotId, sptr<ApnItem> &apnItem);
    typedef bool (*IS_CARD_ALLOW_DATA)(int32_t simId, int32_t capability);
    typedef bool (*IS_VSIM_ENABLED)(void);
    /* add for vsim end */
    typedef void (*SEND_DATA_SWITCH_CHANGE_INFO)(const char*, bool);
    typedef bool (*IS_ALL_CELLULAR_DATA_ALLOWED)(
        const NetRequest &, const HasSystemUse hasSystemUse);
    typedef bool (*IS_DUAL_CELLULAR_CARD_ALLOWED)();
    typedef void (*DATA_END_RETRY_STRATEGY)(int64_t&, int32_t&, int64_t&, int32_t&);
    DATA_EDN_SELF_CURE dataEndSelfCure_ = nullptr;
    IS_APN_ALLOWED_ACTIVE isApnAllowedActive_ = nullptr;
    GET_VSIM_SLOT_ID getVSimSlotId_ = nullptr;
    CREATE_ALL_APN_ITEM_EXT createAllApnItemExt_ = nullptr;
    IS_CARD_ALLOW_DATA isCardAllowData_ = nullptr;
    IS_VSIM_ENABLED isVSimEnabled_ = nullptr;
    SEND_DATA_SWITCH_CHANGE_INFO sendDataSwitchChangeInfo_ = nullptr;
    IS_ALL_CELLULAR_DATA_ALLOWED isAllCellularDataAllowed_ = nullptr;
    IS_DUAL_CELLULAR_CARD_ALLOWED isDualCellularCardAllowed_ = nullptr;
    DATA_END_RETRY_STRATEGY dataEndRetryStrategy_ = nullptr;
private:
    void* telephonyExtWrapperHandle_ = nullptr;
    void* telephonyVSimWrapperHandle_ = nullptr;

    void InitTelephonyExtWrapperForCellularData();
    void InitDataEndSelfCure();
    void InitIsApnAllowedActive();
    void InitTelephonyExtWrapperForVSim();
    void InitSendDataSwitchChangeInfo();
    void InitIsAllCellularDataAllowed();
    void InitIsDualCellularCardAllowed();
    void InitDataEndRetryStrategy();
};

#define TELEPHONY_EXT_WRAPPER ::OHOS::DelayedRefSingleton<TelephonyExtWrapper>::GetInstance()
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_EXT_WRAPPER_H