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

#ifndef CELLULAR_DATA_CONSTANT_H
#define CELLULAR_DATA_CONSTANT_H

#include <array>
#include <string>
#include <vector>

#include "cellular_data_types.h"

namespace OHOS {
namespace Telephony {
enum ApnProfileState {
    PROFILE_STATE_IDLE,
    PROFILE_STATE_CONNECTING,
    PROFILE_STATE_CONNECTED,
    PROFILE_STATE_DISCONNECTING,
    PROFILE_STATE_FAILED,
    PROFILE_STATE_RETRYING
};

enum class RecoveryState : int32_t {
    STATE_REQUEST_CONTEXT_LIST,
    STATE_CLEANUP_CONNECTIONS,
    STATE_REREGISTER_NETWORK,
    STATE_RADIO_STATUS_RESTART
};

struct AddressInfo {
    std::string ip = "";
    std::string netMask = "";
    uint8_t type = 0;
    uint8_t prefixLen = 0;
};

struct RouteInfo {
    std::string ip = "";
    uint8_t type = 0;
    std::string destination = "";
};

struct NetSupplier {
    uint32_t supplierId = 0;
    uint64_t capability = 0;
    int32_t slotId = 0;
    int32_t simId = 0;
    int32_t regState = -1;
};
enum RegisterType {
    UNKOWN,
    REGISTER,
    REQUEST
};
struct NetRequest {
    uint64_t capability = 0;
    std::string ident = "";
    int32_t registerType = UNKOWN;
    uint64_t bearTypes = 0;
    uint32_t uid = 0;
};

static const uint32_t DEFAULT_BANDWIDTH = 14;
struct LinkBandwidthInfo {
    uint32_t upBandwidth = DEFAULT_BANDWIDTH;
    uint32_t downBandwidth = DEFAULT_BANDWIDTH;
};

constexpr int32_t CellularDataStateAdapter(ApnProfileState state)
{
    switch (state) {
        case PROFILE_STATE_CONNECTING:
            return static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTING);
        case PROFILE_STATE_CONNECTED:
            [[fallthrough]]; // fall_through
        case PROFILE_STATE_DISCONNECTING:
            return static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED);
        case PROFILE_STATE_FAILED:
            [[fallthrough]]; // fall_through
        case PROFILE_STATE_RETRYING:
            [[fallthrough]]; // fall_through
        case PROFILE_STATE_IDLE:
            return static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED);
        default:
            return static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED);
    }
}

constexpr int32_t WrapCellularDataState(const int32_t cellularDataState)
{
    switch (cellularDataState) {
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_DISCONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTING): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTING);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_SUSPENDED);
        }
        default: {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_UNKNOWN);
        }
    }
}

enum DataContextRolesId {
    DATA_CONTEXT_ROLE_INVALID_ID = -1,
    DATA_CONTEXT_ROLE_ALL_ID = 0,
    DATA_CONTEXT_ROLE_DEFAULT_ID = 1,
    DATA_CONTEXT_ROLE_MMS_ID = 2,
    DATA_CONTEXT_ROLE_SUPL_ID = 3,
    DATA_CONTEXT_ROLE_DUN_ID = 4,
    DATA_CONTEXT_ROLE_IMS_ID = 5,
    DATA_CONTEXT_ROLE_IA_ID = 6,
    DATA_CONTEXT_ROLE_EMERGENCY_ID = 7,
    DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID = 8,
    DATA_CONTEXT_ROLE_XCAP_ID = 9,
    DATA_CONTEXT_ROLE_BIP_ID = 10,
    DATA_CONTEXT_ROLE_SNSSAI1_ID = 11,
    DATA_CONTEXT_ROLE_SNSSAI2_ID = 12,
    DATA_CONTEXT_ROLE_SNSSAI3_ID = 13,
    DATA_CONTEXT_ROLE_SNSSAI4_ID = 14,
    DATA_CONTEXT_ROLE_SNSSAI5_ID = 15,
    DATA_CONTEXT_ROLE_SNSSAI6_ID = 16
};

enum class DataContextPriority : int32_t { PRIORITY_NONE, PRIORITY_LOW, PRIORITY_NORMAL, PRIORITY_HIGH };

enum TelCallStatus {
    CALL_STATUS_UNKNOWN = -1,
    CALL_STATUS_ACTIVE = 0,
    CALL_STATUS_HOLDING = 1,
    CALL_STATUS_DIALING = 2,
    CALL_STATUS_ALERTING = 3,
    CALL_STATUS_INCOMING = 4,
    CALL_STATUS_WAITING = 5,
    CALL_STATUS_DISCONNECTED = 6,
    CALL_STATUS_DISCONNECTING = 7,
    CALL_STATUS_IDLE = 8,
};

