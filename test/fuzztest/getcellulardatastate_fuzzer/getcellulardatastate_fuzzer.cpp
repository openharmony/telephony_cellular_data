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

#include "getcellulardatastate_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "adddatatoken_fuzzer.h"
#include "cellular_data_client.h"
#include "system_ability_definition.h"

using namespace OHOS::Telephony;
namespace OHOS {
void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= 0) {
        return;
    }

    int32_t slotId = static_cast<int32_t>(size);
    CellularDataClient::GetInstance().GetCellularDataState();
    CellularDataClient::GetInstance().IsCellularDataEnabled();
    CellularDataClient::GetInstance().IsCellularDataRoamingEnabled(slotId);
    CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, static_cast<int32_t>(size % 2));
    CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(slotId);
    CellularDataClient::GetInstance().HasInternetCapability(slotId, size);
    CellularDataClient::GetInstance().ClearCellularDataConnections(slotId);
    CellularDataClient::GetInstance().GetCellularDataFlowType();
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AddDataTokenFuzzer token;
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
