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

namespace OHOS {
namespace Telephony {
class CellularDataClient : public DelayedRefSingleton<CellularDataClient> {
    DECLARE_DELAYED_REF_SINGLETON(CellularDataClient);

public:
    bool IsConnect() const;
    int32_t EnableCellularData(bool enable);
    int32_t IsCellularDataEnabled(bool &dataEnabled);
    int32_t GetCellularDataState();
    int32_t IsCellularDataRoamingEnabled(int32_t slotId, bool &dataRoamingEnabled);
    int32_t EnableCellularDataRoaming(int32_t slotId, bool enable);
    int32_t GetDefaultCellularDataSlotId();
    int32_t SetDefaultCellularDataSlotId(int32_t slotId);
    int32_t GetCellularDataFlowType();
    int32_t HasInternetCapability(int32_t slotId, int32_t cid);
    int32_t ClearCellularDataConnections(int32_t slotId);
    sptr<ICellularDataManager> GetProxy();
    int32_t UpdateDefaultCellularDataSlotId();

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

private:
    std::mutex mutexProxy_;
    sptr<ICellularDataManager> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ { nullptr };
    sptr<SimAccountCallback> callback_ { nullptr };
    int32_t defaultCellularDataSlotId_;
    bool registerStatus_ = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_CLIENT_H