enum class DisConnectionReason : int32_t {
    REASON_NORMAL,
    REASON_GSM_AND_CALLING_ONLY,
    REASON_RETRY_CONNECTION,
    REASON_CLEAR_CONNECTION,
    REASON_CHANGE_CONNECTION,
    REASON_PERMANENT_REJECT
};

enum class ApnTypes : int32_t {
    NONETYPE = 0,
    DEFAULT = 1,
    MMS = 2,
    SUPL = 4,
    DUN = 8,
    HIPRI = 16,
    FOTA = 32,
    IMS = 64,
    CBS = 128,
    IA = 256,
    EMERGENCY = 512,
    MCX = 1024,
    XCAP = 2048,
    INTERNAL_DEFAULT = 4096,
    BIP = 8192,
    SNSSAI1 = 16384,
    SNSSAI2 = 32768,
    SNSSAI3 = 65536,
    SNSSAI4 = 131072,
    SNSSAI5 = 262144,
    SNSSAI6 = 524288,
    ALL = 1048575
};

enum class RetryScene : int32_t {
    RETRY_SCENE_SETUP_DATA = 0,
    RETRY_SCENE_MODEM_DEACTIVATE = 1,
    RETRY_SCENE_OTHERS = 2,
};

static constexpr const char *DATA_CONTEXT_ROLE_ALL = "*";
static constexpr const char *DATA_CONTEXT_ROLE_DEFAULT = "default";
static constexpr const char *DATA_CONTEXT_ROLE_MMS = "mms";
static constexpr const char *DATA_CONTEXT_ROLE_SUPL = "supl";
static constexpr const char *DATA_CONTEXT_ROLE_DUN = "dun";
static constexpr const char *DATA_CONTEXT_ROLE_IMS = "ims";
static constexpr const char *DATA_CONTEXT_ROLE_BIP = "bip";
static constexpr const char *DATA_CONTEXT_ROLE_IA = "ia";
static constexpr const char *DATA_CONTEXT_ROLE_EMERGENCY = "emergency";
static constexpr const char *DATA_CONTEXT_ROLE_INTERNAL_DEFAULT = "internal_default";
static constexpr const char *DATA_CONTEXT_ROLE_XCAP = "xcap";
static constexpr const char *DATA_CONTEXT_ROLE_SNSSAI1 = "snssai1";
static constexpr const char *DATA_CONTEXT_ROLE_SNSSAI2 = "snssai2";
static constexpr const char *DATA_CONTEXT_ROLE_SNSSAI3 = "snssai3";
static constexpr const char *DATA_CONTEXT_ROLE_SNSSAI4 = "snssai4";
static constexpr const char *DATA_CONTEXT_ROLE_SNSSAI5 = "snssai5";
static constexpr const char *DATA_CONTEXT_ROLE_SNSSAI6 = "snssai6";
static const int32_t DATA_PROFILE_DEFAULT = 0;
static const int32_t DATA_PROFILE_MMS = 1;
static const int32_t DATA_PROFILE_INTERNAL_DEFAULT = 2;
static const int32_t DATA_PROFILE_SUPL = 3;
static const int32_t DATA_PROFILE_DUN = 4;
static const int32_t DATA_PROFILE_IA = 5;
static const int32_t DATA_PROFILE_XCAP = 6;
static const int32_t DATA_PROFILE_BIP = 7;
static const int32_t DATA_PROFILE_SNSSAI1 = 8;
static const int32_t DATA_PROFILE_SNSSAI2 = 9;
static const int32_t DATA_PROFILE_SNSSAI3 = 10;
static const int32_t DATA_PROFILE_SNSSAI4 = 11;
static const int32_t DATA_PROFILE_SNSSAI5 = 12;
static const int32_t DATA_PROFILE_SNSSAI6 = 13;
static const int32_t CMCC_MCC_MNC = 46002;
static const int32_t DEFAULT_AUTH_TYPE = 0;
static const int32_t DEFAULT_MTU = 1500;
static const uint8_t DEFAULT_STRENGTH = 20;
static const uint32_t DEFAULT_FREQUENCY = 50;
static const int64_t CORE_INIT_DELAY_TIME = 1000;
static const int32_t MASK_BYTE_BIT = 8;
static const int32_t IPV4_BIT = 32;
static const int32_t IPV6_BIT = 128;
static const int32_t MIN_IPV6_ITEM = 16;
static const int32_t MAX_IPV4_ITEM = 8;
static const int32_t MIN_IPV4_ITEM = 4;
static constexpr const char *DEFAULT_OPERATOR_NUMERIC = "46001";
static constexpr const char *DATA_METERED_CONTEXT_ROLES = "default";
static constexpr const char *IS_CELLULAR_DATA_ENABLE = "isCellularDataEnable";
static constexpr const char *IS_ROAMING = "isRoaming";
static constexpr const char *SETTING_SWITCH = "settingSwitch";
static constexpr const char *IDENT_PREFIX = "simId";
static constexpr const char *DEFAULT_HOSTNAME = "";
static constexpr const char *DEFAULT_MASK = "";
static constexpr const char *CELLULAR_DATA_RDB_URI = "datashare:///com.ohos.pdpprofileability";
static constexpr const char *CELLULAR_DATA_RDB_SELECTION =
    "datashare:///com.ohos.pdpprofileability/net/pdp_profile";
