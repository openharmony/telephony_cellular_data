/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "adddatatoken_fuzzer.h"

#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
const int PERMS_NUM = 3;

AddDataTokenFuzzer::AddDataTokenFuzzer()
{
    const char *perms[PERMS_NUM] = {
        "ohos.permission.GET_NETWORK_INFO",
        "ohos.permission.GET_TELEPHONY_STATE",
        "ohos.permission.SET_TELEPHONY_STATE",
    };

    NativeTokenInfoParams testDataInfoParams = {
        .dcapsNum = 0,
        .permsNum = PERMS_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "data_fuzzer",
        .aplStr = "system_basic",
    };
    currentID_ = GetAccessTokenId(&testDataInfoParams);
    SetSelfTokenID(currentID_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
AddDataTokenFuzzer::~AddDataTokenFuzzer() {}
} // namespace OHOS