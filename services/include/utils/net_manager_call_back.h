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

#ifndef NET_MANAGER_CALL_BACK_H
#define NET_MANAGER_CALL_BACK_H

#include "net_supplier_callback_base.h"
#include "cellular_data_constant.h"
namespace OHOS {
namespace Telephony {

class NetManagerCallBack : public NetManagerStandard::NetSupplierCallbackBase {
public:
    int32_t RequestNetwork(const std::string &ident, const std::set<NetManagerStandard::NetCap> &netCaps,
        const NetManagerStandard::NetRequest &netrequest = {}) override;
    int32_t ReleaseNetwork(const std::string &ident, const std::set<NetManagerStandard::NetCap> &netCaps) override;
    int32_t AddRequest(const NetManagerStandard::NetRequest &netrequest) override;
    int32_t RemoveRequest(const NetManagerStandard::NetRequest &netrequest) override;
};
} // namespace Telephony
} // namespace OHOS
#endif // NET_MANAGER_CALL_BACK_H
