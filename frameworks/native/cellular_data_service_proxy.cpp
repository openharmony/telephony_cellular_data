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

#include "cellular_data_service_proxy.h"

#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
int32_t CellularDataServiceProxy::IsCellularDataEnabled(bool &dataEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::IS_CELLULAR_DATA_ENABLED, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function IsCellularDataEnabled call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    if (result == TELEPHONY_SUCCESS) {
        dataEnabled = reply.ReadBool();
    }

    return result;
}

int32_t CellularDataServiceProxy::EnableCellularData(bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteBool(enable);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::ENABLE_CELLULAR_DATA, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function EnableCellularData call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetCellularDataState()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::GET_CELLULAR_DATA_STATE, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetCellularDataState call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::IsCellularDataRoamingEnabled(int32_t slotId, bool &dataRoamingEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::IS_DATA_ROAMING_ENABLED, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function IsCellularDataRoamingEnabled call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    if (result == TELEPHONY_SUCCESS) {
        dataRoamingEnabled = reply.ReadBool();
    }

    return result;
}

int32_t CellularDataServiceProxy::EnableCellularDataRoaming(int32_t slotId, bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    data.WriteBool(enable);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::ENABLE_DATA_ROAMING, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function EnableCellularDataRoaming call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::HandleApnChanged(int32_t slotId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::APN_DATA_CHANGED, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HandleApnChanged call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetDefaultCellularDataSlotId()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::GET_DEFAULT_SLOT_ID, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetDefaultCellularDataSlotId call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::SetDefaultCellularDataSlotId(int32_t slotId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::SET_DEFAULT_SLOT_ID, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function SetDefaultCellularDataSlotId call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetCellularDataFlowType()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::GET_FLOW_TYPE_ID, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetCellularDataFlowType call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::HasInternetCapability(int32_t slotId, int32_t cid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    data.WriteInt32(cid);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::HAS_CAPABILITY, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Strategy switch fail! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::ClearCellularDataConnections(int32_t slotId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)FuncCode::CLEAR_ALL_CONNECTIONS, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Strategy switch fail! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::RegisterSimAccountCallback(const sptr<SimAccountCallback> &callback)
{
    if (callback == nullptr) {
        TELEPHONY_LOGE("callback is nullptr!");
        return TELEPHONY_ERR_ARGUMENT_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        TELEPHONY_LOGE("write remote object failed!");
        return TELEPHONY_ERR_WRITE_DATA_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest(static_cast<uint32_t>(FuncCode::REG_SIM_ACCOUNT_CALLBACK), data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("error! errCode:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t CellularDataServiceProxy::UnregisterSimAccountCallback()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error =
        remote->SendRequest(static_cast<uint32_t>(FuncCode::UN_REG_SIM_ACCOUNT_CALLBACK), data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("error! errCode:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
} // namespace Telephony
} // namespace OHOS