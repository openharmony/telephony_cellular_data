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

#include <dlfcn.h>
#include "telephony_ext_wrapper.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
namespace {
const std::string TELEPHONY_EXT_WRAPPER_PATH = "libtelephony_ext_service.z.so";
const std::string TELEPHONY_VSIM_WRAPPER_PATH = "libtel_vsim_symbol.z.so";
} // namespace

TelephonyExtWrapper::TelephonyExtWrapper() {}
TelephonyExtWrapper::~TelephonyExtWrapper()
{
    TELEPHONY_LOGD("TelephonyExtWrapper::~TelephonyExtWrapper() start");
    if (telephonyExtWrapperHandle_ != nullptr) {
        dlclose(telephonyExtWrapperHandle_);
        telephonyExtWrapperHandle_ = nullptr;
    }
    if (telephonyVSimWrapperHandle_ != nullptr) {
        dlclose(telephonyVSimWrapperHandle_);
        telephonyVSimWrapperHandle_ = nullptr;
    }
}

void TelephonyExtWrapper::InitTelephonyExtWrapper()
{
    TELEPHONY_LOGD("TelephonyExtWrapper::InitTelephonyExtWrapper() start");
    InitTelephonyExtWrapperForCellularData();
    InitTelephonyExtWrapperForVSim();
    TELEPHONY_LOGI("telephony ext wrapper init success");
}

void TelephonyExtWrapper::InitTelephonyExtWrapperForCellularData()
{
    telephonyExtWrapperHandle_ = dlopen(TELEPHONY_EXT_WRAPPER_PATH.c_str(), RTLD_NOW);
    if (telephonyExtWrapperHandle_ == nullptr) {
        TELEPHONY_LOGE("libtelephony_ext_service.z.so was not loaded, error: %{public}s", dlerror());
        return;
    }
    InitDataEndSelfCure();
    InitIsApnAllowedActive();
}

void TelephonyExtWrapper::InitDataEndSelfCure()
{
    dataEndSelfCure_ = (DATA_EDN_SELF_CURE)dlsym(telephonyExtWrapperHandle_, "DataEndSelfCure");
    if (dataEndSelfCure_ == nullptr) {
        TELEPHONY_LOGE("telephony ext wrapper symbol DataEndSelfCure failed, error: %{public}s", dlerror());
        return;
    }
    TELEPHONY_LOGD("telephony ext wrapper init DataEndSelfCure success");
}

void TelephonyExtWrapper::InitIsApnAllowedActive()
{
    isApnAllowedActive_ = (IS_APN_ALLOWED_ACTIVE)dlsym(telephonyExtWrapperHandle_, "IsApnAllowedActive");
    if (isApnAllowedActive_ == nullptr) {
        TELEPHONY_LOGE("telephony ext wrapper symbol InitIsApnAllowedActive failed, error: %{public}s", dlerror());
        return;
    }
    TELEPHONY_LOGD("telephony ext wrapper init InitIsApnAllowedActive success");
}

void TelephonyExtWrapper::InitTelephonyExtWrapperForVSim()
{
    TELEPHONY_LOGI("[VSIM] telephony ext wrapper init begin");
    telephonyVSimWrapperHandle_ = dlopen(TELEPHONY_VSIM_WRAPPER_PATH.c_str(), RTLD_NOW);
    if (telephonyVSimWrapperHandle_ == nullptr) {
        TELEPHONY_LOGE("libtel_vsim_symbol.z.so was not loaded, error: %{public}s", dlerror());
        return;
    }
    getVSimSlotId_ = (GET_VSIM_SLOT_ID) dlsym(telephonyVSimWrapperHandle_, "GetVSimSlotId");
    createAllApnItemExt_ = (CREATE_ALL_APN_ITEM_EXT) dlsym(telephonyVSimWrapperHandle_, "CreateAllApnItemExt");
    isCardAllowData_ = (IS_CARD_ALLOW_DATA) dlsym(telephonyVSimWrapperHandle_, "IsCardAllowData");
    if (getVSimSlotId_ == nullptr || createAllApnItemExt_ == nullptr || isCardAllowData_ == nullptr) {
        TELEPHONY_LOGE("[VSIM] telephony ext wrapper symbol failed, error: %{public}s", dlerror());
        return;
    }
    TELEPHONY_LOGI("[VSIM] telephony ext wrapper init success");
}

} // namespace Telephony
} // namespace OHOS