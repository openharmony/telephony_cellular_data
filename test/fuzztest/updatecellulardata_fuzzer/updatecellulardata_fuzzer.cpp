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

#include "updatecellulardata_fuzzer.h"

#define private public
#define protected public

#include "adddatatoken_fuzzer.h"
#include "cellular_data_dump_helper.h"
#include "cellular_data_handler.h"
#include "cellular_data_incall_observer.h"


namespace OHOS {
using namespace OHOS::Telephony;
using namespace AppExecFwk;
using namespace OHOS::EventFwk;

void UpdateCellularDataDumpHelperFuzz(const uint8_t *data, size_t size)
{
    CellularDataDumpHelper help;
    std::string arg(reinterpret_cast<const char*>(data), size);
    std::vector<std::string> args;
    args.emplace_back(arg);
    std::string result = reinterpret_cast<const char*>(data);
    help.Dump(args, result);
    help.ShowHelp(result);
    help.ShowCellularDataInfo(result);
    std::shared_ptr<CellularDataHandler> cellularDataHandler = nullptr;
    auto cellularDataIncallObserver = std::make_shared<CellularDataIncallObserver>(cellularDataHandler);
    cellularDataIncallObserver->OnChange();
}

void UpdateActiveMachineWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    UpdateCellularDataDumpHelperFuzz(data, size);
    UpdateCellularDataIncallObserverFuzz(data, size);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AddDataTokenFuzzer token;
    /* Run your code on data */
    OHOS::UpdateActiveMachineWithMyAPI(data, size);
    return 0;
}