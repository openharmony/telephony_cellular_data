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

#ifndef CELLULAR_DATA_CLIENT_H
#define CELLULAR_DATA_CLIENT_H

#include <cstdint>
#include <iremote_object.h>
#include <singleton.h>

#include "data_sim_account_call_back.h"
#include "i_cellular_data_manager.h"
#include "sim_account_callback.h"
#include "apn_item.h"
#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
class CellularDataClient : public DelayedRefSingleton<CellularDataClient> {
    DECLARE_DELAYED_REF_SINGLETON(CellularDataClient);

public:
    /**
     * @brief Whether cellular data service is connected.
     *
     * @return Return true on connected, false on not connected.
     */
    bool IsConnect();

    /**
     * @brief Whether to enable cellular data user switch
     *
     * @param enable Enable or not.
     * @return Return 84082688 invalid parameter, 0 data enable success, others enable fail.
     */
    int32_t EnableCellularData(bool enable);

    /**
     * @brief Whether to enable intelligence switch
     *
     * @param enable Enable or not.
     * @return Return 84082688 invalid parameter, 1 data enable success, 0 enable fail.
     */
    int32_t EnableIntelligenceSwitch(bool enable);

    /**
     * @brief Whether the cellular data user switch is enabled
     *
     * @param dataEnabled Indicates the result of data enabled status.
     * @return Returns error code.
     */
    int32_t IsCellularDataEnabled(bool &dataEnabled);

    /**
     * @brief Cellular data connection status
     *
     * @return Returns data connection status defined in DataConnectionStatus.
     */
    int32_t GetCellularDataState();

    /**
     * @brief Get the apn status based on slotId and apnType
     *
     * @return Returns apn status
     */
    int32_t GetApnState(int32_t slotId, const std::string &apnType);

    /**
     * Get IntelligenceSwitch State
     *
     * @param switchState Returns IntelligenceSwitch State
     * @return Returns 0 on success, others on failure.
     */
    int32_t GetIntelligenceSwitchState(bool &switchState);

    /**
     * @brief Get recovery state
     */
    int32_t GetDataRecoveryState();

    /**
     * @brief Whether roaming is allowed
     *
     * @param slotId Indicates card slot identification
     * @param dataRoamingEnabled Indicates the result of data roaming enabled status.
     * @return Returns error code.
     */
    int32_t IsCellularDataRoamingEnabled(int32_t slotId, bool &dataRoamingEnabled);

    /**
     * @brief Whether roaming switches are allowed
     *
     * @param slotId card slot identification
     * @param enable Whether roaming switches are allowed
     * @return Returns 0 on failure, 1 on failure. 84082688 invalid parameter
     */
    int32_t EnableCellularDataRoaming(int32_t slotId, bool enable);

    /**
     * @brief Get the slotId that uses the data traffic by default
     *
     * @return Returns the default settings data card, -1 error code
     */
    int32_t GetDefaultCellularDataSlotId();

    /**
     * Get the simId that uses the data traffic by default
     *
     * @param simId Returns default settings data sim id
     * @return Returns 0 on success, others on failure.
     */
    int32_t GetDefaultCellularDataSimId(int32_t &simId);

    /**
     * @brief Set the slotId that uses the data traffic by default
     *
     * @param slotId card slot identification
     * @return 0 set success, others set fail, 84082688 invalid parameter
     */
    int32_t SetDefaultCellularDataSlotId(int32_t slotId);

    /**
     * @brief Get data packet type
     *
     * @return Returns cell data flow type defined in CellDataFlowType.
     */
    int32_t GetCellularDataFlowType();

    /**
     * @brief Whether cellular data has internet capability.
     *
     * @param slotId Card slot identification.
     * @param cid Context identification.
     * @return Return 1 if has, 0 if hasn't.
     */
    int32_t HasInternetCapability(int32_t slotId, int32_t cid);

    /**
     * @brief Clear cellular data connections.
     *
     * @param slotId Card slot identification.
     * @return 1 set success, 0 set fail, 84082688 invalid parameter
     */
    int32_t ClearCellularDataConnections(int32_t slotId);

    int32_t ClearAllConnections(int32_t slotId, DisConnectionReason reason);

    int32_t HandleApnChanged(int32_t slotId);

    /**
     * @brief Get cellular data proxy.
     *
     * @return Cellular data service.
     */
    sptr<ICellularDataManager> GetProxy();

    /**
     * @brief Update the slotId that uses the data traffic by default
     *
     * @return Returns the default settings data card, -1 error code
     */
    int32_t UpdateDefaultCellularDataSlotId();

    /**
     * @brief Get data connections apn attribute.
     *
     * @param slotId Card slot identification.
     * @param apnType Indicates the APN attribute used by the data connection.
     * @return 1 set success, 0 set fail
     */
    int32_t GetDataConnApnAttr(int32_t slotId, ApnItem::Attribute &apnAttr);

    /**
     * @brief Get data connections ip type.
     *
     * @param slotId Card slot identification.
     * @param ipType Indicates IP array after data connection.
     * @return 1 set success, 0 set fail
     */
    int32_t GetDataConnIpType(int32_t slotId, std::string &ipType);

    /**
     * @brief Whether do recovery is needed.
     *
     * @param slotId Card slot identification.
     * @param needDoRecovery Whether do recovery is needed.
     * @return 1 set success, 0 set fail
     */
    int32_t IsNeedDoRecovery(int32_t slotId, bool needDoRecovery);

    /**
     * @brief Init CellularDataController instance.
     *
     * @param slotId Card slot identification.
     * @return 1 set success, 0 set fail
     */
    int32_t InitCellularDataController(int32_t slotId);

private:
    class CellularDataDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit CellularDataDeathRecipient(CellularDataClient &client) : client_(client) {}
        ~CellularDataDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        CellularDataClient &client_;
    };

    void OnRemoteDied(const wptr<IRemoteObject> &remote);
    void RegisterSimAccountCallback();
    void UnregisterSimAccountCallback();
    bool IsValidSlotId(int32_t slotId);
    bool IsCellularDataSysAbilityExist(sptr<IRemoteObject> &object);

private:
    std::mutex mutexProxy_;
    sptr<ICellularDataManager> proxy_ { nullptr };
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ { nullptr };
    sptr<SimAccountCallback> callback_ { nullptr };
    static int32_t defaultCellularDataSlotId_;
    static int32_t defaultCellularDataSimId_;
    bool registerStatus_ = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_CLIENT_H
