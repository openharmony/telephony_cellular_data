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
#ifndef TEL_CELLULAR_DATA_IMPL_H
#define TEL_CELLULAR_DATA_IMPL_H

#include "tel_cellular_data_utils.h"

namespace OHOS {
namespace Telephony {

class CellularDataImpl {
public:
    static int32_t GetDefaultCellularDataSlotId();
    static int32_t GetCellularDataFlowType(int32_t &errCode);
    static int32_t GetCellularDataState(int32_t &errCode);
    static bool IsCellularDataEnabled(int32_t &errCode);
    static bool IsCellularDataRoamingEnabled(int32_t slotId, int32_t &errCode);
    static int32_t GetDefaultCellularDataSimId();
};
}
}

#endif // TELEPHONY_CALL_IMPL_H