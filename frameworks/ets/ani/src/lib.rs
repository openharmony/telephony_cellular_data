// Copyright (c) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

mod bridge;
mod cellulardata;
mod wrapper;

use ani_rs::ani_constructor;

ani_constructor!(
    namespace "L@ohos/telephony/data/data"
    [
        "nativeIsCellularDataEnabled": cellulardata::is_cellulardata_enabled_sync,
        "nativeEnableCellularData": cellulardata::enable_cellular_data_sync,
        "nativeDisableCellularData": cellulardata::disable_cellular_data_sync,
        "nativeGetDefaultCellularDataSlotIdSync": cellulardata::get_default_cellular_data_slot_id_sync,
        "nativeGetCellularDataState": cellulardata::get_cellular_data_state,
        "nativeDisableCellularDataRoaming": cellulardata::disable_cellular_data_roaming_sync,
        "nativeEnableCellularDataRoaming": cellulardata::enable_cellular_data_roaming_sync,
        "nativeIsCellularDataRoamingEnabled": cellulardata::is_cellular_data_roaming_enabled_sync,
        "nativeSetDefaultCellularDataSlotId": cellulardata::set_default_cellular_data_slot_id_sync,
        "nativeGetCellularDataFlowType": cellulardata::get_cellular_data_flow_type_sync,
        "nativeSetPreferredApn": cellulardata::set_preferred_apn_sync,
        "nativeGetDefaultCellularDataSimId": cellulardata::get_default_cellular_data_sim_id_sync,
        "nativeQueryApnIds": cellulardata::query_apn_ids_sync,
        "nativeQueryAllApns": cellulardata::query_all_apns_sync,
        "nativeGetActiveApnName": cellulardata::get_active_apn_name_sync,
    ]
);
