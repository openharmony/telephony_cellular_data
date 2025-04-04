/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#include "securec.h"
#include "string_ex.h"

namespace OHOS {
namespace Telephony {
constexpr int32_t BUFFER_MAX = 65538;
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::IS_CELLULAR_DATA_ENABLED, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::ENABLE_CELLULAR_DATA, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetIntelligenceSwitchState(bool &state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::GET_INTELLIGENCE_SWITCH_STATE, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    state = reply.ReadBool();
    return result;
}

int32_t CellularDataServiceProxy::EnableIntelligenceSwitch(bool enable)
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::ENABLE_INTELLIGENCE_SWITCH, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::GET_CELLULAR_DATA_STATE, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetApnState(int32_t slotId, const std::string &apnType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    data.WriteString(apnType);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::GET_CELLULAR_DATA_APN_STATE, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetCellularDataState call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetDataRecoveryState()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::GET_RECOVERY_STATE, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetDefaultCellularDataSlotId call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::IS_DATA_ROAMING_ENABLED, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::ENABLE_DATA_ROAMING, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::APN_DATA_CHANGED, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::GET_DEFAULT_SLOT_ID, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetDefaultCellularDataSimId(int32_t &simId)
{
    MessageParcel data;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("failed: remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::GET_DEFAULT_SIM_ID, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendRequest failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    TELEPHONY_LOGD("end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        simId = reply.ReadInt32();
    }
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::SET_DEFAULT_SLOT_ID, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::GET_FLOW_TYPE_ID, data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::HAS_CAPABILITY, data, reply, option);
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
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::CLEAR_ALL_CONNECTIONS, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Strategy switch fail! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::ClearAllConnections(int32_t slotId, DisConnectionReason reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor());
    data.WriteInt32(slotId);
    data.WriteInt32(static_cast<int32_t>(reason));
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::CLEAR_ALL_CONNECTIONS_USE_REASON, data,
        reply, option);
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
    if (!data.WriteRemoteObject(callback->AsObject())) {
        TELEPHONY_LOGE("write remote object failed!");
        return TELEPHONY_ERR_WRITE_DATA_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest(static_cast<uint32_t>(CellularDataInterfaceCode::REG_SIM_ACCOUNT_CALLBACK),
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGD("error! errCode:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t CellularDataServiceProxy::UnregisterSimAccountCallback(const sptr<SimAccountCallback> &callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        TELEPHONY_LOGE("write remote object failed!");
        return TELEPHONY_ERR_WRITE_DATA_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest(static_cast<uint32_t>(CellularDataInterfaceCode::UN_REG_SIM_ACCOUNT_CALLBACK),
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("error! errCode:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t CellularDataServiceProxy::GetDataConnApnAttr(int32_t slotId, ApnItem::Attribute &apnAttr)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!dataParcel.WriteInt32(slotId)) {
        TELEPHONY_LOGE("write userId failed!");
        return TELEPHONY_ERR_WRITE_DATA_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::GET_DATA_CONN_APN_ATTR,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Strategy switch fail! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    if (result == TELEPHONY_ERR_SUCCESS) {
        auto apnAttrPtr = replyParcel.ReadRawData(sizeof(ApnItem::Attribute));
        if (apnAttrPtr == nullptr) {
            return TELEPHONY_ERR_READ_DATA_FAIL;
        }
        if (memcpy_s(&apnAttr, sizeof(ApnItem::Attribute), apnAttrPtr, sizeof(ApnItem::Attribute)) != EOK) {
            return TELEPHONY_ERR_MEMCPY_FAIL;
        }
    } else {
        TELEPHONY_LOGE("end failed: result=%{public}d", result);
    }

    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataServiceProxy::GetDataConnIpType(int32_t slotId, std::string &ipType)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!dataParcel.WriteInt32(slotId)) {
        TELEPHONY_LOGE("write userId failed!");
        return TELEPHONY_ERR_WRITE_DATA_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::GET_DATA_CONN_IP_TYPE, dataParcel,
        replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Strategy switch fail! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    TELEPHONY_LOGI("end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        ipType = replyParcel.ReadString();
    }

    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataServiceProxy::IsNeedDoRecovery(int32_t slotId, bool needDoRecovery)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteBool(needDoRecovery);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::IS_NEED_DO_RECOVERY, dataParcel,
        replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function IsNeedDoRecovery call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::InitCellularDataController(int32_t slotId)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteInt32(slotId);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::INIT_CELLULAR_DATA_CONTROLLER, dataParcel,
        replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function InitCellularDataController call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::EstablishAllApnsIfConnectable(int32_t slotId)
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

    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::ESTABLISH_ALL_APNS_IF_CONNECTABLE,
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("EstablishAllApnsIfConnectable fail! errCode: %{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::ReleaseCellularDataConnection(int32_t slotId)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!dataParcel.WriteInt32(slotId)) {
        TELEPHONY_LOGE("write slotId failed");
        return TELEPHONY_ERR_WRITE_DATA_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::RELEASE_CELLULAR_DATA_CONNECTION,
                                        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function ReleaseCellularDataConnection call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result;
    if (!replyParcel.ReadInt32(result)) {
        TELEPHONY_LOGE("read reply result failed");
    }
    return result;
}

int32_t CellularDataServiceProxy::GetCellularDataSupplierId(int32_t slotId, uint64_t capability, uint32_t &supplierId)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteUint64(capability);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::GET_CELLULAR_DATA_SUPPLIERID,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetCellularDataSupplierId call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    TELEPHONY_LOGD("end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        supplierId = replyParcel.ReadUint32();
    }
    return result;
}

int32_t CellularDataServiceProxy::CorrectNetSupplierNoAvailable(int32_t slotId)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteInt32(slotId);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::CORRECT_NET_SUPPLIER_NO_AVAILABLE,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function CorrectNetSupplierNoAvailable call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::GetSupplierRegisterState(uint32_t supplierId, int32_t &regState)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteUint32(supplierId);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::GET_SUPPLIER_REGISTER_STATE,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetSupplierRegisterState call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    TELEPHONY_LOGD("end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        regState = replyParcel.ReadInt32();
    }
    return result;
}

int32_t CellularDataServiceProxy::GetIfSupportDunApn(bool &isSupportDun)
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
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::GET_IF_SUPPORT_DUN_APN, data,
        reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    if (result == TELEPHONY_ERR_SUCCESS) {
        isSupportDun = reply.ReadBool();
    }
    return result;
}

int32_t CellularDataServiceProxy::GetDefaultActReportInfo(int32_t slotId, ApnActivateReportInfo &info)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteInt32(slotId);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::GET_DEFAULT_ACT_REPORT_INFO,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetDefaultActReportInfo call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    TELEPHONY_LOGD("end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        info.actTimes = replyParcel.ReadUint32();
        info.averDuration = replyParcel.ReadUint32();
        info.topReason = replyParcel.ReadUint32();
        info.actSuccTimes = replyParcel.ReadUint32();
    }
    return result;
}

int32_t CellularDataServiceProxy::GetInternalActReportInfo(int32_t slotId, ApnActivateReportInfo &info)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteInt32(slotId);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::GET_INTERNAL_ACT_REPORT_INFO,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function GetInternalActReportInfo call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    TELEPHONY_LOGD("end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        info.actTimes = replyParcel.ReadUint32();
        info.averDuration = replyParcel.ReadUint32();
        info.topReason = replyParcel.ReadUint32();
        info.actSuccTimes = replyParcel.ReadUint32();
    }
    return result;
}

int32_t CellularDataServiceProxy::QueryApnIds(ApnInfo apnInfo, std::vector<uint32_t> &apnIdList)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteString16(apnInfo.apnName);
    dataParcel.WriteString16(apnInfo.apn);
    dataParcel.WriteString16(apnInfo.mcc);
    dataParcel.WriteString16(apnInfo.mnc);
    dataParcel.WriteString16(apnInfo.user);
    dataParcel.WriteString16(apnInfo.type);
    dataParcel.WriteString16(apnInfo.proxy);
    dataParcel.WriteString16(apnInfo.mmsproxy);
    TELEPHONY_LOGI("QueryApnIds type: %{public}s", Str16ToStr8(apnInfo.type).c_str());
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::QUERY_APN_INFO,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function QueryApnIds call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    TELEPHONY_LOGI("QueryApnIds end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        int32_t size = replyParcel.ReadInt32();
        if (size > MAX_REPLY_COUNT) {
            TELEPHONY_LOGE("QueryApnIds size error = %{public}d", size);
            return result;
        }
        TELEPHONY_LOGI("QueryApnIds size = %{public}d", size);
        apnIdList.clear();
        for (int i = 0; i < size; i++) {
            int apnId = replyParcel.ReadInt32();
            TELEPHONY_LOGI("QueryApnIds success apnId = %{public}d", apnId);
            apnIdList.emplace_back(apnId);
        }
    }
    return result;
}

int32_t CellularDataServiceProxy::SetPreferApn(int32_t apnId)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteInt32(apnId);
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::SET_PREFER_APN,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function SetPreferApn call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return replyParcel.ReadInt32();
}

