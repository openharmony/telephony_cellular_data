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

#include "cellular_data_manager_stub.h"
#include "cellular_data_constant.h"
#include "cellular_data_controller.h"
#include "traffic_management.h"
#include "apn_activate_report_info.h"
#include "apn_attribute.h"
#include "apn_item.h"

namespace OHOS {
namespace Telephony {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING,
};

class CellularDataService : public SystemAbility, public CellularDataManagerStub {
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
    int32_t GetCellularDataState(int32_t &state) override;
    int32_t IsCellularDataRoamingEnabled(const int32_t slotId, bool &dataRoamingEnabled) override;
    int32_t EnableCellularDataRoaming(const int32_t slotId, bool enable) override;
    int32_t HandleApnChanged(const int32_t slotId) override;
    int32_t GetDefaultCellularDataSlotId(int32_t &slotId) override;
    int32_t GetDefaultCellularDataSimId(int32_t &simId) override;
    int32_t SetDefaultCellularDataSlotId(const int32_t slotId) override;
    int32_t GetCellularDataFlowType(int32_t &type) override;
    void DispatchEvent(int32_t slotId, const AppExecFwk::InnerEvent::Pointer &event);
    int32_t HasInternetCapability(const int32_t slotId, const int32_t cid, int32_t &capability) override;
    int32_t ClearCellularDataConnections(const int32_t slotId) override;
    int32_t ClearAllConnections(const int32_t slotId, const int32_t reason) override;
    int32_t ChangeConnectionForDsds(const int32_t slotId, bool enable);
    int32_t StrategySwitch(int32_t slotId, bool enable);
    int32_t RequestNet(const NetRequest &request);
    int32_t ReleaseNet(const NetRequest &request);
    int32_t AddUid(const NetRequest &request);
    int32_t RemoveUid(const NetRequest &request);
    int32_t GetServiceRunningState();
    int64_t GetSpendTime();
    int32_t GetApnState(int32_t slotId, const std::string &apnType, int &state) override;
    int32_t GetDataRecoveryState(int32_t &state) override;
    int32_t RegisterSimAccountCallback(const sptr<SimAccountCallback> &callback) override;
    int32_t UnregisterSimAccountCallback(const sptr<SimAccountCallback> &callback) override;
    int32_t GetDataConnApnAttr(int32_t slotId, ApnAttribute &apnAttr) override;
    int32_t GetDataConnIpType(int32_t slotId, std::string &ipType) override;
    int32_t IsNeedDoRecovery(int32_t slotId, bool needDoRecovery) override;
    int32_t EnableIntelligenceSwitch(bool enable) override;
    int32_t InitCellularDataController(int32_t slotId) override;
    int32_t GetIntelligenceSwitchState(bool &switchState) override;
    int32_t EstablishAllApnsIfConnectable(int32_t slotId) override;
    int32_t ReleaseCellularDataConnection(int32_t slotId) override;
    int32_t GetCellularDataSupplierId(int32_t slotId, uint64_t capability, uint32_t &supplierId) override;
    int32_t CorrectNetSupplierNoAvailable(int32_t slotId) override;
    int32_t GetSupplierRegisterState(uint32_t supplierId, int32_t &regState) override;
    int32_t GetIfSupportDunApn(bool &isSupportDun) override;
    int32_t GetDefaultActReportInfo(int32_t slotId, ApnActivateReportInfoIpc &info) override;
    int32_t GetInternalActReportInfo(int32_t slotId, ApnActivateReportInfoIpc &info) override;
    int32_t QueryApnIds(const ApnInfo& apnInfo, std::vector<uint32_t> &apnIdList) override;
    int32_t SetPreferApn(int32_t apnId) override;
    int32_t QueryAllApnInfo(std::vector<ApnInfo> &apnInfoList) override;
    int32_t SendUrspDecodeResult(int32_t slotId, const std::vector<uint8_t>& buffer) override;
    int32_t SendUePolicySectionIdentifier(int32_t slotId, const std::vector<uint8_t>& buffer) override;
    int32_t SendImsRsdList(int32_t slotId, const std::vector<uint8_t>& buffer) override;
    int32_t GetNetworkSliceAllowedNssai(int32_t slotId, const std::vector<uint8_t>& buffer) override;
    int32_t GetNetworkSliceEhplmn(int32_t slotId) override;
    int32_t GetActiveApnName(std::string &apnName) override;

private:
    bool Init();
    void InitModule();
    void UnRegisterAllNetSpecifier();
    void AddNetSupplier(int32_t slotId, CellularDataNetAgent &netAgent, std::vector<uint64_t> &netCapabilities);
    void ClearCellularDataControllers();
    void AddCellularDataControllers(int32_t slotId, std::shared_ptr<CellularDataController> cellularDataController);
    std::shared_ptr<CellularDataController> GetCellularDataController(int32_t slotId);

private:
    std::map<int32_t, std::shared_ptr<CellularDataController>> cellularDataControllers_;
    bool registerToService_;
    int64_t beginTime_ = 0L;
    int64_t endTime_ = 0L;
    ServiceRunningState state_;
    std::mutex mapLock_;
    bool isInitSuccess_ = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_SERVICE_H
