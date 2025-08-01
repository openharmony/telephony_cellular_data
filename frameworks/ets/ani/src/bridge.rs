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

use ani_rs::ani;

#[ani_rs::ani(path = "L@ohos/telephony/data/data/DataConnectState")]
#[repr(i32)]
pub enum DataConnectState {
    DataStateUnknown = -1,

    DataStateDisconnected = 0,

    DataStateConnecting = 1,

    DataStateConnected = 2,

    DataStateSuspended = 3,
}

#[ani_rs::ani(path = "L@ohos/telephony/data/data/DataFlowType")]
#[repr(i32)]
pub enum DataFlowType {
    DataFlowTypeNone = 0,
    DataFlowTypeDown = 1,
    DataFlowTypeUp = 2,
    DataFlowTypeUpDown = 3,
    DataFlowTypeDormant = 4,
}

#[ani_rs::ani(path = "L@ohos/telephony/data/data/ApnInfo")]
pub struct ApnInfo {
    pub apnName: String,
    pub apn: String,
    pub mcc: String,
    pub mnc: String,
    pub user: Option<String>,
    pub type_: Option<String>,
    pub proxy: Option<String>,
    pub mmsproxy: Option<String>,
}
