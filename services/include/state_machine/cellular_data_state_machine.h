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

#ifndef CELLULAR_DATA_STATE_MACHINE_H
#define CELLULAR_DATA_STATE_MACHINE_H

#include <memory>
#include <map>

#include "event_handler.h"
#include "inner_event.h"

#include "network_state.h"

#include "apn_item.h"
#include "cellular_data_net_agent.h"
#include "data_connection_manager.h"
#include "data_connection_params.h"
#include "data_disconnect_params.h"
#include "state_machine.h"

namespace OHOS {
namespace Telephony {
class DataConnectionManager;
class CellularDataStateMachine : public StateMachine,
    public std::enable_shared_from_this<CellularDataStateMachine> {
public:
    CellularDataStateMachine(sptr<DataConnectionManager> &cdConnectionManager,
        std::shared_ptr<AppExecFwk::EventHandler> &&cellularDataHandler,
        const std::shared_ptr<AppExecFwk::EventRunner> &runner)
        : StateMachine(runner), cdConnectionManager_(cdConnectionManager),
        cellularDataHandler_(std::move(cellularDataHandler)), cid_(0), capability_(0),
        rilRat_(RadioTech::RADIO_TECHNOLOGY_UNKNOWN), apnId_(0)
    {}
    ~CellularDataStateMachine() = default;
    bool operator==(const CellularDataStateMachine &stateMachine) const;
    bool IsInactiveState() const;
    sptr<State> GetCurrentState() const;
    void SetCapability(uint64_t capability);
    uint64_t GetCapability() const;
    int32_t GetCid() const;
    int32_t GetSlotId() const;
    sptr<ApnItem> GetApnItem() const;
    void Init();
    void UpdateNetworkInfo(const SetupDataCallResultInfo &dataCallInfo);
    void UpdateNetworkInfo();
    void SetConnectionBandwidth(const uint32_t upBandwidth, const uint32_t downBandwidth);
    void SetConnectionTcpBuffer(const std::string &tcpBuffer);

protected:
    sptr<State> activeState_;
    sptr<State> inActiveState_;
    sptr<State> activatingState_;
    sptr<State> disconnectingState_;
    sptr<State> defaultState_;
    sptr<State> currentState_;
    sptr<DataConnectionManager> cdConnectionManager_;
    std::shared_ptr<AppExecFwk::EventHandler> cellularDataHandler_;
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo_;
    sptr<NetManagerStandard::NetSupplierInfo> netSupplierInfo_;

private:
    void SetCurrentState(const sptr<State> &&state);
    void SetCid(const int32_t cid);
    void DoConnect(const DataConnectionParams &connectionParams);
    void FreeConnection(const DataDisconnectParams &params);
    void ResolveIp(std::vector<AddressInfo> &ipInfoArray);
    void ResolveDns(std::vector<AddressInfo> &dnsInfoArray);
    void ResolveRoute(std::vector<AddressInfo> &routeInfoArray, const std::string &name);

private:
    friend class Active;
    friend class Activating;
    friend class Inactive;
    friend class Default;
    friend class Disconnecting;
    int32_t cid_;
    uint64_t capability_;
    RadioTech rilRat_;
    sptr<ApnItem> apnItem_;
    int32_t apnId_;
    std::mutex mtx_;
    uint32_t upBandwidth_ = 0;
    uint32_t downBandwidth_ = 0;
    std::string tcpBuffer_;
    int32_t connectId_ = 0;
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_STATE_MACHINE_H