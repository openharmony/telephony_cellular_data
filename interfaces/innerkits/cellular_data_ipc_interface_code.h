/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef CELLULAR_DATA_IPC_INTERFACE_CODE_H
#define CELLULAR_DATA_IPC_INTERFACE_CODE_H

/* SAID:4007 */
namespace OHOS {
namespace Telephony {
enum class CellularDataInterfaceCode {
    IS_CELLULAR_DATA_ENABLED = 0,
    ENABLE_CELLULAR_DATA,
    GET_CELLULAR_DATA_STATE,
    GET_CELLULAR_DATA_APN_STATE,
    GET_RECOVERY_STATE,
    IS_DATA_ROAMING_ENABLED,
    ENABLE_DATA_ROAMING,
    GET_DEFAULT_SLOT_ID,
    GET_DEFAULT_SIM_ID,
    SET_DEFAULT_SLOT_ID,
    GET_FLOW_TYPE_ID,
    HAS_CAPABILITY,
    CLEAR_ALL_CONNECTIONS,
    CLEAR_ALL_CONNECTIONS_USE_REASON,
    APN_DATA_CHANGED,
    REG_SIM_ACCOUNT_CALLBACK,
    UN_REG_SIM_ACCOUNT_CALLBACK,
    GET_DATA_CONN_APN_ATTR,
    GET_DATA_CONN_IP_TYPE,
    IS_NEED_DO_RECOVERY,
    ENABLE_INTELLIGENCE_SWITCH,
    INIT_CELLULAR_DATA_CONTROLLER,
    GET_INTELLIGENCE_SWITCH_STATE
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_IPC_INTERFACE_CODE_H