/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef TEL_CELLULAR_DATA_FFI_H
#define TEL_CELLULAR_DATA_FFI_H

#include "ffi_remote_data.h"
#include "tel_cellular_data_utils.h"
#include "tel_cellular_data_impl.h"

namespace OHOS {
namespace Telephony {
extern "C" {
    FFI_EXPORT int32_t FfiCellularDataGetDefaultCellularDataSlotId();
    FFI_EXPORT int32_t FfiCellularDataGetCellularDataFlowType(int32_t *errCode);
    FFI_EXPORT int32_t FfiCellularDataGetCellularDataState(int32_t *errCode);
    FFI_EXPORT bool FfiCellularDataIsCellularDataEnabled(int32_t *errCode);
    FFI_EXPORT bool FfiCellularDataIsCellularDataRoamingEnabled(int32_t slotId, int32_t *errCode);
    FFI_EXPORT int32_t FfiCellularDataGetDefaultCellularDataSimId();
}
}
}

#endif