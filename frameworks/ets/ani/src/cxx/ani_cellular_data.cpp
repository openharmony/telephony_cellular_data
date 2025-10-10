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
#include "napi_util.h"
#include "cxx.h"
#include "wrapper.rs.h"

namespace OHOS {
using namespace Telephony;
namespace CellularDataAni {
static constexpr const char *SET_TELEPHONY_STATE = "ohos.permission.SET_TELEPHONY_STATE";
static constexpr const char *GET_NETWORK_INFO = "ohos.permission.GET_NETWORK_INFO";

static bool IsCellularDataManagerInited()
{
    return CellularDataClient::GetInstance().IsConnect();
}

ArktsError IsCellularDataEnabled(bool &dataEnabled)
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().IsCellularDataEnabled(dataEnabled);
    } else {
        errorCode =  ERROR_SERVICE_UNAVAILABLE;
    }

    JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(errorCode,
                                                                    "IsCellularDataEnabled",
                                                                    GET_NETWORK_INFO);
    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };

    return ArktsErr;
}

ArktsError EnableCellularDataSync()
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularData(true);
    } else {
        errorCode =  ERROR_SERVICE_UNAVAILABLE;
    }

    JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(errorCode,
                                                                    "enableCellularData",
                                                                    SET_TELEPHONY_STATE);
    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };

    return ArktsErr;
}

ArktsError DisableCellularDataSync()
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularData(false);
    } else {
        errorCode =  ERROR_SERVICE_UNAVAILABLE;
    }

    JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(errorCode,
                                                                    "disableCellularData",
                                                                    SET_TELEPHONY_STATE);
    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };

    return ArktsErr;
}

int32_t GetDefaultCellularDataSlotIdSync()
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

ArktsError GetCellularDataState(int32_t &CellularDataState)
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

} // namespace CellularDataAni
} // namespace OHOS