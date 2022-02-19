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
#include "cellular_data_roaming_observer.h"
#include "cellular_data_setting_observer.h"

namespace OHOS {
namespace Telephony {
class CellularDataController : public AppExecFwk::EventHandler {
public:
    explicit CellularDataController(std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    ~CellularDataController();
    void Init();
    bool ReleaseNet(const NetRequest &request);
    bool RequestNet(const NetRequest &request);
    bool SetCellularDataEnable(bool userDataEnabled);
    bool IsCellularDataEnabled() const;
    bool SetCellularDataRoamingEnabled(bool dataRoamingEnabled);
    ApnProfileState GetCellularDataState() const;
    ApnProfileState GetCellularDataState(const std::string &apnType) const;
    bool IsCellularDataRoamingEnabled() const;
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
    void UnRegisterDataObserver();

private:
    std::shared_ptr<CellularDataHandler> cellularDataHandler_;
    sptr<CellularDataSettingObserver> settingObserver_;
    sptr<CellularDataRoamingObserver> roamingObserver_;
    sptr<CellularDataRdbObserver> cellularDataRdbObserver_;
    const int32_t slotId_;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_CONTROLLER_H