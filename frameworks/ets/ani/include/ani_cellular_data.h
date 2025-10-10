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

ArktsError isCellularDataEnabled(bool &dataEnabled);
ArktsError enableCellularDataSync();
ArktsError disableCellularDataSync();
int32_t getDefaultCellularDataSlotIdSync();
ArktsError getCellularDataState(int32_t &CellularDataState);

} // namespace CellularDataAni
} // namespace OHOS
#endif
