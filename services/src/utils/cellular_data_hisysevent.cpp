/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "cellular_data_hisysevent.h"

#include "chrono"
#include "string"
#include "type_traits"
#include "apn_manager.h"
#include "cellular_data_net_agent.h"
#include "pdp_profile_data.h"

namespace OHOS {
namespace Telephony {
// EVENT
static constexpr const char *DATA_CONNECTION_STATE_EVENT = "DATA_CONNECTION_STATE";
static constexpr const char *ROAMING_DATA_CONNECTION_STATE_EVENT = "ROAMING_DATA_CONNECTION_STATE";
static constexpr const char *DATA_ACTIVATE_FAILED_EVENT = "DATA_ACTIVATE_FAILED";
static constexpr const char *DATA_DEACTIVED_EVENT = "DATA_DEACTIVED";
static constexpr const char *CELLULAR_REQUEST_EVENT = "CELLULAR_REQUEST";
static constexpr const char *APN_INFO_EVENT = "APN_INFO_EVENT";

// KEY
static constexpr const char *MODULE_NAME_KEY = "MODULE";
static constexpr const char *SLOT_ID_KEY = "SLOT_ID";
static constexpr const char *SUPPLIER_ID_KEY = "SUPPLIER_ID";
static constexpr const char *STATE_KEY = "STATE";
static constexpr const char *DATA_SWITCH_KEY = "DATA_SWITCH";
static constexpr const char *UPLINK_DATA_KEY = "UPLINK_DATA";
static constexpr const char *DOWNLINK_DATA_KEY = "DOWNLINK_DATA";
static constexpr const char *DATASTATE_KEY = "DATASTATE";
static constexpr const char *ERROR_TYPE_KEY = "ERROR_TYPE";
static constexpr const char *ERROR_MSG_KEY = "ERROR_MSG";
static constexpr const char *TYPE_KEY = "TYPE";
static constexpr const char *APN_TYPE_KEY = "APN_TYPE";
static constexpr const char *CALL_UID_KEY = "CALL_UID";
static constexpr const char *CALL_PID_KEY = "CALL_PID";
static constexpr const char *NAME_KEY = "NAME";
static constexpr const char *REQUEST_ID_KEY = "REQUEST_ID";

static constexpr const char *CARDID_KEY = "CARDID";
static constexpr const char *CARRIER_KEY = "CARRIER";
static constexpr const char *APN_KEY = "APN";
static constexpr const char *PROXY_KEY = "PROXY";
static constexpr const char *MMSPROXY_KEY = "MMSPROXY";
static constexpr const char *NUMERIC_KEY = "NUMERIC";
static constexpr const char *AUTHTYPE_KEY = "AUTHTYPE";
static constexpr const char *APNTYPES_KEY = "APNTYPES";
static constexpr const char *PROTOCOL_KEY = "PROTOCOL";
static constexpr const char *ROAMINGPROTOCOL_KEY = "ROAMINGPROTOCOL";
static constexpr const char *BEARER_KEY = "BEARER";
static constexpr const char *MVNOTYPE_KEY = "MVNOTYPE";
static constexpr const char *MVNOMATCHDATA_KEY = "MVNOMATCHDATA";
static constexpr const char *ISCREATEDAPN_KEY = "ISCREATEDAPN";
static constexpr const char *HASUSERPSD_KEY = "HASUSERPSD";
static constexpr const char *SERVER_KEY = "SERVER";

// VALUE
static constexpr const char *CELLULAR_DATA_MODULE = "CELLULAR_DATA";
static constexpr int32_t NUMBER_MINUS_ONE = -1;

void CellularDataHiSysEvent::WriteDataDeactiveBehaviorEvent(const int32_t slotId, const DataDisconnectCause type,
    const std::string &apnType)
{
    int32_t bitMap = ApnManager::FindApnTypeByApnName(apnType);
    HiWriteBehaviorEvent(DATA_DEACTIVED_EVENT, SLOT_ID_KEY, slotId, APN_TYPE_KEY, bitMap,
        TYPE_KEY, static_cast<int32_t>(type));
}

void CellularDataHiSysEvent::WriteDataConnectStateBehaviorEvent(const int32_t slotId, const std::string &apnType,
    const uint64_t capability, const int32_t state)
{
    int32_t bitMap = ApnManager::FindApnTypeByApnName(apnType);
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    int32_t supplierId = netAgent.GetSupplierId(slotId, capability);
    HiWriteBehaviorEvent(DATA_CONNECTION_STATE_EVENT, SLOT_ID_KEY, slotId, APN_TYPE_KEY, bitMap,
        SUPPLIER_ID_KEY, supplierId, STATE_KEY, state);
}

void CellularDataHiSysEvent::WriteRoamingConnectStateBehaviorEvent(const int32_t state)
{
    HiWriteBehaviorEvent(ROAMING_DATA_CONNECTION_STATE_EVENT, STATE_KEY, state);
}

void CellularDataHiSysEvent::WriteCellularRequestBehaviorEvent(
    const uint32_t uid, const std::string name, const uint64_t type, const int32_t state)
{
    HiWriteBehaviorEvent(CELLULAR_REQUEST_EVENT, CALL_UID_KEY, static_cast<int32_t>(uid),
        CALL_PID_KEY, NUMBER_MINUS_ONE, NAME_KEY, name, REQUEST_ID_KEY, NUMBER_MINUS_ONE,
        TYPE_KEY, static_cast<int32_t>(type), STATE_KEY, state);
}

void CellularDataHiSysEvent::WriteDataActivateFaultEvent(
    const int32_t slotId, const int32_t switchState, const CellularDataErrorCode errorType, const std::string &errorMsg)
{
    HiWriteFaultEvent(DATA_ACTIVATE_FAILED_EVENT, MODULE_NAME_KEY, CELLULAR_DATA_MODULE, SLOT_ID_KEY, slotId,
        DATA_SWITCH_KEY, switchState, UPLINK_DATA_KEY, INVALID_PARAMETER, DOWNLINK_DATA_KEY, INVALID_PARAMETER,
        DATASTATE_KEY, INVALID_PARAMETER, ERROR_TYPE_KEY, static_cast<int32_t>(errorType), ERROR_MSG_KEY, errorMsg);
}

void CellularDataHiSysEvent::WriteApnInfoBehaviorEvent(const int32_t slotId, struct PdpProfile &apnData)
{
    std::string numeric = apnData.mcc + apnData.mnc;
    int32_t apnHasPsd = apnData.authPwd.empty() ? 1 : 0;
    HiWriteBehaviorEvent(APN_INFO_EVENT,
        CARDID_KEY, slotId,
        CARRIER_KEY, apnData.profileName,
        APN_KEY, apnData.apn,
        PROXY_KEY, apnData.proxyIpAddress,
        MMSPROXY_KEY, apnData.mmsIpAddress,
        NUMERIC_KEY, numeric,
        AUTHTYPE_KEY, apnData.authType,
        APNTYPES_KEY, apnData.apnTypes,
        PROTOCOL_KEY, apnData.pdpProtocol,
        ROAMINGPROTOCOL_KEY, apnData.roamPdpProtocol,
        BEARER_KEY, apnData.bearingSystemType,
        MVNOTYPE_KEY, apnData.mvnoType,
        MVNOMATCHDATA_KEY, apnData.mvnoMatchData,
        ISCREATEDAPN_KEY, apnData.edited,
        HASUSERPSD_KEY, apnHasPsd,
        SERVER_KEY, apnData.server);
}

void CellularDataHiSysEvent::SetCellularDataActivateStartTime()
{
    dataActivateStartTime_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
}

void CellularDataHiSysEvent::JudgingDataActivateTimeOut(const int32_t slotId, const int32_t switchState)
{
    int64_t dataActivateEndTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    if (dataActivateEndTime - dataActivateStartTime_ > DATA_ACTIVATE_TIME) {
        WriteDataActivateFaultEvent(slotId, switchState, CellularDataErrorCode::DATA_ERROR_DATA_ACTIVATE_TIME_OUT,
            "data activate time out " + std::to_string(dataActivateEndTime - dataActivateStartTime_));
    }
}
} // namespace Telephony
} // namespace OHOS
