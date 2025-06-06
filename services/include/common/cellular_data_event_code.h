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

#ifndef CELLULAR_DATA_EVENT_CODE_H
#define CELLULAR_DATA_EVENT_CODE_H

namespace OHOS {
namespace Telephony {
class CellularDataEventCode {
public:
    /**
     * The event code that drives the state transition
     */
    static const int32_t BASE = 0x00040000;
    static const int32_t MSG_SM_CONNECT = BASE + 0;
    static const int32_t MSG_SM_DISCONNECT = BASE + 1;
    static const int32_t MSG_SM_DISCONNECT_ALL = BASE + 2;
    static const int32_t MSG_SM_TEAR_DOWN_NOW = BASE + 3;
    static const int32_t MSG_SM_LOST_CONNECTION = BASE + 4;
    static const int32_t MSG_SM_DRS_OR_RAT_CHANGED = BASE + 5;
    static const int32_t MSG_SM_DATA_ROAM_ON = BASE + 6;
    static const int32_t MSG_SM_DATA_ROAM_OFF = BASE + 7;
    static const int32_t MSG_SM_BW_REFRESH_RESPONSE = BASE + 8;
    static const int32_t MSG_SM_VOICE_CALL_STARTED = BASE + 9;
    static const int32_t MSG_SM_VOICE_CALL_ENDED = BASE + 10;
    static const int32_t MSG_SM_GET_LAST_FAIL_DONE = BASE + 11;
    static const int32_t MSG_ESTABLISH_DATA_CONNECTION = BASE + 12;
    static const int32_t MSG_ESTABLISH_DATA_CONNECTION_COMPLETE = BASE + 13;
    static const int32_t MSG_DISCONNECT_DATA_COMPLETE = BASE + 14;
    static const int32_t MSG_STATE_MACHINE_INIT = BASE + 15;
    static const int32_t MSG_STATE_MACHINE_QUIT = BASE + 16;
    static const int32_t MSG_SETTING_SWITCH = BASE + 17;
    static const uint32_t MSG_ASYNCHRONOUS_REGISTER_EVENT_ID = BASE + 18;
    static const uint32_t MSG_RUN_MONITOR_TASK = BASE + 19;
    static const uint32_t MSG_STALL_DETECTION_EVENT_ID = BASE + 20;
    static const uint32_t MSG_REQUEST_NETWORK = BASE + 21;
    static const uint32_t MSG_APN_CHANGED = BASE + 22;
    static const uint32_t MSG_SET_RIL_ATTACH_APN = BASE + 23;
    static const uint32_t MSG_GET_RIL_BANDWIDTH = BASE + 24;
    static const uint32_t MSG_SET_RIL_BANDWIDTH = BASE + 25;
    static const uint32_t MSG_CONNECT_TIMEOUT_CHECK = BASE + 26;
    static const uint32_t MSG_DISCONNECT_TIMEOUT_CHECK = BASE + 27;
    static const uint32_t MSG_DB_SETTING_ENABLE_CHANGED = BASE + 28;
    static const uint32_t MSG_DB_SETTING_ROAMING_CHANGED = BASE + 29;
    static const uint32_t MSG_SET_DATA_PERMITTED = BASE + 30;
    static const uint32_t MSG_SM_LINK_CAPABILITY_CHANGED = BASE + 31;
    static const uint32_t MSG_DB_SETTING_INCALL_CHANGED = BASE + 32;
    static const uint32_t MSG_SM_INCALL_DATA_SETTINGS_ON = BASE + 33;
    static const uint32_t MSG_SM_INCALL_DATA_SETTINGS_OFF = BASE + 34;
    static const uint32_t MSG_SM_INCALL_DATA_CALL_STARTED = BASE + 35;
    static const uint32_t MSG_SM_INCALL_DATA_CALL_ENDED = BASE + 36;
    static const uint32_t MSG_SM_INCALL_DATA_DATA_DISCONNECTED = BASE + 37;
    static const uint32_t MSG_SM_INCALL_DATA_DATA_CONNECTED = BASE + 38;
    static const uint32_t MSG_SM_INCALL_DATA_DSDS_CHANGED = BASE + 39;
    static const uint32_t MSG_INCALL_DATA_COMPLETE = BASE + 40;
    static const uint32_t MSG_SM_RIL_ADAPTER_HOST_DIED = BASE + 41;
    static const uint32_t MSG_DATA_CALL_LIST_CHANGED = BASE + 42;
    static const uint32_t MSG_RETRY_TO_SETUP_DATACALL = BASE + 43;
    static const uint32_t MSG_SM_UPDATE_NETWORK_INFO = BASE + 44;
    static const uint32_t MSG_ESTABLISH_ALL_APNS_IF_CONNECTABLE = BASE + 45;
    static const uint32_t MSG_RESUME_DATA_PERMITTED_TIMEOUT = BASE + 46;
    static const uint32_t MSG_SEND_UEPOLICY_COMPLETE = BASE + 47;
    static const uint32_t MSG_SEND_UEPOLICY_COMMAND_REJECT = BASE + 48;
    static const uint32_t MSG_SEND_UE_STATE_INDICATION = BASE + 49;
    static const uint32_t MSG_SEND_IMS_RSDLIST = BASE + 50;
    static const uint32_t MSG_SYNC_ALLOWED_NSSAI_WITH_MODEM = BASE + 51;
    static const uint32_t MSG_SYNC_EHPLMN_WITH_MODEM = BASE + 52;
    static const uint32_t MSG_RETRY_TO_CREATE_APN = BASE + 53;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_EVENT_CODE_H
