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
    TELEPHONY_LOGI("end: result=%{public}d", result);
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
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
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
} // namespace Telephony
} // namespace OHOS
