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
int32_t NetManagerCallBack::RequestNetwork(const std::string &ident,
                                           const std::set<NetCap> &netCaps,
                                           const NetManagerStandard::NetRequest &netrequest)
{
    if (netCaps.empty()) {
        TELEPHONY_LOGI("netCaps is empty[%{public}s]", ident.c_str());
        return CELLULAR_DATA_INVALID_PARAM;
    }
    NetRequest request;
    for (const auto &netCap : netCaps) {
        request.capability |= 1L << netCap;
    }
    request.ident = ident;
    request.registerType = netrequest.registerType;
    request.uid = netrequest.uid;
    for (const auto &netBearType : netrequest.bearTypes) {
        request.bearTypes |= 1L << netBearType;
    }
    int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().RequestNet(request);
    return result;
}

int32_t NetManagerCallBack::ReleaseNetwork(const NetManagerStandard::NetRequest &netrequest)
{
    if (netrequest.netCaps.empty()) {
        TELEPHONY_LOGE("netCaps is empty[%{public}s]", netrequest.ident.c_str());
        return CELLULAR_DATA_INVALID_PARAM;
    }
    NetRequest request;
    for (const auto &netCap : netrequest.netCaps) {
        request.capability |= 1L << netCap;
    }
    request.uid = netrequest.uid;
    request.ident = netrequest.ident;
    request.requestId = netrequest.requestId;
    int32_t result = DelayedRefSingleton<CellularDataService>::GetInstance().RemoveUid(request);
    if (result != static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)) {
        TELEPHONY_LOGD("RemoveUid Request Fail");
        return result;
    }
    if (netrequest.isRemoveUid != 0) {
        return result;
    }
    result = DelayedRefSingleton<CellularDataService>::GetInstance().ReleaseNet(request);
    return result;
}

int32_t NetManagerCallBack::AddRequest(const NetManagerStandard::NetRequest &netrequest)
{
    NetRequest request;
    request.ident = netrequest.ident;
    for (const auto &netCap : netrequest.netCaps) {
        request.capability |= 1L << netCap;
    }
    request.uid = netrequest.uid;
    request.requestId = netrequest.requestId;
    return DelayedRefSingleton<CellularDataService>::GetInstance().AddUid(request);
}

} // namespace Telephony
} // namespace OHOS