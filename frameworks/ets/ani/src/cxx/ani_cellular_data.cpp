/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ani_cellular_data.h"
#include "cellular_data_client.h"
#include "cxx.h"
#include "napi_util.h"
#include "telephony_types.h"
#include "wrapper.rs.h"

namespace OHOS {
using namespace Telephony;
namespace CellularDataAni {
static constexpr const char *SET_TELEPHONY_STATE = "ohos.permission.SET_TELEPHONY_STATE";
static constexpr const char *GET_NETWORK_INFO = "ohos.permission.GET_NETWORK_INFO";
static constexpr const char *MANAGE_APN_SETTING = "ohos.permission.MANAGE_APN_SETTING";

static bool IsCellularDataManagerInited()
{
    return CellularDataClient::GetInstance().IsConnect();
}

static inline bool IsValidSlotId(int32_t slotId)
{
    return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT));
}

static inline ArktsError ConvertArktsErrorWithPermission(int32_t errorCode, const std::string &funcName,
                                                         const std::string &permission)
{
    JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(errorCode, funcName, permission);

    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };
    return ArktsErr;
}

ArktsError isCellularDataEnabled(bool &dataEnabled)
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().IsCellularDataEnabled(dataEnabled);
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    return ConvertArktsErrorWithPermission(errorCode, "IsCellularDataEnabled", GET_NETWORK_INFO);
}

ArktsError enableCellularDataSync()
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularData(true);
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    return ConvertArktsErrorWithPermission(errorCode, "enableCellularData", SET_TELEPHONY_STATE);
}

ArktsError disableCellularDataSync()
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularData(false);
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    return ConvertArktsErrorWithPermission(errorCode, "disableCellularData", SET_TELEPHONY_STATE);
}

int32_t getDefaultCellularDataSlotIdSync()
{
    int32_t slotId = -1;
    slotId = CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    return slotId;
}

static int32_t WrapCellularDataType(const int32_t cellularDataType)
{
    switch (cellularDataType) {
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

ArktsError getCellularDataState(int32_t &CellularDataState)
{
    int32_t errorCode;
    if (IsCellularDataManagerInited()) {
        int32_t dataState = CellularDataClient::GetInstance().GetCellularDataState();
        CellularDataState = WrapCellularDataType(dataState);
        errorCode = TELEPHONY_ERR_SUCCESS;
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    JsError error = NapiUtil::ConverErrorMessageForJs(errorCode);
    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };
    return ArktsErr;
}

ArktsError disableCellularDataRoamingSync(int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "disableCellularDataRoaming",
                                               SET_TELEPHONY_STATE);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, false);
    }

    return ConvertArktsErrorWithPermission(errorCode, "disableCellularDataRoaming", SET_TELEPHONY_STATE);
}

ArktsError enableCellularDataRoamingSync(int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "enableCellularDataRoaming", SET_TELEPHONY_STATE);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, true);
    }

    return ConvertArktsErrorWithPermission(errorCode, "enableCellularDataRoaming", SET_TELEPHONY_STATE);
}

ArktsError isCellularDataRoamingEnabledSync(int32_t slotId, bool &dataEnabled)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "isCellularDataRoamingEnabled", GET_NETWORK_INFO);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().IsCellularDataRoamingEnabled(slotId, dataEnabled);
    }

    return ConvertArktsErrorWithPermission(errorCode, "isCellularDataRoamingEnabled", GET_NETWORK_INFO);
}

ArktsError setDefaultCellularDataSlotIdSyn(int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "setDefaultCellularDataSlotId",
                                               SET_TELEPHONY_STATE);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(slotId);
    }

    return ConvertArktsErrorWithPermission(errorCode, "setDefaultCellularDataSlotId", SET_TELEPHONY_STATE);
}

int32_t getCellularDataFlowTypeSyn()
{
    return CellularDataClient::GetInstance().GetCellularDataFlowType();
}

ArktsError setPreferredApnSyn(int32_t apnId, bool &ret)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().SetPreferApn(apnId);
    }
    ret = errorCode == TELEPHONY_SUCCESS;
    return ConvertArktsErrorWithPermission(errorCode, "setPreferredApn", MANAGE_APN_SETTING);
}

int32_t getDefaultCellularDataSimIdSyn()
{
    int32_t simId = 0;
    return CellularDataClient::GetInstance().GetDefaultCellularDataSimId(simId);
}

std::u16string Utf8ToU16String(const std::string &str)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str);
}

std::string U16StringToUtf8(const std::u16string &str)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(str);
}

ArktsError queryApnIdsSync(const ApnInfo &info, rust::vec<uint32_t> &ret)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    if (IsCellularDataManagerInited()) {
        std::vector<uint32_t> apnIdList;
        OHOS::Telephony::ApnInfo apnInfo;
        apnInfo.apnName = Utf8ToU16String(std::string(info.apnName));
        apnInfo.apn = Utf8ToU16String(std::string(info.apn));
        apnInfo.mcc = Utf8ToU16String(std::string(info.mcc));
        apnInfo.mnc = Utf8ToU16String(std::string(info.mnc));
        apnInfo.user = Utf8ToU16String(std::string(info.user));
        apnInfo.type = Utf8ToU16String(std::string(info.type_));
        apnInfo.proxy = Utf8ToU16String(std::string(info.proxy));
        apnInfo.mmsproxy = Utf8ToU16String(std::string(info.mmsproxy));
        errorCode = CellularDataClient::GetInstance().QueryApnIds(apnInfo, apnIdList);
        if (errorCode == TELEPHONY_SUCCESS) {
            for (auto apnId : apnIdList) {
                ret.push_back(apnId);
            }
        }
    }
    return ConvertArktsErrorWithPermission(errorCode, "queryApnIds", MANAGE_APN_SETTING);
}

ArktsError queryAllApnsSync(rust::vec<ApnInfo> &ret)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    std::vector<OHOS::Telephony::ApnInfo> apnInfoList;
    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().QueryAllApnInfo(apnInfoList);
    }
    for (auto info : apnInfoList) {
        ret.push_back(ApnInfo{
            .apnName = rust::string(U16StringToUtf8(info.apnName)),
            .apn = rust::string(U16StringToUtf8(info.apn)),
            .mcc = rust::string(U16StringToUtf8(info.mcc)),
            .mnc = rust::string(U16StringToUtf8(info.mnc)),
            .user = rust::string(U16StringToUtf8(info.user)),
            .type_ = rust::string(U16StringToUtf8(info.type)),
            .proxy = rust::string(U16StringToUtf8(info.proxy)),
            .mmsproxy = rust::string(U16StringToUtf8(info.mmsproxy)),
        });
    }

    return ConvertArktsErrorWithPermission(errorCode, "queryAllApns", MANAGE_APN_SETTING);
}

ArktsError getActiveApnNameSync(rust::String &apnName)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    std::string apnNameStr;
    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().GetActiveApnName(apnNameStr);
    }
    apnName = rust::string(apnNameStr);
    return ConvertArktsErrorWithPermission(errorCode, "GetActiveApnName", GET_NETWORK_INFO);
}
} // namespace CellularDataAni
} // namespace OHOS