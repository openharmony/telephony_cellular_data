/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef DATA_SIM_ACCOUNT_CALL_BACK_H
#define DATA_SIM_ACCOUNT_CALL_BACK_H

#include "sim_account_callback_stub.h"

namespace OHOS {
namespace Telephony {
class DataSimAccountCallback : public SimAccountCallbackStub {
public:
    void OnSimAccountChanged() override;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_SIM_ACCOUNT_CALL_BACK_H