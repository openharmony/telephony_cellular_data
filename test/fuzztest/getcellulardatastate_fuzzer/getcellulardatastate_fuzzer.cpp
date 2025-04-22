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
#include "cellular_data_manager_stub.h"
#include "system_ability_definition.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "icellular_data_manager.h"

using namespace OHOS::Telephony;
namespace OHOS {
static bool g_isInited = false;
constexpr int32_t SLOT_NUM_MAX = 3;
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
    uint32_t code = static_cast<uint32_t>(ICellularDataManagerIpcCode::COMMAND_IS_CELLULAR_DATA_ROAMING_ENABLED);
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
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
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
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
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

void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    EnableCellularData(data, size);
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
