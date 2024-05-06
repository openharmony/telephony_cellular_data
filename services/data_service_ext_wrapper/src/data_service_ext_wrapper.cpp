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
#include "cellular_data_constant.h"
#include "parameter.h"

namespace OHOS {
namespace Telephony {

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
    char path[PATH_PARAMETER_SIZE] = {0};
    int retLen = GetParameter(CONFIG_DATA_SERVICE_EXT_PATH, "", path, PATH_PARAMETER_SIZE);
    if (retLen == 0) {
        TELEPHONY_LOGE("Failed to get vendor library path through system properties.");
        return;
    }
    DataServiceExtWrapperHandle_ = dlopen(path, RTLD_NOW);
    if (DataServiceExtWrapperHandle_ == nullptr) {
        TELEPHONY_LOGE("DATA_SERVICE_EXT_WRAPPER_PATH was not loaded, error: %{public}s", dlerror());
        return;
    }

    requestTcpAndDnsPackets_ = (REQUEST_TCP_AND_DNS_PACKETS)dlsym(DataServiceExtWrapperHandle_,
        "SendTcpPktCollecToKernel");
    // Check whether all function pointers are empty.
    if (requestTcpAndDnsPackets_ == nullptr) {
        TELEPHONY_LOGE("data service ext wrapper symbol failed, error: %{public}s", dlerror());
        return;
    }
    TELEPHONY_LOGI("data service ext wrapper init success");
}
} // namespace Telephony
} // namespace OHOS