static constexpr const char *CELLULAR_DATA_RDB_RESET =
    "datashare:///com.ohos.pdpprofileability/net/pdp_profile/reset";
static constexpr const char *CELLULAR_DATA_RDB_PREFER =
    "datashare:///com.ohos.pdpprofileability/net/pdp_profile/preferapn";
static constexpr const char *CELLULAR_DATA_RDB_INIT =
    "datashare:///com.ohos.pdpprofileability/net/pdp_profile/init";
static constexpr const char *CELLULAR_DATA_SETTING_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static constexpr const char *CELLULAR_DATA_SETTING_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
static constexpr const char *CELLULAR_DATA_SETTING_DATA_ENABLE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=cellular_data_enable";
static constexpr const char *CELLULAR_DATA_SETTING_ANY_SIM_DETECTED_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=any_sim_detected";
static constexpr const char *CELLULAR_DATA_SETTING_DATA_ROAMING_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=cellular_data_roaming_enable";
static constexpr const char *CELLULAR_DATA_SETTING_DATA_INCALL_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=cellular_data_incall_enable";
static constexpr const char *CELLULAR_DATA_SETTING_INTELLIGENCE_SWITCH_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?"
    "Proxy=true&key=intelligence_card_switch_enable";
static constexpr const char *CELLULAR_DATA_SETTING_INTELLIGENCE_NETWORK_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?"
    "Proxy=true&key=intelligence_network_switching";
static constexpr const char *CELLULAR_DATA_AIRPLANE_MODE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=airplane_mode";
static const int32_t DEFAULT_NET_STATISTICS_PERIOD = 3 * 1000;
static const int32_t DATA_STALL_ALARM_NON_AGGRESSIVE_DELAY_IN_MS_DEFAULT = 1000 * 60 * 10;
static const int32_t DATA_STALL_ALARM_AGGRESSIVE_DELAY_IN_MS_DEFAULT = 1000 * 10;
static const int32_t ESTABLISH_DATA_CONNECTION_DELAY = 1 * 1000;
static const int32_t CONNECTION_TIMEOUT = 180 * 1000;
static const int32_t DISCONNECTION_TIMEOUT = 90 * 1000;
static const int32_t CONNECTION_TASK_TIME = 170 * 1000;
static const int32_t RESUME_DATA_PERMITTED_TIMEOUT = 30 * 1000;
static const int32_t RECOVERY_TRIGGER_PACKET = 10;
static const int32_t ERROR_APN_ID = -1;
static const int32_t VALID_IP_SIZE = 2;
static const int32_t TYPE_REQUEST_NET = 1;
static const int32_t TYPE_RELEASE_NET = 0;
static const int32_t DEFAULT_READ_APN_TIME = 2;
static const int32_t DEFAULT_MCC_SIZE = 3;
static const int32_t NULL_POINTER_EXCEPTION = -1;
static const int32_t PATH_PARAMETER_SIZE = 128;
static constexpr const char *ROUTED_IPV4 = "0.0.0.0";
static constexpr const char *ROUTED_IPV6 = "::";
static constexpr const char *CONFIG_DOWNLINK_THRESHOLDS = "persist.sys.data.downlink";
static constexpr const char *CONFIG_UPLINK_THRESHOLDS = "persist.sys.data.uplink";
static constexpr const char *CONFIG_TCP_BUFFER = "persist.sys.data.tcpbuffer";
static constexpr const char *CONFIG_PREFERAPN = "persist.sys.data.preferapn";
static constexpr const char *CONFIG_MOBILE_MTU = "persist.sys.data.mobilemtu";
static constexpr const char *CONFIG_DATA_SERVICE_EXT_PATH = "persist.sys.data.dataextpath";
static constexpr const char *CONFIG_MULTIPLE_CONNECTIONS = "persist.sys.data.multiple.connections";
static constexpr const char *CAPACITY_THRESHOLDS_FOR_DOWNLINK = "100,500,1000,5000,10000,20000,50000,75000,"
                                                                "100000,200000,500000,1000000,1500000,2000000";
