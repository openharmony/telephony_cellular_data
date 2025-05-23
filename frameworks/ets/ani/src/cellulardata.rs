// Copyright (c) 2023 Huawei Device Co., Ltd.
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

use crate::wrapper;

#[ani_rs::ani(path = "L@ohos/telephony/data/data/DataConnectState")]
#[repr(i32)]
pub enum DataConnectState {
    DataStateUnknown = -1,

    DataStateDisconnected = 0,

    DataStateConnecting = 1,

    DataStateConnected = 2,

    DataStateSuspended = 3
}

impl From<i32> for DataConnectState {
    fn from(value: i32) -> Self {
        match value {
            0 => DataConnectState::DataStateDisconnected,
            1 => DataConnectState::DataStateConnecting,
            2 => DataConnectState::DataStateConnected,
            3 => DataConnectState::DataStateSuspended,
            _ => DataConnectState::DataStateUnknown,
        }
    }
}

#[ani_rs::native]
pub fn is_cellulardata_enabled_sync() -> Result<bool, BusinessError> {
    let mut data_enabled = false;
    let arkts_error = wrapper::ffi::isCellularDataEnabled(&mut data_enabled);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error))
    }

    Ok(data_enabled)
}

#[ani_rs::native]
pub fn enable_cellular_data_sync() -> Result<(), BusinessError> {
    let arkts_error = wrapper::ffi::enableCellularDataSync();
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error))
    }

    Ok(())
}

#[ani_rs::native]
pub fn disable_cellular_data_sync() -> Result<(), BusinessError> {
    let arkts_error = wrapper::ffi::disableCellularDataSync();
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error))
    }

    Ok(())
}

#[ani_rs::native]
pub fn get_default_cellular_data_slot_id_sync() -> Result<i32, BusinessError> {
    let slot_id = wrapper::ffi::getDefaultCellularDataSlotIdSync();
    Ok(slot_id)   
}

#[ani_rs::native]
pub fn get_cellular_data_state() -> Result<DataConnectState, BusinessError> {
    let mut cellular_data_state = -1;
    let arkts_error = wrapper::ffi::getCellularDataState(&mut cellular_data_state);
    if arkts_error.is_error() {
        return Err(BusinessError::from(arkts_error))
    }

    Ok(DataConnectState::from(cellular_data_state))
}