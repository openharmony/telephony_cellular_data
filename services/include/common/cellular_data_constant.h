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
    std::string ip;
    std::string netMask;
    uint8_t type;
    uint8_t prefixLen;
};

struct RouteInfo {
    std::string ip;
    uint8_t type;
    std::string destination;
};

struct NetSupplier {
    uint32_t supplierId;
    uint64_t capability;
    int32_t slotId;
};

struct NetRequest {
    uint64_t capability;
    std::string ident;
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

enum DataContextRolesId {
    DATA_CONTEXT_ROLE_INVALID_ID = -1,
    DATA_CONTEXT_ROLE_ALL_ID = 0,
    DATA_CONTEXT_ROLE_DEFAULT_ID = 1,
    DATA_CONTEXT_ROLE_MMS_ID = 2,
    DATA_CONTEXT_ROLE_SUPL_ID = 3,
    DATA_CONTEXT_ROLE_DUN_ID = 4,
    DATA_CONTEXT_ROLE_IMS_ID = 5,
    DATA_CONTEXT_ROLE_IA_ID = 6,
    DATA_CONTEXT_ROLE_EMERGENCY_ID = 7
};

enum class DataContextPriority : int32_t { PRIORITY_LOW, PRIORITY_NORMAL, PRIORITY_HIGH };

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
    REASON_CHANGE_CONNECTION
};

static constexpr const char *DATA_CONTEXT_ROLE_ALL = "*";
static constexpr const char *DATA_CONTEXT_ROLE_DEFAULT = "default";
static constexpr const char *DATA_CONTEXT_ROLE_MMS = "mms";
static constexpr const char *DATA_CONTEXT_ROLE_SUPL = "supl";
static constexpr const char *DATA_CONTEXT_ROLE_DUN = "dun";
static constexpr const char *DATA_CONTEXT_ROLE_IMS = "ims";
static constexpr const char *DATA_CONTEXT_ROLE_IA = "ia";
static constexpr const char *DATA_CONTEXT_ROLE_EMERGENCY = "emergency";
static const int32_t INVALID_PROFILE_ID = -1;
static const int32_t DATA_PROFILE_DEFAULT = 0;
static const int32_t DATA_PROFILE_MMS = 1;
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
static constexpr const char *CELLULAR_DATA_RDB_URI = "dataability:///com.ohos.pdpprofileability/net/pdp_profile";
static constexpr const char *CELLULAR_DATA_SETTING_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static constexpr const char *CELLULAR_DATA_SETTING_DATA_ENABLE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=cellular_data_enable";
static constexpr const char *CELLULAR_DATA_SETTING_DATA_ROAMING_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=cellular_data_roaming_enable";
static const int32_t DEFAULT_NET_STATISTICS_PERIOD = 3 * 1000;
static const int32_t DEFAULT_STALL_DETECTION_PERIOD = 10 * 1000;
static const int32_t ESTABLISH_DATA_CONNECTION_DELAY = 1 * 1000;
static const int32_t CONNECTION_DISCONNECTION_TIMEOUT = 60 * 1000;
static const int32_t RECOVERY_TRIGGER_PACKET = 10;
static const int32_t ERROR_APN_ID = -1;
static const int32_t VALID_IP_SIZE = 2;
static const int32_t TYPE_REQUEST_NET = 1;
static const int32_t TYPE_RELEASE_NET = 0;
static const int32_t DEFAULT_READ_APN_TIME = 2;
static const int32_t DEFAULT_MCC_SIZE = 3;
static const int32_t NULL_POINTER_EXCEPTION = -1;
static constexpr const char *ROUTED_IPV4 = "0.0.0.0";
static constexpr const char *ROUTED_IPV6 = "::";
static constexpr const char *CONFIG_DOWNLINK_THRESHOLDS = "persist.sys.data.downlink";
static constexpr const char *CONFIG_UPLINK_THRESHOLDS = "persist.sys.data.uplink";
static constexpr const char *CONFIG_TCP_BUFFER = "persist.sys.data.tcpbuffer";
static constexpr const char *CONFIG_BANDWIDTH = "persist.sys.data.bandwidth";
static constexpr const char *CONFIG_PREFERAPN = "persist.sys.data.preferapn";
static constexpr const char *CONFIG_MOBILE_MTU = "persist.sys.data.mobilemtu";
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
                                                         "LTE:524288,1048576,2097152,262144,524288,1048576;"
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
static constexpr const char *DEFAULT_MULTIPLE_CONNECTIONS = "0";
static const int MAX_BUFFER_SIZE = 1024;
static const int MIN_BUFFER_SIZE = 5;
static const int UP_DOWN_LINK_SIZE = 100;
static const int32_t VALID_VECTOR_SIZE = 2;
static const int32_t DELAY_SET_RIL_BANDWIDTH_MS = 3000;
static const int32_t DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS = 50;
static constexpr const char *CELLULAR_DATA_COLUMN_ENABLE = "cellular_data_enable";
static constexpr const char *CELLULAR_DATA_COLUMN_ROAMING = "cellular_data_roaming_enable";
static constexpr const char *CELLULAR_DATA_COLUMN_KEYWORD = "KEYWORD";
static constexpr const char *CELLULAR_DATA_COLUMN_VALUE = "VALUE";
const int32_t INVALID_SIM_ID = 0;
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_CONSTANT_H
