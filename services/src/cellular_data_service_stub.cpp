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

#include "cellular_data_service_stub.h"

#include <string_ex.h>

#include "cellular_data_controller.h"
#include "cellular_data_service.h"
#include "ipc_skeleton.h"
#include "sim_account_callback_proxy.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CellularDataServiceStub::CellularDataServiceStub() = default;

CellularDataServiceStub::~CellularDataServiceStub() = default;

int32_t CellularDataServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescriptor = CellularDataServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    // NetManager has no transport description
    if (myDescriptor != remoteDescriptor) {
        TELEPHONY_LOGE("descriptor check fail!");
        return TELEPHONY_ERR_DESCRIPTOR_MISMATCH;
    }
    std::map<uint32_t, Fun>::iterator it = eventIdFunMap_.find(code);
    if (it != eventIdFunMap_.end()) {
        if (it->second != nullptr) {
            return (this->*(it->second))(data, reply);
        }
    } else {
        TELEPHONY_LOGE("event code is not exist");
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t CellularDataServiceStub::OnIsCellularDataEnabled(MessageParcel &data, MessageParcel &reply)
{
    bool dataEnabled = false;
    int32_t result = IsCellularDataEnabled(dataEnabled);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("OnIsCellularDataEnabled write int32 reply failed.");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    if (result != TELEPHONY_ERR_SUCCESS) {
        return result;
    }
    if (!reply.WriteBool(dataEnabled)) {
        TELEPHONY_LOGE("OnIsCellularDataEnabled write bool reply failed.");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CellularDataServiceStub::OnEnableCellularData(MessageParcel &data, MessageParcel &reply)
{
    bool enable = data.ReadBool();
    int32_t result = EnableCellularData(enable);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnGetCellularDataState(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetCellularDataState();
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnIsCellularDataRoamingEnabled(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    bool dataRoamingEnabled = false;
    int32_t result = IsCellularDataRoamingEnabled(slotId, dataRoamingEnabled);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("OnIsCellularDataRoamingEnabled write int32 reply failed.");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    if (result != TELEPHONY_ERR_SUCCESS) {
        return result;
    }
    if (!reply.WriteBool(dataRoamingEnabled)) {
        TELEPHONY_LOGE("OnIsCellularDataRoamingEnabled write bool reply failed.");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }

    return TELEPHONY_SUCCESS;
}

int32_t CellularDataServiceStub::OnEnableCellularDataRoaming(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    bool enable = data.ReadBool();
    int32_t result = EnableCellularDataRoaming(slotId, enable);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnHandleApnChanged(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t result = HandleApnChanged(slotId);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnGetDefaultCellularDataSlotId(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetDefaultCellularDataSlotId();
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnSetDefaultCellularDataSlotId(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t result = SetDefaultCellularDataSlotId(slotId);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnGetCellularDataFlowType(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetCellularDataFlowType();
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnHasInternetCapability(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t cid = data.ReadInt32();
    int32_t result = HasInternetCapability(slotId, cid);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnClearCellularDataConnections(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t result = ClearCellularDataConnections(slotId);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CellularDataServiceStub::OnRegisterSimAccountCallback(MessageParcel &data, MessageParcel &reply)
{
    sptr<SimAccountCallback> callback = iface_cast<SimAccountCallback>(data.ReadRemoteObject());
    int32_t result;
    if (callback == nullptr) {
        TELEPHONY_LOGE("callback is nullptr!");
        result = TELEPHONY_ERR_ARGUMENT_NULL;
    } else {
        result = RegisterSimAccountCallback(callback);
    }
    reply.WriteInt32(result);
    return result;
}

int32_t CellularDataServiceStub::OnUnregisterSimAccountCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = UnregisterSimAccountCallback();
    reply.WriteInt32(result);
    return result;
}
} // namespace Telephony
} // namespace OHOS