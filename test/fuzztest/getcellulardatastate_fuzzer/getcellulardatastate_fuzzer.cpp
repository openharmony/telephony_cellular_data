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
#include "cellular_data_service.h"
#include "cellular_data_service_stub.h"
#include "system_ability_definition.h"

using namespace OHOS::Telephony;
namespace OHOS {
static bool g_isInited = false;
constexpr int32_t SLOT_NUM = 2;

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

void OnRemoteRequest(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(CellularDataServiceStub::GetDescriptor())) {
        return;
    }
    size_t dataSize = size - sizeof(uint32_t);
    dataMessageParcel.WriteBuffer(data + sizeof(uint32_t), dataSize);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(size);
    MessageParcel reply;
    MessageOption option;
    DelayedSingleton<CellularDataService>::GetInstance()->OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void EnableCellularData(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnEnableCellularData(dataMessageParcel, reply);
}

void GetCellularDataState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnGetCellularDataState(dataMessageParcel, reply);
}

void IsCellularDataEnabled(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnIsCellularDataEnabled(dataMessageParcel, reply);
}

void IsCellularDataRoamingEnabled(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    size_t dataSize = size - sizeof(int32_t);
    dataMessageParcel.WriteBuffer(data + sizeof(int32_t), dataSize);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnIsCellularDataRoamingEnabled(dataMessageParcel, reply);
}

void GetDefaultCellularDataSlotId(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnGetDefaultCellularDataSlotId(dataMessageParcel, reply);
}

void EnableCellularDataRoaming(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    size_t dataSize = size - sizeof(int32_t);
    dataMessageParcel.WriteBuffer(data + sizeof(int32_t), dataSize);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnEnableCellularDataRoaming(dataMessageParcel, reply);
}

void SetDefaultCellularDataSlotId(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    size_t dataSize = size - sizeof(int32_t);
    dataMessageParcel.WriteBuffer(data + sizeof(int32_t), dataSize);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnSetDefaultCellularDataSlotId(dataMessageParcel, reply);
}

void HasInternetCapability(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    size_t dataSize = size - sizeof(int32_t);
    dataMessageParcel.WriteBuffer(data + sizeof(int32_t), dataSize);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnHasInternetCapability(dataMessageParcel, reply);
}

void ClearCellularDataConnections(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    size_t dataSize = size - sizeof(int32_t);
    dataMessageParcel.WriteBuffer(data + sizeof(int32_t), dataSize);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnClearCellularDataConnections(dataMessageParcel, reply);
}

void GetCellularDataFlowType(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }

    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<CellularDataService>::GetInstance()->OnGetCellularDataFlowType(dataMessageParcel, reply);
}

void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    OnRemoteRequest(data, size);
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
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AddDataTokenFuzzer token;
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
