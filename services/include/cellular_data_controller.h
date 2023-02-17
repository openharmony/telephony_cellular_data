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

#ifndef CELLULAR_DATA_CONTROLLER_H
#define CELLULAR_DATA_CONTROLLER_H

#include <memory>

#include "cellular_data_constant.h"
#include "cellular_data_handler.h"
#include "cellular_data_rdb_observer.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Telephony {
class CellularDataController : public AppExecFwk::EventHandler {
public:
    explicit CellularDataController(std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    ~CellularDataController();
    void Init();
    bool ReleaseNet(const NetRequest &request);
    bool RequestNet(const NetRequest &request);
    int32_t SetCellularDataEnable(bool userDataEnabled);
    int32_t IsCellularDataEnabled(bool &dataEnabled) const;
    int32_t SetCellularDataRoamingEnabled(bool dataRoamingEnabled);
    ApnProfileState GetCellularDataState() const;
    ApnProfileState GetCellularDataState(const std::string &apnType) const;
    int32_t IsCellularDataRoamingEnabled(bool &dataRoamingEnabled) const;
    void SetDataPermitted(bool dataPermitted) const;
    void AsynchronousRegister();
    bool HandleApnChanged();
    int32_t GetCellularDataFlowType();
    void EstablishDataConnection();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);
    int32_t SetPolicyDataOn(bool enable);
    bool IsRestrictedMode() const;
    DisConnectionReason GetDisConnectionReason();
    bool HasInternetCapability(const int32_t cid) const;
    bool ClearAllConnections(DisConnectionReason reason) const;

private:
    void RegisterEvents();
    void UnRegisterEvents();
    void RegisterDatabaseObserver();
    void UnRegisterDatabaseObserver();

private:
    std::shared_ptr<CellularDataHandler> cellularDataHandler_;
    sptr<CellularDataRdbObserver> cellularDataRdbObserver_;
    sptr<ISystemAbilityStatusChange> netManagerListener_ = nullptr;
    const int32_t slotId_;

private:
    class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(int32_t slotId, std::shared_ptr<CellularDataHandler> handler);
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

    private:
        bool isNetStopped_ = false;
        const int32_t slotId_;
        std::shared_ptr<CellularDataHandler> handler_;
    };
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_CONTROLLER_H
