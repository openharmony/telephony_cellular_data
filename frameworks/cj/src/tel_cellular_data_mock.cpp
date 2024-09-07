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

#include "cj_common_ffi.h"

extern "C" {
FFI_EXPORT int FfiCellularDataGetDefaultCellularDataSlotId = 0;
FFI_EXPORT int FfiCellularDataGetCellularDataFlowType = 0;
FFI_EXPORT int FfiCellularDataGetCellularDataState = 0;
FFI_EXPORT int FfiCellularDataIsCellularDataEnabled = 0;
FFI_EXPORT int FfiCellularDataIsCellularDataRoamingEnabled = 0;
FFI_EXPORT int FfiCellularDataGetDefaultCellularDataSimId = 0;
}