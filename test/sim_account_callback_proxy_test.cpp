/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#define private public
#define protected public

#include <gtest/gtest.h>
#include "sim_account_callback_proxy.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class SimAccountCallbackProxyTest : public testing::Test {
public:
    SimAccountCallbackProxyTest() = default;
    ~SimAccountCallbackProxyTest() = default;
};

HWTEST_F(SimAccountCallbackProxyTest, SimAccountCallbackProxy_01, Function | MediumTest | Level1)
{
    std::u16string des = SimAccountCallbackProxy::GetDescriptor();
    std::cout << "SimAccountCallbackProxy_01 Descriptor " << des.c_str() << std::endl;
    SimAccountCallbackProxy *proxy = new SimAccountCallbackProxy(nullptr);
    proxy->OnSimAccountChanged();
    ASSERT_FALSE(des.length() == 0);
}

} // namespace Telephony
} // namespace OHOS