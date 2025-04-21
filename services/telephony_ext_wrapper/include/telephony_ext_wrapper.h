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
    typedef bool (*IS_VSIM_IN_DISABLE_PROCESS)(void);
    /* add for vsim end */
    typedef void (*SEND_DATA_SWITCH_CHANGE_INFO)(const char*, int32_t, bool);
    typedef bool (*IS_ALL_CELLULAR_DATA_ALLOWED)(
        const NetRequest &, const HasSystemUse hasSystemUse);
    typedef bool (*IS_DUAL_CELLULAR_CARD_ALLOWED)();
    typedef int64_t (*HANDLE_DEND_FAILCAUSE)(int32_t, int64_t);
    typedef int32_t (*CONVERT_PDP_ERROR)(int32_t);
    typedef void (*RESTART_RADIO_IF_RQUIRED)(int32_t, int32_t);
    typedef bool (*GET_USER_DATA_ROAMING_EXPEND)(int32_t, bool);
    typedef void (*SEND_APN_NEED_RETRY_INFO)(int32_t);
    typedef bool (*JUDGE_OTHER_REQUEST_HOLDING)(const NetRequest &, const HasSystemUse hasSystemUse);
    typedef void (*DynamicLoadInit)(void);
    typedef void (*NotifyReqCellularData)(bool isReqCellularData);

    DATA_EDN_SELF_CURE dataEndSelfCure_ = nullptr;
    IS_APN_ALLOWED_ACTIVE isApnAllowedActive_ = nullptr;
    GET_VSIM_SLOT_ID getVSimSlotId_ = nullptr;
    CREATE_ALL_APN_ITEM_EXT createAllApnItemExt_ = nullptr;
    IS_CARD_ALLOW_DATA isCardAllowData_ = nullptr;
    IS_VSIM_ENABLED isVSimEnabled_ = nullptr;
    IS_VSIM_IN_DISABLE_PROCESS isVSimInDisableProcess_ = nullptr;
    SEND_DATA_SWITCH_CHANGE_INFO sendDataSwitchChangeInfo_ = nullptr;
    IS_ALL_CELLULAR_DATA_ALLOWED isAllCellularDataAllowed_ = nullptr;
    IS_DUAL_CELLULAR_CARD_ALLOWED isDualCellularCardAllowed_ = nullptr;
    GET_USER_DATA_ROAMING_EXPEND getUserDataRoamingExpend_ = nullptr;
    SEND_APN_NEED_RETRY_INFO sendApnNeedRetryInfo_ = nullptr;
    HANDLE_DEND_FAILCAUSE handleDendFailcause_ = nullptr;
    CONVERT_PDP_ERROR convertPdpError_ = nullptr;
    RESTART_RADIO_IF_RQUIRED restartRadioIfRequired_ = nullptr;
    JUDGE_OTHER_REQUEST_HOLDING judgeOtherRequestHolding_ = nullptr;
    DynamicLoadInit dynamicLoadInit_ = nullptr;
    NotifyReqCellularData dynamicLoadNotifyReqCellularDataStatus_ = nullptr;

private:
    void* telephonyExtWrapperHandle_ = nullptr;
    void* telephonyVSimWrapperHandle_ = nullptr;
    void* telephonyDynamicLoadWrapperHandle_ = nullptr;

    void InitTelephonyExtWrapperForCellularData();
    void InitDataEndSelfCure();
    void InitTelephonyExtForCustomization();
    void InitTelephonyExtWrapperForVSim();
    void InitSendDataSwitchChangeInfo();
    void InitIsAllCellularDataAllowed();
    void InitIsDualCellularCardAllowed();
    void InitHandleDendFailcause();
    void InitConvertPdpError();
    void InitRestartRadioIfRequired();
    void InitSendApnNeedRetryInfo();
    void InitJudgeOtherRequestHolding();
    void InitTelephonyExtWrapperForDynamicLoad();
};

#define TELEPHONY_EXT_WRAPPER ::OHOS::DelayedRefSingleton<TelephonyExtWrapper>::GetInstance()
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_EXT_WRAPPER_H
