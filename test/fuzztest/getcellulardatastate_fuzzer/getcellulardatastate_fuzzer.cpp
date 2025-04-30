/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "getcellulardatastate_fuzzer.h"

#include <cstddef>
#include <cstdint>
#define private public
#include "adddatatoken_fuzzer.h"
#include "cellular_data_ipc_interface_code.h"
#include "cellular_data_service.h"
#include "cellular_data_types.h"
#include "cellular_data_manager_stub.h"
#include "data_sim_account_callback.h"
#include "system_ability_definition.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "icellular_data_manager.h"

using namespace OHOS::Telephony;
namespace OHOS {
static bool g_isInited = false;
constexpr int32_t SLOT_NUM_MAX = 3;
constexpr int32_t APN_ID_MAX = 5;
constexpr int32_t APN_STRING_LENGTH = 10;
constexpr int32_t NET_CAPABILITY_MAX = 32;
static constexpr const char16_t *DESCRIPTOR = u"OHOS.Telephony.ICellularDataManager";
static inline const std::u16string GetDescriptor()
{
    return DESCRIPTOR;
}
bool IsServiceInited()
{
    if (!g_isInited) {
        DelayedSingleton<CellularDataService>::GetInstance()->OnStart();
        if (DelayedSingleton<CellularDataService>::GetInstance()->GetServiceRunningState() ==
            static_cast<int32_t>(ServiceRunningState::STATE_RUNNING)) {
            g_isInited = true;
        }
    }
    return g_isInited;
}

void EnableCellularData(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_ENABLE_CELLULAR_DATA);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void EnableIntelligenceSwitch(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_ENABLE_INTELLIGENCE_SWITCH);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetCellularDataState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_CELLULAR_DATA_STATE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void IsCellularDataEnabled(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_IS_CELLULAR_DATA_ENABLED);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void IsCellularDataRoamingEnabled(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_IS_CELLULAR_DATA_ROAMING_ENABLED);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetDefaultCellularDataSlotId(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_DEFAULT_CELLULAR_DATA_SLOT_ID);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void EnableCellularDataRoaming(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_ENABLE_CELLULAR_DATA_ROAMING);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void SetDefaultCellularDataSlotId(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_SET_DEFAULT_CELLULAR_DATA_SLOT_ID);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void HasInternetCapability(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_HAS_INTERNET_CAPABILITY);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void ClearCellularDataConnections(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_CLEAR_CELLULAR_DATA_CONNECTIONS);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetCellularDataFlowType(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_CELLULAR_DATA_FLOW_TYPE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void RegisterSimAccountCallback(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    sptr<SimAccountCallback> callback = new (std::nothrow) DataSimAccountCallback();
    if (callback == nullptr) {
        return;
    }
    dataMessageParcel.WriteRemoteObject(callback->AsObject());
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_REGISTER_SIM_ACCOUNT_CALLBACK);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void UnregisterSimAccountCallback(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    sptr<SimAccountCallback> callback = new (std::nothrow) DataSimAccountCallback();
    if (callback == nullptr) {
        return;
    }
    dataMessageParcel.WriteRemoteObject(callback->AsObject());
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_UNREGISTER_SIM_ACCOUNT_CALLBACK);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetDefaultActReportInfo(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_DEFAULT_ACT_REPORT_INFO);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetInternalActReportInfo(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_INTERNAL_ACT_REPORT_INFO);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void HandleApnChanged(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_HANDLE_APN_CHANGED);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetDefaultCellularDataSimId(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_DEFAULT_CELLULAR_DATA_SIM_ID);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void ClearAllConnections(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    int32_t reason = fdp.ConsumeIntegralInRange<int32_t>(
        0, static_cast<int32_t>(DisConnectionReason::REASON_PERMANENT_REJECT));
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteInt32(reason);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_CLEAR_ALL_CONNECTIONS);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetDataConnApnAttr(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_DATA_CONN_APN_ATTR);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetDataConnIpType(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_DATA_CONN_IP_TYPE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetApnState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    std::string apnType = fdp.ConsumeRandomLengthString(APN_STRING_LENGTH);
    if (!dataMessageParcel.WriteString16(Str8ToStr16(apnType))) {
        return;
    }
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_APN_STATE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetDataRecoveryState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_DATA_RECOVERY_STATE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetCellularDataSupplierId(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    dataMessageParcel.WriteInt32(slotId);
    uint64_t capability = fdp.ConsumeIntegralInRange<uint64_t>(0, NET_CAPABILITY_MAX);
    dataMessageParcel.WriteUint64(capability);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_CELLULAR_DATA_SUPPLIER_ID);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void IsNeedDoRecovery(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_IS_NEED_DO_RECOVERY);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void CorrectNetSupplierNoAvailable(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_CORRECT_NET_SUPPLIER_NO_AVAILABLE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetIfSupportDunApn(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_IF_SUPPORT_DUN_APN);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void QueryApnIds(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    ApnInfo apnInfo;
    dataMessageParcel.WriteParcelable(&apnInfo);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_QUERY_APN_IDS);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void SetPreferApn(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t apnId = fdp.ConsumeIntegralInRange<uint32_t>(0, APN_ID_MAX);
    dataMessageParcel.WriteInt32(apnId);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_SET_PREFER_APN);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void InitCellularDataController(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_INIT_CELLULAR_DATA_CONTROLLER);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void EstablishAllApnsIfConnectable(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_ESTABLISH_ALL_APNS_IF_CONNECTABLE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void ReleaseCellularDataConnection(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_RELEASE_CELLULAR_DATA_CONNECTION);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void GetSupplierRegisterState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t slotId = fdp.ConsumeIntegralInRange<uint32_t>(0, SLOT_NUM_MAX);
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_GET_SUPPLIER_REGISTER_STATE);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    EnableCellularData(data, size);
    EnableIntelligenceSwitch(data, size);
    GetCellularDataState(data, size);
    IsCellularDataEnabled(data, size);
    IsCellularDataRoamingEnabled(data, size);
    GetDefaultCellularDataSlotId(data, size);
    GetCellularDataFlowType(data, size);
    EnableCellularDataRoaming(data, size);
    SetDefaultCellularDataSlotId(data, size);
    HasInternetCapability(data, size);
    ClearCellularDataConnections(data, size);
    RegisterSimAccountCallback(data, size);
    UnregisterSimAccountCallback(data, size);
    GetDefaultActReportInfo(data, size);
    GetInternalActReportInfo(data, size);
    HandleApnChanged(data, size);
    GetDefaultCellularDataSimId(data, size);
    ClearAllConnections(data, size);
    GetDataConnApnAttr(data, size);
    GetDataConnIpType(data, size);
    GetApnState(data, size);
    GetDataRecoveryState(data, size);
    IsNeedDoRecovery(data, size);
    InitCellularDataController(data, size);
    EstablishAllApnsIfConnectable(data, size);
    ReleaseCellularDataConnection(data, size);
    GetSupplierRegisterState(data, size);
    GetCellularDataSupplierId(data, size);
    CorrectNetSupplierNoAvailable(data, size);
    GetIfSupportDunApn(data, size);
    QueryApnIds(data, size);
    SetPreferApn(data, size);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AddDataTokenFuzzer token;
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::DelayedSingleton<CellularDataService>::DestroyInstance();
    return 0;
}