static constexpr const char *CAPACITY_THRESHOLDS_FOR_UPLINK = "100,500,1000,5000,10000,20000,50000,75000,"
                                                              "100000,200000,500000";
static constexpr const char *DEFAULT_TCP_BUFFER_CONFIG = "UMTS:58254,349525,1048576,58254,349525,1048576;"
                                                         "HSPA:40778,244668,734003,16777,100663,301990;"
                                                         "HSUPA:131072,262144,2441216,4096,16384,399360;"
                                                         "HSDPA:61167,367002,1101005,8738,52429,262114;"
                                                         "HSPAP:122334,734003,2202010,32040,192239,576717;"
                                                         "EDGE:4093,26280,70800,4096,16384,70800;"
                                                         "eHRPD:131072,262144,1048576,4096,16384,524288;"
                                                         "1xRTT:16384,32768,131072,4096,16384,102400;"
                                                         "GPRS:4092,8760,48000,4096,8760,48000;"
                                                         "EVDO:4094,87380,262144,4096,16384,262144;"
                                                         "LTE:524288,4194304,8388608,262144,524288,1048576;"
                                                         "NR:2097152,6291456,16777216,512000,2097152,8388608;"
                                                         "LTE_CA:4096,6291456,12582912,4096,1048576,2097152";
constexpr const char *DEFAULT_BANDWIDTH_CONFIG =
    "GPRS:24,24;EDGE:70,18;UMTS:115,115;CDMA-IS95A:14,14;"
    "CDMA-IS95B:14,14;1xRTT:30,30;EvDo-rev.0:750,48;EvDo-rev.A:950,550;HSDPA:4300,620;"
    "HSUPA:4300,1800;HSPA:4300,1800;EvDo-rev.B:1500,550;eHRPD:750,48;HSPAP:13000,3400;"
    "TD-SCDMA:115,115;LTE:30000,15000;NR_NSA:47000,18000;NR_NSA_MMWAVE:145000,60000;"
    "NR_SA:145000,60000";
static constexpr const char *DEFAULT_PREFER_APN = "1";
static constexpr const char *DEFAULT_MOBILE_MTU = "1500";
static constexpr const char *DEFAULT_MULTIPLE_CONNECTIONS = "1";
static const int MAX_BUFFER_SIZE = 1024;
static const int MIN_BUFFER_SIZE = 5;
static const int UP_DOWN_LINK_SIZE = 100;
static const int32_t VALID_VECTOR_SIZE = 2;
static const int32_t DELAY_SET_RIL_BANDWIDTH_MS = 3000;
static const int32_t DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS = 50;
static const int32_t MAX_REPLY_COUNT = 200;
static constexpr const char *CELLULAR_DATA_COLUMN_ENABLE = "cellular_data_enable";
static constexpr const char *SIM_DETECTED_COLUMN_ENABLE = "any_sim_detected";
static constexpr const char *CELLULAR_DATA_COLUMN_ROAMING = "cellular_data_roaming_enable";
static constexpr const char *CELLULAR_DATA_COLUMN_INCALL = "cellular_data_incall_enable";
static constexpr const char *INTELLIGENCE_SWITCH_COLUMN_ENABLE = "intelligence_card_switch_enable";
static constexpr const char *INTELLIGENCE_NETWORK_COLUMN_ENABLE = "intelligence_network_switching";
static constexpr const char *CELLULAR_DATA_COLUMN_AIRPLANE = "settings.telephony.airplanemode";
static constexpr const char *CELLULAR_DATA_COLUMN_KEYWORD = "KEYWORD";
static constexpr const char *CELLULAR_DATA_COLUMN_VALUE = "VALUE";
static const int32_t INVALID_SIM_ID = 0;
static const int32_t INVALID_SLOT_ID = -1;
static const int32_t CELLULAR_DATA_VSIM_SLOT_ID = 2;
static const int32_t SUPPLIER_INVALID_REG_STATE = -1;
static const int32_t MAX_SLOT_NUM = 2;
static constexpr const char *PROTOCOL_IPV4 = "IP";
static constexpr const char *PROTOCOL_IPV6 = "IPV6";
static constexpr const char *PROTOCOL_IPV4V6 = "IPV4V6";
static constexpr const char *NOT_FILLED_IN = "not-filled-in";
static const int32_t APN_CREATE_RETRY_TIMES = 3;
static const int32_t RETRY_DELAY_TIME = 5 * 1000;
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_CONSTANT_H