int32_t CellularDataServiceProxy::QueryAllApnInfo(std::vector<ApnInfo> &apnInfoList)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    if (!dataParcel.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<OHOS::IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("remote is nullptr!");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t error = remote->SendRequest((uint32_t)CellularDataInterfaceCode::QUERY_ALL_APN_INFO,
        dataParcel, replyParcel, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("function QueryAllApnInfo call failed! errCode:%{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = replyParcel.ReadInt32();
    TELEPHONY_LOGI("QueryAllApnInfo end: result=%{public}d", result);
    if (result == TELEPHONY_ERR_SUCCESS) {
        int32_t size = replyParcel.ReadInt32();
        if (size > MAX_REPLY_COUNT) {
            TELEPHONY_LOGE("QueryAllApnInfo size error = %{public}d", size);
            return result;
        }
        TELEPHONY_LOGI("QueryAllApnInfo size = %{public}d", size);
        apnInfoList.clear();
        for (int i = 0; i < size; i++) {
            ApnInfo apnInfo;
            apnInfo.ReadFromParcel(replyParcel);
            apnInfoList.emplace_back(apnInfo);
        }
    }
    return result;
}

int32_t CellularDataServiceProxy::SendUrspDecodeResult(int32_t slotId, std::vector<uint8_t> buffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(slotId);
    int32_t bufferlen = (int32_t)buffer.size();
    if (bufferlen <= 0 || bufferlen > BUFFER_MAX) {
        TELEPHONY_LOGE("buffer length is invalid: %{public}d", bufferlen);
        return TELEPHONY_ERR_ARGUMENT_INVALID;
    }
    data.WriteInt32(bufferlen);
    for (int i = 0; i < bufferlen; ++i) {
        data.WriteInt32(buffer[i]);
    }
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
 
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::SEND_MANAGE_UEPOLICY_DECODE_RESULT,
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendUrspDecodeResult fail! errCode: %{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t CellularDataServiceProxy::SendUePolicySectionIdentifier(int32_t slotId, std::vector<uint8_t> buffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(slotId);
    int32_t bufferlen = (int32_t)buffer.size();
    if (bufferlen <= 0 || bufferlen > BUFFER_MAX) {
        TELEPHONY_LOGE("buffer length is invalid: %{public}d", bufferlen);
        return TELEPHONY_ERR_ARGUMENT_INVALID;
    }
    data.WriteInt32(bufferlen);
    for (int i = 0; i < bufferlen; ++i) {
        data.WriteInt32(buffer[i]);
    }
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
 
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::SEND_UE_STATE_INDICATION,
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendUePolicySectionIdentifier fail! errCode: %{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}
 
int32_t CellularDataServiceProxy::SendImsRsdList(int32_t slotId, std::vector<uint8_t> buffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(slotId);
    int32_t bufferlen = (int32_t)buffer.size();
    if (bufferlen <= 0 || bufferlen > BUFFER_MAX) {
        TELEPHONY_LOGE("buffer length is invalid: %{public}d", bufferlen);
        return TELEPHONY_ERR_ARGUMENT_INVALID;
    }
    data.WriteInt32(bufferlen);
    for (int i = 0; i < bufferlen; ++i) {
        data.WriteInt32(buffer[i]);
    }
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
 
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::SEND_IMS_RSDLIST,
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendImsRsdList fail! errCode: %{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}
 
int32_t CellularDataServiceProxy::GetNetworkSliceAllowedNssai(int32_t slotId, std::vector<uint8_t> buffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(slotId);
    int32_t bufferlen = (int32_t)buffer.size();
    if (bufferlen <= 0 || bufferlen > BUFFER_MAX) {
        TELEPHONY_LOGE("buffer length is invalid: %{public}d", bufferlen);
        return TELEPHONY_ERR_ARGUMENT_INVALID;
    }
    data.WriteInt32(bufferlen);
    for (int i = 0; i < bufferlen; ++i) {
        data.WriteInt32(buffer[i]);
    }
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
 
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::SYNC_ALLOWED_NSSAI_WITH_MODEM,
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetNetworkSliceAllowedNssai fail! errCode: %{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}
 
int32_t CellularDataServiceProxy::GetNetworkSliceEhplmn(int32_t slotId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(CellularDataServiceProxy::GetDescriptor())) {
        TELEPHONY_LOGE("write interface token failed!");
        return TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteInt32(slotId);
    if (Remote() == nullptr) {
        TELEPHONY_LOGE("remote is null");
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
 
    int32_t error = Remote()->SendRequest((uint32_t)CellularDataInterfaceCode::SYNC_EHPLMN_WITH_MODEM,
        data, reply, option);
    if (error != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetNetworkSliceEhplmn fail! errCode: %{public}d", error);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = reply.ReadInt32();
    return result;
}

} // namespace Telephony
} // namespace OHOS
