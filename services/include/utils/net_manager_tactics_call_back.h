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

#ifndef NET_MANAGER_TACTICS_CALL_BACK_H
#define NET_MANAGER_TACTICS_CALL_BACK_H

#include "net_policy_callback_stub.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;
class NetManagerTacticsCallBack : public NetPolicyCallbackStub {
private:
    int32_t NetStrategySwitch(const std::string &simId, bool enable) override;
    bool ConvertStrToInt(const std::string& str, int32_t& value);
};
} // Telephony
} // OHOS
#endif // NET_MANAGER_TACTICS_CALL_BACK_H
