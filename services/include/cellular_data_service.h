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

#ifndef CELLULAR_DATA_SERVICE_H
#define CELLULAR_DATA_SERVICE_H

#include "iremote_object.h"
#include "nocopyable.h"
#include "singleton.h"
#include "system_ability.h"

#include "cellular_data_service_stub.h"
#include "cellular_data_constant.h"
#include "cellular_data_controller.h"
#include "traffic_management.h"

namespace OHOS {
namespace Telephony {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING,
};

class CellularDataService : public SystemAbility, public CellularDataServiceStub {
    DECLARE_DELAYED_REF_SINGLETON(CellularDataService)
    DECLARE_SYSTEM_ABILITY(CellularDataService)

public:
    /**
     * service Start
     */
    void OnStart() override;
    /**
     * service OnStop
     */
    void OnStop() override;
    int32_t Dump(std::int32_t fd, const std::vector<std::u16string>& args) override;
    std::string GetBeginTime();
    std::string GetEndTime();
    std::string GetCellularDataSlotIdDump();
    std::string GetStateMachineCurrentStatusDump();
    std::string GetFlowDataInfoDump();
    int32_t IsCellularDataEnabled(bool &dataEnabled) override;
    int32_t EnableCellularData(bool enable) override;
    int32_t GetCellularDataState() override;
    int32_t IsCellularDataRoamingEnabled(const int32_t slotId, bool &dataRoamingEnabled) override;
    int32_t EnableCellularDataRoaming(const int32_t slotId, bool enable) override;
    int32_t HandleApnChanged(const int32_t slotId) override;
    int32_t GetDefaultCellularDataSlotId() override;
    int32_t SetDefaultCellularDataSlotId(const int32_t slotId) override;
    int32_t GetCellularDataFlowType() override;
    void DispatchEvent(int32_t slotId, const AppExecFwk::InnerEvent::Pointer &event);
    int32_t HasInternetCapability(const int32_t slotId, const int32_t cid) override;
    int32_t ClearCellularDataConnections(const int32_t slotId) override;
    int32_t ClearAllConnections(const int32_t slotId, DisConnectionReason reason);
    int32_t StrategySwitch(int32_t slotId, bool enable);
    int32_t RequestNet(const NetRequest &request);
    int32_t ReleaseNet(const NetRequest &request);
    int32_t GetServiceRunningState();
    int64_t GetSpendTime();
    int32_t RegisterSimAccountCallback(const sptr<SimAccountCallback> &callback) override;
    int32_t UnregisterSimAccountCallback() override;

private:
    bool Init();
    void InitModule();
    bool CheckParamValid(const int32_t slotId);
    void UnRegisterAllNetSpecifier();

private:
    std::map<int32_t, std::shared_ptr<CellularDataController>> cellularDataControllers_;
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    bool registerToService_;
    int64_t beginTime_ = 0L;
    int64_t endTime_ = 0L;
    ServiceRunningState state_;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_SERVICE_H
