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

#ifndef NAPI_CELLULAR_DATA_H
#define NAPI_CELLULAR_DATA_H

#include <stdint.h>

#include "base_context.h"
#include "telephony_napi_common_error.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
struct AsyncContext : BaseContext {
    int32_t slotId = 0;
    int32_t result = ERROR_SERVICE_UNAVAILABLE;
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_CELLULAR_DATA_H
