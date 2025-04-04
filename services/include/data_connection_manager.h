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

#ifndef DATA_CONNECTION_MANAGER_H
#define DATA_CONNECTION_MANAGER_H

#include <tel_ril_data_parcel.h>
#include <map>
#include <memory>

#include "data_connection_monitor.h"
#include "state_machine.h"

namespace OHOS {
namespace Telephony {
class CellularDataStateMachine;
class DataConnectionManager : public StateMachine, public RefBase {
public:
    explicit DataConnectionManager(int32_t slotId);
    ~DataConnectionManager();
    void Init();
    void AddConnectionStateMachine(const std::shared_ptr<CellularDataStateMachine> &stateMachine);
    void RemoveConnectionStateMachine(const std::shared_ptr<CellularDataStateMachine> &stateMachine);
    void AddActiveConnectionByCid(const std::shared_ptr<CellularDataStateMachine> &stateMachine);
    std::shared_ptr<CellularDataStateMachine> GetActiveConnectionByCid(int32_t cid);
    bool isNoActiveConnection();
    std::map<int32_t, std::shared_ptr<CellularDataStateMachine>> GetActiveConnection();
    bool IsBandwidthSourceModem() const;
    void RemoveActiveConnectionByCid(int32_t cid);
    void StartStallDetectionTimer();
    void StopStallDetectionTimer();
    void RegisterRadioObserver();
    void UnRegisterRadioObserver() const;
    void BeginNetStatistics();
    void EndNetStatistics();
    int32_t GetDataFlowType();
    void SetDataFlowType(CellDataFlowType dataFlowType);
    int32_t GetSlotId() const;
    std::vector<std::shared_ptr<CellularDataStateMachine>> GetAllConnectionMachine();
    void GetDefaultBandWidthsConfig();
    void GetDefaultTcpBufferConfig();
    LinkBandwidthInfo GetBandwidthsByRadioTech(const int32_t radioTech);
    std::string GetTcpBufferByRadioTech(const int32_t radioTech);
    void UpdateCallState(int32_t state);
    int32_t GetDataRecoveryState();
    void IsNeedDoRecovery(bool needDoRecovery) const;
    void HandleScreenStateChanged(bool isScreenOn) const;

private:
    void UpdateBandWidthsUseLte();

private:
    std::shared_ptr<DataConnectionMonitor> connectionMonitor_;
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines_;
    std::map<int32_t, std::shared_ptr<CellularDataStateMachine>> cidActiveConnectionMap_;
    std::mutex stateMachineMutex_;
    std::mutex activeConnectionMutex_;
    std::mutex tcpBufferConfigMutex_;
    std::mutex bandwidthConfigMutex_;
    sptr<State> ccmDefaultState_;
    const int32_t slotId_;
    std::map<std::string, LinkBandwidthInfo> bandwidthConfigMap_;
    std::map<std::string, std::string> tcpBufferConfigMap_;
    bool bandwidthSourceModem_ = true;
    bool uplinkUseLte_ = false;
};

class CcmDefaultState : public State {
public:
    CcmDefaultState(DataConnectionManager &connectManager, std::string &&name)
        : State(std::move(name)), connectManager_(connectManager)
    {}
    virtual ~CcmDefaultState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

protected:
    void RadioDataCallListChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void RadioLinkCapabilityChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void UpdateNetworkInfo(const AppExecFwk::InnerEvent::Pointer &event);
    void RadioNetworkSliceUrspRpt(const AppExecFwk::InnerEvent::Pointer &event);
    void RadioNetworkSliceAllowedNssaiRpt(const AppExecFwk::InnerEvent::Pointer &event);
    void RadioNetworkSliceEhplmnRpt(const AppExecFwk::InnerEvent::Pointer &event);

private:
    DataConnectionManager &connectManager_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_CONNECTION_MANAGER_H
