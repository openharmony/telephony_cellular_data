/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cstdint>

#include "tel_cellular_data_log.h"
#include "tel_cellular_data_impl.h"

#include "cellular_data_client.h"
#include "napi_util.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {

    static bool IsCellularDataManagerInited()
    {
        return CellularDataClient::GetInstance().IsConnect();
    }

    static inline bool IsValidSlotId(int32_t slotId)
    {
        return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT));
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

    static int32_t WrapGetCellularDataFlowTypeType(const int32_t cellularDataType)
    {
        switch (cellularDataType) {
            case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE): {
                return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE);
            }
            case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN): {
                return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN);
            }
            case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP): {
                return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP);
            }
            case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN): {
                return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN);
            }
            case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT): {
                return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
            }
            default: {
                return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE);
            }
        }
    }

    static int32_t ConvertCJErrCode(int32_t errCode)
    {
        switch (errCode) {
            case TELEPHONY_ERR_ARGUMENT_MISMATCH:
            case TELEPHONY_ERR_ARGUMENT_INVALID:
            case TELEPHONY_ERR_ARGUMENT_NULL:
            case TELEPHONY_ERR_SLOTID_INVALID:
            case ERROR_SLOT_ID_INVALID:
                // 83000001
                return CJ_ERROR_TELEPHONY_ARGUMENT_ERROR;
            case TELEPHONY_ERR_DESCRIPTOR_MISMATCH:
            case TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL:
            case TELEPHONY_ERR_WRITE_DATA_FAIL:
            case TELEPHONY_ERR_READ_DATA_FAIL:
            case TELEPHONY_ERR_WRITE_REPLY_FAIL:
            case TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL:
            case TELEPHONY_ERR_REGISTER_CALLBACK_FAIL:
            case TELEPHONY_ERR_CALLBACK_ALREADY_REGISTERED:
            case TELEPHONY_ERR_UNINIT:
            case TELEPHONY_ERR_UNREGISTER_CALLBACK_FAIL:
                // 83000002
                return CJ_ERROR_TELEPHONY_SERVICE_ERROR;
            case TELEPHONY_ERR_VCARD_FILE_INVALID:
            case TELEPHONY_ERR_FAIL:
            case TELEPHONY_ERR_MEMCPY_FAIL:
            case TELEPHONY_ERR_MEMSET_FAIL:
            case TELEPHONY_ERR_STRCPY_FAIL:
            case TELEPHONY_ERR_LOCAL_PTR_NULL:
            case TELEPHONY_ERR_SUBSCRIBE_BROADCAST_FAIL:
            case TELEPHONY_ERR_PUBLISH_BROADCAST_FAIL:
            case TELEPHONY_ERR_STRTOINT_FAIL:
            case TELEPHONY_ERR_ADD_DEATH_RECIPIENT_FAIL:
            case TELEPHONY_ERR_RIL_CMD_FAIL:
            case TELEPHONY_ERR_DATABASE_WRITE_FAIL:
            case TELEPHONY_ERR_DATABASE_READ_FAIL:
            case TELEPHONY_ERR_UNKNOWN_NETWORK_TYPE:
                // 83000003
                return CJ_ERROR_TELEPHONY_SYSTEM_ERROR;
            case TELEPHONY_ERR_NO_SIM_CARD:
                // 83000004
                return CJ_ERROR_TELEPHONY_NO_SIM_CARD;
            case TELEPHONY_ERR_AIRPLANE_MODE_ON:
                // 83000005
                return CJ_ERROR_TELEPHONY_AIRPLANE_MODE_ON;
            case TELEPHONY_ERR_NETWORK_NOT_IN_SERVICE:
                // 83000006
                return CJ_ERROR_TELEPHONY_NETWORK_NOT_IN_SERVICE;
            case TELEPHONY_ERR_PERMISSION_ERR:
                // 201
                return CJ_ERROR_TELEPHONY_PERMISSION_DENIED;
            case TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API:
                // 202
                return CJ_ERROR_TELEPHONY_PERMISSION_DENIED;
            default:
                return errCode;
        }
    }

    int32_t CellularDataImpl::GetDefaultCellularDataSlotId()
    {
        return CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    }

    int32_t CellularDataImpl::GetCellularDataFlowType(int32_t &errCode)
    {
        int32_t dataState = 0;
        if (IsCellularDataManagerInited()) {
            dataState = CellularDataClient::GetInstance().GetCellularDataFlowType();
            dataState = WrapGetCellularDataFlowTypeType(dataState);
            errCode = ERROR_NONE;
        } else {
            errCode = ERROR_SERVICE_UNAVAILABLE;
        }
        return dataState;
    }

    int32_t CellularDataImpl::GetCellularDataState(int32_t &errCode)
    {
        int32_t dataState = 0;
        if (IsCellularDataManagerInited()) {
            dataState = CellularDataClient::GetInstance().GetCellularDataState();
            dataState = WrapCellularDataType(dataState);
            errCode = ERROR_NONE;
        } else {
            errCode = ERROR_SERVICE_UNAVAILABLE;
        }
        return dataState;
    }

    bool CellularDataImpl::IsCellularDataEnabled(int32_t &errCode)
    {
        bool enabled = false;
        if (IsCellularDataManagerInited()) {
            errCode = CellularDataClient::GetInstance().IsCellularDataEnabled(enabled);
        } else {
            errCode = ERROR_SERVICE_UNAVAILABLE;
        }
        errCode = ConvertCJErrCode(errCode);
        return enabled;
    }

    bool CellularDataImpl::IsCellularDataRoamingEnabled(int32_t slotId, int32_t &errCode)
    {
        bool enabled = false;
        if (!IsValidSlotId(slotId)) {
            LOGE("CellularDataImpl::IsCellularDataRoamingEnabled slotId is invalid");
            errCode = ConvertCJErrCode(ERROR_SLOT_ID_INVALID);
            return enabled;
        }

        if (IsCellularDataManagerInited()) {
            errCode = CellularDataClient::GetInstance().IsCellularDataRoamingEnabled(slotId, enabled);
        } else {
            errCode = ERROR_SERVICE_UNAVAILABLE;
        }
        errCode = ConvertCJErrCode(errCode);
        return enabled;
    }

    int32_t CellularDataImpl::GetDefaultCellularDataSimId()
    {
        int32_t simId = 0;
        CellularDataClient::GetInstance().GetDefaultCellularDataSimId(simId);
        return simId;
    }
}
}