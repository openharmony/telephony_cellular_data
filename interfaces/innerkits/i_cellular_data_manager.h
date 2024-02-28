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

#ifndef I_CELLULAR_DATA_MANAGER_H
#define I_CELLULAR_DATA_MANAGER_H

#include "iremote_broker.h"
#include "sim_account_callback.h"
#include "apn_item.h"
#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
class ICellularDataManager : public IRemoteBroker {
public:
    /**
     * Whether the cellular data user switch is enabled
     *
     * @return return errorCode of is cellulardata enabled
     */
    virtual int32_t IsCellularDataEnabled(bool &dataEnabled) = 0;

    /**
     * Whether to enable cellular data user switch
     *
     * @param enable allow or not
     * @return return 84082688 invalid parameter, 1 data enable success, 0 enable fail
     */
    virtual int32_t EnableCellularData(bool enable) = 0;

    /**
     * Whether to enable intelligence switch
     *
     * @param enable allow or not
     * @return return 84082688 invalid parameter, 1 data enable success, 0 enable fail
     */
    virtual int32_t EnableIntelligenceSwitch(bool enable) = 0;

    /**
     * Cellular data connection status
     *
     * @return 84082688 Indicates that a cellular data link is unknown
     *         11 Indicates that a cellular data link is disconnected
     *         12 Indicates that a cellular data link is being connected
     *         13 Indicates that a cellular data link is connected
     *         14 Indicates that a cellular data link is suspended
     */
    virtual int32_t GetCellularDataState() = 0;

    /**
     * Whether roaming is allowed
     *
     * @param slotId card slot identification
     * @param dataRoamingEnabled result of data is enabled
     * @return return errorCode of is cellulardata enabled
     */
    virtual int32_t IsCellularDataRoamingEnabled(int32_t slotId, bool &dataRoamingEnabled) = 0;

    /**
     * Whether roaming switches are allowed
     *
     * @param slotId card slot identification
     * @param enable Whether roaming switches are allowed
     * @return Returns 0 on failure, 1 on failure. 84082688 invalid parameter
     */
    virtual int32_t EnableCellularDataRoaming(int32_t slotId, bool enable) = 0;

    /**
     * Processing of APN content changes
     *
     * @param slotId card slot identification
     * @param apns changed apns
     * @return the number of apns created else 84082688 invalid parameter
     */
    virtual int32_t HandleApnChanged(int32_t slotId) = 0;

    /**
     * Get the slotId that uses the data traffic by default
     *
     * @return default settings data card, -1 error code
     */
    virtual int32_t GetDefaultCellularDataSlotId() = 0;

    /**
     * Get the simId that uses the data traffic by default
     *
     * @param simId Returns default settings data sim id
     * @return Returns 0 on success, others on failure.
     */
    virtual int32_t GetDefaultCellularDataSimId(int32_t &simId) = 0;

    /**
     * set the slotId that uses the data traffic by default
     *
     * @return 1 set success, 0 set fail, 84082688 invalid parameter
     */
    virtual int32_t SetDefaultCellularDataSlotId(int32_t slotId) = 0;

    /**
     * get data packet type
     *
     * @return 0 Indicates that there is no uplink or down link data,
     *         1 Indicates that there is only down link data,
     *         2 Indicates that there is only uplink data,
     *         3 Indicates that there is uplink and down link data
     *         4 Indicates that there is no uplink or down link data,
     *           and the bottom-layer link is in the dormant state
     *         84082688 Indicates invalid parameter
     */
    virtual int32_t GetCellularDataFlowType() = 0;

    /**
     * get intelligence switch state
     *
     * @param state the state of intelligence switch
     * @return return 84082688 invalid parameter, 1 data enable success, 0 enable fail
     */
    virtual int32_t GetIntelligenceSwitchState(bool &state) = 0;

    virtual int32_t HasInternetCapability(int32_t slotId, int32_t cid) = 0;

    virtual int32_t ClearCellularDataConnections(int32_t slotId) = 0;

    virtual int32_t ClearAllConnections(int32_t slotId, DisConnectionReason reason) = 0;

    virtual int32_t RegisterSimAccountCallback(const sptr<SimAccountCallback> &callback) = 0;

    virtual int32_t UnregisterSimAccountCallback() = 0;

    virtual int32_t GetDataConnApnAttr(int32_t slotId, ApnItem::Attribute &apnAttr) = 0;

    virtual int32_t GetDataConnIpType(int32_t slotId, std::string &ipType) = 0;

    virtual int32_t GetApnState(int32_t slotId, const std::string &apnType) = 0;

    virtual int32_t GetDataRecoveryState() = 0;

    virtual int32_t IsNeedDoRecovery(int32_t slotId, bool needDoRecovery) = 0;

    virtual int32_t InitCellularDataController(int32_t slotId) = 0;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.telephony.ICellularDataManager");
};
} // namespace Telephony
} // namespace OHOS
#endif // I_CELLULAR_DATA_MANAGER_H
