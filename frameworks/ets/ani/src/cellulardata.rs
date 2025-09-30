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

use ani_rs::business_error::BusinessError;

use crate::bridge;
use crate::wrapper;

impl From<i32> for bridge::DataConnectState {
    fn from(value: i32) -> Self {
        match value {
            0 => bridge::DataConnectState::DataStateDisconnected,
            1 => bridge::DataConnectState::DataStateConnecting,
            2 => bridge::DataConnectState::DataStateConnected,
            3 => bridge::DataConnectState::DataStateSuspended,
            _ => bridge::DataConnectState::DataStateUnknown,
        }
    }
}

impl From<i32> for bridge::DataFlowType {
    fn from(value: i32) -> Self {
        match value {
            0 => bridge::DataFlowType::DataFlowTypeNone,
            1 => bridge::DataFlowType::DataFlowTypeDown,
            2 => bridge::DataFlowType::DataFlowTypeUp,
            3 => bridge::DataFlowType::DataFlowTypeUpDown,
            4 => bridge::DataFlowType::DataFlowTypeDormant,
            _ => bridge::DataFlowType::DataFlowTypeNone,
        }
    }
}

#[ani_rs::native]
pub fn is_cellulardata_enabled_sync() -> Result<bool, BusinessError> {
    let mut data_enabled = false;
    let arkts_error = wrapper::ffi::IsCellularDataEnabled(&mut data_enabled);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(data_enabled)
}

#[ani_rs::native]
pub fn enable_cellular_data_sync() -> Result<(), BusinessError> {
    let arkts_error = wrapper::ffi::EnableCellularDataSync();
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(())
}

#[ani_rs::native]
pub fn disable_cellular_data_sync() -> Result<(), BusinessError> {
    let arkts_error = wrapper::ffi::DisableCellularDataSync();
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(())
}

#[ani_rs::native]
pub fn get_default_cellular_data_slot_id_sync() -> Result<i32, BusinessError> {
    let slot_id = wrapper::ffi::GetDefaultCellularDataSlotIdSync();
    Ok(slot_id)
}

#[ani_rs::native]
pub fn get_cellular_data_state() -> Result<bridge::DataConnectState, BusinessError> {
    let mut cellular_data_state = -1;
    let arkts_error = wrapper::ffi::GetCellularDataState(&mut cellular_data_state);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(bridge::DataConnectState::from(cellular_data_state))
}

#[ani_rs::native]
pub fn disable_cellular_data_roaming_sync(slotId: i32) -> Result<(), BusinessError> {
    let arkts_error = wrapper::ffi::DisableCellularDataRoamingSync(slotId);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(())
}

#[ani_rs::native]
pub fn enable_cellular_data_roaming_sync(slotId: i32) -> Result<(), BusinessError> {
    let arkts_error = wrapper::ffi::EnableCellularDataRoamingSync(slotId);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(())
}

#[ani_rs::native]
pub fn is_cellular_data_roaming_enabled_sync(slotId: i32) -> Result<bool, BusinessError> {
    let mut data_enabled = false;
    let arkts_error = wrapper::ffi::IsCellularDataRoamingEnabledSync(slotId, &mut data_enabled);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(data_enabled)
}

#[ani_rs::native]
pub fn set_default_cellular_data_slot_id_sync(slotId: i32) -> Result<(), BusinessError> {
    let arkts_error = wrapper::ffi::SetDefaultCellularDataSlotIdSyn(slotId);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(())
}

#[ani_rs::native]
pub fn get_cellular_data_flow_type_sync(
    slotId: i32,
) -> Result<bridge::DataFlowType, BusinessError> {
    let flowType = wrapper::ffi::GetCellularDataFlowTypeSyn();
    Ok(bridge::DataFlowType::from(flowType))
}

#[ani_rs::native]
pub fn set_preferred_apn_sync(apnId: i32) -> Result<bool, BusinessError> {
    let mut ret = false;
    let arkts_error = wrapper::ffi::SetPreferredApnSyn(apnId, &mut ret);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }

    Ok(ret)
}

#[ani_rs::native]
pub fn get_default_cellular_data_sim_id_sync() -> Result<i32, BusinessError> {
    let mut ret = wrapper::ffi::GetDefaultCellularDataSimIdSyn();
    Ok(ret)
}

#[ani_rs::native]
pub fn query_apn_ids_sync(info: bridge::ApnInfo) -> Result<Vec<u32>, BusinessError> {
    let mut ret: Vec<u32> = vec![];
    let arkts_error = wrapper::ffi::QueryApnIdsSync(&info.into(), &mut ret);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }
    Ok(ret)
}

#[ani_rs::native]
pub fn query_all_apns_sync() -> Result<Vec<bridge::ApnInfo>, BusinessError> {
    let mut ret: Vec<wrapper::ffi::ApnInfo> = vec![];
    let arkts_error = wrapper::ffi::QueryAllApnsSync(&mut ret);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }
    Ok(ret.into_iter().map(Into::into).collect())
}

#[ani_rs::native]
pub fn get_active_apn_name_sync() -> Result<String, BusinessError> {
    let mut ret = String::new();
    let arkts_error = wrapper::ffi::GetActiveApnNameSync(&mut ret);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error));
    }
    Ok(ret)
}