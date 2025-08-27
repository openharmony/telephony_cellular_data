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

use crate::bridge;
use ani_rs::business_error::BusinessError;
use ffi::ArktsError;

pub const TELEPHONY_SUCCESS: i32 = 8300000;

impl From<ffi::ApnInfo> for bridge::ApnInfo {
    fn from(handle: ffi::ApnInfo) -> Self {
        bridge::ApnInfo {
            apnName: handle.apnName,
            apn: handle.apn,
            mcc: handle.mcc,
            mnc: handle.mnc,
            user: Some(handle.user),
            type_: Some(handle.type_),
            proxy: Some(handle.proxy),
            mmsproxy: Some(handle.mmsproxy),
        }
    }
}

impl From<bridge::ApnInfo> for ffi::ApnInfo {
    fn from(handle: bridge::ApnInfo) -> Self {
        ffi::ApnInfo {
            apnName: handle.apnName,
            apn: handle.apn,
            mcc: handle.mcc,
            mnc: handle.mnc,
            user: handle.user.unwrap_or_default(),
            type_: handle.type_.unwrap_or_default(),
            proxy: handle.proxy.unwrap_or_default(),
            mmsproxy: handle.mmsproxy.unwrap_or_default(),
        }
    }
}

#[cxx::bridge(namespace = "OHOS::CellularDataAni")]
pub mod ffi {
    struct ArktsError {
        errorCode: i32,
        errorMessage: String,
    }

    pub struct ApnInfo {
        pub apnName: String,
        pub apn: String,
        pub mcc: String,
        pub mnc: String,
        pub user: String,
        pub type_: String,
        pub proxy: String,
        pub mmsproxy: String,
    }

    unsafe extern "C++" {
        include!("ani_cellular_data.h");

        fn isCellularDataEnabled(dataEnabled: &mut bool) -> ArktsError;
        fn enableCellularDataSync() -> ArktsError;
        fn disableCellularDataSync() -> ArktsError;
        fn getDefaultCellularDataSlotIdSync() -> i32;
        fn getCellularDataState(cellular_data_state: &mut i32) -> ArktsError;
        fn disableCellularDataRoamingSync(slotId: i32) -> ArktsError;
        fn enableCellularDataRoamingSync(slotId: i32) -> ArktsError;
        fn isCellularDataRoamingEnabledSync(slotId: i32, ret: &mut bool) -> ArktsError;
        fn setDefaultCellularDataSlotIdSyn(slotId: i32) -> ArktsError;
        fn setPreferredApnSyn(apnId: i32, ret: &mut bool) -> ArktsError;
        fn getDefaultCellularDataSimIdSyn() -> i32;
        fn getCellularDataFlowTypeSyn() -> i32;
        fn queryApnIdsSync(info: &ApnInfo, ret: &mut Vec<u32>) -> ArktsError;
        fn queryAllApnsSync(ret: &mut Vec<ApnInfo>) -> ArktsError;
        fn getActiveApnNameSync(ret: &mut String) -> ArktsError;
    }
}

impl ArktsError {
    pub fn is_error(&self) -> bool {
        if self.errorCode != TELEPHONY_SUCCESS {
            return true;
        }
        false
    }
}

impl From<ArktsError> for BusinessError {
    fn from(value: ArktsError) -> Self {
        BusinessError::new(value.errorCode, value.errorMessage)
    }
}
