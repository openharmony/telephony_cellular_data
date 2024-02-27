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

#ifndef CELLULAR_DATA_SERVICE_STUB_H
#define CELLULAR_DATA_SERVICE_STUB_H

#include <map>

#include "iremote_object.h"
#include "iremote_stub.h"

#include "cellular_data_ipc_interface_code.h"
#include "i_cellular_data_manager.h"

namespace OHOS {
namespace Telephony {
class CellularDataServiceStub : public IRemoteStub<ICellularDataManager> {
public:
    CellularDataServiceStub();
    ~CellularDataServiceStub();
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t OnRegisterSimAccountCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnUnregisterSimAccountCallback(MessageParcel &data, MessageParcel &reply);

private:
    int32_t OnIsCellularDataEnabled(MessageParcel &data, MessageParcel &reply);
    int32_t OnEnableCellularData(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetCellularDataState(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsCellularDataRoamingEnabled(MessageParcel &data, MessageParcel &reply);
    int32_t OnEnableCellularDataRoaming(MessageParcel &data, MessageParcel &reply);
    int32_t OnHandleApnChanged(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetDefaultCellularDataSlotId(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetDefaultCellularDataSimId(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetDefaultCellularDataSlotId(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetCellularDataFlowType(MessageParcel &data, MessageParcel &reply);
    int32_t OnHasInternetCapability(MessageParcel &data, MessageParcel &reply);
    int32_t OnClearCellularDataConnections(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetDataConnApnAttr(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetDataConnIpType(MessageParcel &data, MessageParcel &reply);
    int32_t OnClearAllConnections(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetApnState(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetRecoveryState(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsNeedDoRecovery(MessageParcel &data, MessageParcel &reply);
    int32_t OnEnableIntelligenceSwitch(MessageParcel &data, MessageParcel &reply);
    int32_t OnInitCellularDataController(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetIntelligenceSwitchState(MessageParcel &data, MessageParcel &reply);

private:
    using Fun = int32_t (CellularDataServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, Fun> eventIdFunMap_ {
        { (uint32_t)CellularDataInterfaceCode::IS_CELLULAR_DATA_ENABLED,
            &CellularDataServiceStub::OnIsCellularDataEnabled },
        { (uint32_t)CellularDataInterfaceCode::ENABLE_CELLULAR_DATA,
            &CellularDataServiceStub::OnEnableCellularData },
        { (uint32_t)CellularDataInterfaceCode::GET_CELLULAR_DATA_STATE,
            &CellularDataServiceStub::OnGetCellularDataState },
        { (uint32_t)CellularDataInterfaceCode::IS_DATA_ROAMING_ENABLED,
            &CellularDataServiceStub::OnIsCellularDataRoamingEnabled },
        { (uint32_t)CellularDataInterfaceCode::ENABLE_DATA_ROAMING,
            &CellularDataServiceStub::OnEnableCellularDataRoaming },
        { (uint32_t)CellularDataInterfaceCode::APN_DATA_CHANGED, &CellularDataServiceStub::OnHandleApnChanged },
        { (uint32_t)CellularDataInterfaceCode::GET_DEFAULT_SLOT_ID,
            &CellularDataServiceStub::OnGetDefaultCellularDataSlotId },
        { (uint32_t)CellularDataInterfaceCode::GET_DEFAULT_SIM_ID,
            &CellularDataServiceStub::OnGetDefaultCellularDataSimId },
        { (uint32_t)CellularDataInterfaceCode::SET_DEFAULT_SLOT_ID,
            &CellularDataServiceStub::OnSetDefaultCellularDataSlotId },
        { (uint32_t)CellularDataInterfaceCode::GET_FLOW_TYPE_ID,
            &CellularDataServiceStub::OnGetCellularDataFlowType },
        { (uint32_t)CellularDataInterfaceCode::HAS_CAPABILITY, &CellularDataServiceStub::OnHasInternetCapability },
        { (uint32_t)CellularDataInterfaceCode::CLEAR_ALL_CONNECTIONS,
            &CellularDataServiceStub::OnClearCellularDataConnections },
        { (uint32_t)CellularDataInterfaceCode::CLEAR_ALL_CONNECTIONS_USE_REASON,
            &CellularDataServiceStub::OnClearAllConnections },
        { (uint32_t)CellularDataInterfaceCode::REG_SIM_ACCOUNT_CALLBACK,
            &CellularDataServiceStub::OnRegisterSimAccountCallback },
        { (uint32_t)CellularDataInterfaceCode::UN_REG_SIM_ACCOUNT_CALLBACK,
            &CellularDataServiceStub::OnUnregisterSimAccountCallback },
        { (uint32_t)CellularDataInterfaceCode::GET_DATA_CONN_APN_ATTR,
            &CellularDataServiceStub::OnGetDataConnApnAttr },
        { (uint32_t)CellularDataInterfaceCode::GET_DATA_CONN_IP_TYPE,
            &CellularDataServiceStub::OnGetDataConnIpType },
        { (uint32_t)CellularDataInterfaceCode::GET_CELLULAR_DATA_APN_STATE,
            &CellularDataServiceStub::OnGetApnState },
        { (uint32_t)CellularDataInterfaceCode::GET_RECOVERY_STATE,
            &CellularDataServiceStub::OnGetRecoveryState },
        { (uint32_t)CellularDataInterfaceCode::IS_NEED_DO_RECOVERY,
            &CellularDataServiceStub::OnIsNeedDoRecovery },
        { (uint32_t)CellularDataInterfaceCode::ENABLE_INTELLIGENCE_SWITCH,
            &CellularDataServiceStub::OnEnableIntelligenceSwitch },
        { (uint32_t)CellularDataInterfaceCode::INIT_CELLULAR_DATA_CONTROLLER,
            &CellularDataServiceStub::OnInitCellularDataController },
        { (uint32_t)CellularDataInterfaceCode::GET_INTELLIGENCE_SWITCH_STATE,
            &CellularDataServiceStub::OnGetIntelligenceSwitchState },    
    };
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_SERVICE_STUB_H
