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

#include "cellular_data_dump_helper.h"

#include "cellular_data_service.h"
#include "core_service_client.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
CellularDataDumpHelper::CellularDataDumpHelper() {}

bool CellularDataDumpHelper::Dump(const std::vector<std::string> &args, std::string &result) const
{
    result.clear();
    for (int32_t i = 0; i < (int32_t)args.size() - 1; i++) {
        if (args[i] == "cellular_data" && args[i + 1] == "--help") {
            ShowHelp(result);
            return true;
        }
    }
    ShowCellularDataInfo(result);
    return true;
}

bool CellularDataDumpHelper::WhetherHasSimCard(const int32_t slotId) const
{
    return DelayedRefSingleton<CoreServiceClient>::GetInstance().HasSimCard(slotId);
}

void CellularDataDumpHelper::ShowHelp(std::string &result) const
{
    result.append("CellularData:\n");
    result.append("Usage:dump <command> [options]\n");
    result.append("Description:\n");
    result.append("-cellular_data_info          ");
    result.append("dump all cellular_data information in the system\n");
    result.append("-output_slot        ");
    result.append("default_slot_Id\n");
    result.append("-output_state_machine");
    result.append("Output state machine information and status\n");
    result.append("-dataflow_info");
    result.append("output_data_flow_info\n");
    result.append("-show_log_level        ");
    result.append("show cellular_data SA's log level\n");
    result.append("-set_log_level <level>     ");
    result.append("set cellular_data SA's log level\n");
    result.append("-perf_dump         ");
    result.append("dump performance statistics\n");
}

void CellularDataDumpHelper::ShowCellularDataInfo(std::string &result) const
{
    CellularDataService &dataService = DelayedRefSingleton<CellularDataService>::GetInstance();
    result.append("Ohos cellular data service: \n");
    result.append("Ohos cellular data bind time:  ");
    result.append(dataService.GetBindTime());
    result.append("\n");
    result.append("Ohos cellular data end time:  ");
    result.append(dataService.GetEndTime());
    result.append("\n");
    result.append("Ohos cellular data spend time:  ");
    result.append(std::to_string(dataService.GetSpendTime()));
    result.append("\n");
    result.append("Ohos cellular data default slot id:  ");
    result.append(dataService.GetCellularDataSlotIdDump());
    result.append("\n");
    result.append("Ohos cellular data output state machine current status:  ");
    result.append(dataService.GetStateMachineCurrentStatusDump());
    result.append("\n");
    result.append("Ohos cellular data output dataflow info: ");
    result.append(dataService.GetFlowDataInfoDump());
    result.append("\n");
    result.append("ServiceRunningState          : ");
    result.append(std::to_string(dataService.GetServiceRunningState()));
    result.append("\n");
    for (int32_t i = 0; i < SIM_SLOT_COUNT; i++) {
        if (WhetherHasSimCard(i)) {
            result.append("SlotId                       : ");
            result.append(std::to_string(i));
            result.append("\n");
            result.append("VoiceActivationState         : ");
            result.append(std::to_string(dataService.IsCellularDataRoamingEnabled(i)));
            result.append("\n");
        }
    }
    result.append("DataConnectionState          : ");
    result.append(std::to_string(dataService.IsCellularDataEnabled()));
    result.append("\n");
    result.append("CellularDataConnectionState  : ");
    result.append(std::to_string(dataService.GetCellularDataState()));
    result.append("\n");
}
} // namespace Telephony
} // namespace OHOS