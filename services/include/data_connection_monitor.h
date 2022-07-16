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

#ifndef DATA_CONNECTION_MONITOR_H
#define DATA_CONNECTION_MONITOR_H

#include "event_handler.h"
#include "inner_event.h"

#include "apn_holder.h"
#include "traffic_management.h"

namespace OHOS {
namespace Telephony {
class DataConnectionMonitor : public AppExecFwk::EventHandler {
public:
    explicit DataConnectionMonitor(const std::shared_ptr<AppExecFwk::EventRunner> &runner, int32_t slotId);
    ~DataConnectionMonitor() = default;

    /**
     * Start the data detection
     */
    void StartStallDetectionTimer();

    /**
     * Data detection is in progress
     */
    void OnStallDetectionTimer();

    /**
     * Stop the data detection
     */
    void StopStallDetectionTimer();

    /**
     * Update flow info
     */
    void UpdateFlowInfo();

    /**
     * Data recovery processing
     */
    void HandleRecovery();

    /**
     * Start the statistical network
     */
    void BeginNetStatistics();

    /**
     * End the statistical network
     */
    void EndNetStatistics();

    /**
     * Update the up and down data status of the network
     */
    void UpdateNetTrafficState();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    void GetPdpContextList();

    /**
     * Set the radio state
     *
     * @param radioState RADIO_STATUS_OFF = 0 or RADIO_STATUS_ON = 1
     * @param eventCode distinguish between the identity of the event
     */
    void SetRadioState(const int32_t &radioState, const int32_t &eventCode);

    /**
     * Get the preferred network
     */
    void GetPreferredNetworkPara();

    /**
     * Update data flow type
     */
    void UpdateDataFlowType();

    CellDataFlowType GetDataFlowType();

    void SetDataFlowType(CellDataFlowType dataFlowType);

    /**
     * Set the preferred network
     *
     * @param event distinguish between the identity of the event
     */
    void SetPreferredNetworkPara(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::unique_ptr<TrafficManagement> trafficManager_;
    std::unique_ptr<TrafficManagement> stallDetectionTrafficManager_;
    bool updateNetStat_ = false;
    bool stallDetectionEnabled = false;
    int64_t noRecvPackets_ = 0;
    RecoveryState dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
    CellDataFlowType dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_NONE;
    const int32_t slotId_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DATA_CONNECTION_MONITOR_H