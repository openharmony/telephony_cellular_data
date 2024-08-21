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

#include "tel_cellular_data_ffi.h"
#include "tel_cellular_data_impl.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace Telephony {
extern "C" {

    int32_t FfiCellularDataGetDefaultCellularDataSlotId()
    {
        return CellularDataImpl::GetDefaultCellularDataSlotId();
    }

    int32_t FfiCellularDataGetCellularDataFlowType(int32_t *errCode)
    {
        return CellularDataImpl::GetCellularDataFlowType(*errCode);
    }

    int32_t FfiCellularDataGetCellularDataState(int32_t *errCode)
    {
        return CellularDataImpl::GetCellularDataState(*errCode);
    }

    bool FfiCellularDataIsCellularDataEnabled(int32_t *errCode)
    {
        return CellularDataImpl::IsCellularDataEnabled(*errCode);
    }

    bool FfiCellularDataIsCellularDataRoamingEnabled(int32_t slotId, int32_t *errCode)
    {
        return CellularDataImpl::IsCellularDataRoamingEnabled(slotId, *errCode);
    }

    int32_t FfiCellularDataGetDefaultCellularDataSimId()
    {
        return CellularDataImpl::GetDefaultCellularDataSimId();
    }
}
}  // namespace Telephony
}  // namespace OHOS
