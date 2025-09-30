/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef ANI_CELLULAR_DATA_H
#define ANI_CELLULAR_DATA_H

#include <cstdint>
#include "cxx.h"

namespace OHOS {
namespace CellularDataAni {
struct ArktsError;
struct ApnInfo;

ArktsError IsCellularDataEnabled(bool &dataEnabled);
ArktsError EnableCellularDataSync();
ArktsError DisableCellularDataSync();
int32_t GetDefaultCellularDataSlotIdSync();
ArktsError GetCellularDataState(int32_t &CellularDataState);
ArktsError DisableCellularDataRoamingSync(int32_t slotId);
ArktsError EnableCellularDataRoamingSync(int32_t slotId);
ArktsError IsCellularDataRoamingEnabledSync(int32_t slotId, bool &dataEnabled);
ArktsError SetDefaultCellularDataSlotIdSyn(int32_t slotId);
int32_t GetCellularDataFlowTypeSyn();
ArktsError SetPreferredApnSyn(int32_t apnId, bool &ret);
int32_t GetDefaultCellularDataSimIdSyn();
ArktsError QueryApnIdsSync(const ApnInfo &info, rust::vec<uint32_t> &ret);
ArktsError QueryAllApnsSync(rust::vec<ApnInfo> &ret);
ArktsError GetActiveApnNameSync(rust::String &apnName);
} // namespace CellularDataAni
} // namespace OHOS
#endif
