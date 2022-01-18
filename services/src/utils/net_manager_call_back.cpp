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

#include "net_manager_call_back.h"

#include "cellular_data_constant.h"
#include "cellular_data_error.h"
#include "cellular_data_service.h"
#include "cellular_data_types.h"

namespace OHOS {
namespace Telephony {
int32_t NetManagerCallBack::RequestNetwork(const std::string &ident, const std::set<NetCap> &netCaps)
{
    if (netCaps.empty()) {
        TELEPHONY_LOGI("netCaps is empty[%{public}s]", ident.c_str());
        return CELLULAR_DATA_INVALID_PARAM;
    }
    NetRequest request;
    request.capability = *netCaps.begin();
    request.ident = ident;
    int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().RequestNet(request);
    return result;
}

int32_t NetManagerCallBack::ReleaseNetwork(const std::string &ident, const std::set<NetCap> &netCaps)
{
    if (netCaps.empty()) {
        TELEPHONY_LOGI("netCaps is empty[%{public}s]", ident.c_str());
        return CELLULAR_DATA_INVALID_PARAM;
    }
    NetRequest request;
    request.capability = *netCaps.begin();
    request.ident = ident;
    int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().ReleaseNet(request);
    return result;
}
} // namespace Telephony
} // namespace OHOS