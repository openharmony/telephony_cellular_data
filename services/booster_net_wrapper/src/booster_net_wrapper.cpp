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
#include "booster_net_wrapper.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
namespace {
const std::string BOOSTER_NET_WRAPPER_PATH = "libradio_boosternet.z.so";
} // namespace

BoosterNetWrapper::BoosterNetWrapper() {}
BoosterNetWrapper::~BoosterNetWrapper()
{
    TELEPHONY_LOGD("BoosterNetWrapper::~BoosterNetWrapper() start");
    dlclose(boosterNetWrapperHandle_);
    boosterNetWrapperHandle_ = nullptr;
}

void BoosterNetWrapper::InitBoosterNetWrapper()
{
    TELEPHONY_LOGD("BoosterNetWrapper::InitBoosterNetWrapper() start");
    boosterNetWrapperHandle_ = dlopen(BOOSTER_NET_WRAPPER_PATH.c_str(), RTLD_NOW);
    if (boosterNetWrapperHandle_ == nullptr) {
        TELEPHONY_LOGE("libradio_boosternet.z.so was not loaded, error: %{public}s", dlerror());
        return;
    }

    sendGetTcpSumToKernel_ = (SEND_GET_TCP_SUM_TO_KERNEL)dlsym(boosterNetWrapperHandle_, "SendGetTcpSumToKernel");
    // Check whether all function pointers are empty.
    if (sendGetTcpSumToKernel_ == nullptr) {
        TELEPHONY_LOGE("booster net wrapper symbol failed, error: %{public}s", dlerror());
        return;
    }
    TELEPHONY_LOGI("booster net wrapper init success");
}
} // namespace Telephony
} // namespace OHOS