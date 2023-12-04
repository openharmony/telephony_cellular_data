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
#include "data_service_ext_wrapper.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
namespace {
const std::string DATA_SERVICE_EXT_WRAPPER_PATH = "libradio_boosternet.z.so";
} // namespace

DataServiceExtWrapper::DataServiceExtWrapper() {}
DataServiceExtWrapper::~DataServiceExtWrapper()
{
    TELEPHONY_LOGD("DataServiceExtWrapper::~DataServiceExtWrapper() start");
    dlclose(DataServiceExtWrapperHandle_);
    DataServiceExtWrapperHandle_ = nullptr;
}

void DataServiceExtWrapper::InitDataServiceExtWrapper()
{
    TELEPHONY_LOGD("DataServiceExtWrapper::InitDataServiceExtWrapper() start");
    DataServiceExtWrapperHandle_ = dlopen(DATA_SERVICE_EXT_WRAPPER_PATH.c_str(), RTLD_NOW);
    if (DataServiceExtWrapperHandle_ == nullptr) {
        TELEPHONY_LOGE("DATA_SERVICE_EXT_WRAPPER_PATH was not loaded, error: %{public}s", dlerror());
        return;
    }

    requestTcpAndDnsPackets_ = (REQUEST_TCP_AND_DNS_PACKETS)dlsym(DataServiceExtWrapperHandle_, "SendGetTcpSumToKernel");
    // Check whether all function pointers are empty.
    if (requestTcpAndDnsPackets_ == nullptr) {
        TELEPHONY_LOGE("data service ext wrapper symbol failed, error: %{public}s", dlerror());
        return;
    }
    TELEPHONY_LOGI("data service ext wrapper init success");
}
} // namespace Telephony
} // namespace OHOS