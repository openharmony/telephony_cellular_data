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

#ifndef ACTIVE_H
#define ACTIVE_H

#include <string>

#include "cellular_data_state_machine.h"
#include "radio_event.h"

namespace OHOS {
namespace Telephony {
class Active : public State {
public:
    Active(std::weak_ptr<CellularDataStateMachine> &&cellularService, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(cellularService))
    {}
    virtual ~Active() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    bool ProcessConnectDone(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDisconnectDone(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDisconnectAllDone(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessLostConnection(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessRilAdapterHostDied(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessLinkCapabilityChanged(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionRoamOn(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionRoamOff(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionVoiceCallStartedOrEnded(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessNrStateChanged(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionComplete(const AppExecFwk::InnerEvent::Pointer &event);
    void RefreshConnectionBandwidths();
    void RefreshTcpBufferSizes();

private:
    using Fun = std::function<bool(const AppExecFwk::InnerEvent::Pointer &data)>;
    std::map<uint32_t, Fun> eventIdFunMap_ {
        { CellularDataEventCode::MSG_SM_CONNECT,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessConnectDone(data); } },
        { CellularDataEventCode::MSG_SM_DISCONNECT,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessDisconnectDone(data); } },
        { CellularDataEventCode::MSG_SM_DISCONNECT_ALL,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessDisconnectAllDone(data); } },
        { CellularDataEventCode::MSG_SM_LOST_CONNECTION,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessLostConnection(data); } },
        { CellularDataEventCode::MSG_SM_LINK_CAPABILITY_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessLinkCapabilityChanged(data); } },
        { CellularDataEventCode::MSG_SM_DATA_ROAM_ON,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessDataConnectionRoamOn(data); } },
        { CellularDataEventCode::MSG_SM_DATA_ROAM_OFF,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessDataConnectionRoamOff(data); } },
        { CellularDataEventCode::MSG_SM_VOICE_CALL_STARTED,
            [this](const AppExecFwk::InnerEvent::Pointer &data) {
                return ProcessDataConnectionVoiceCallStartedOrEnded(data);
            } },
        { CellularDataEventCode::MSG_SM_VOICE_CALL_ENDED,
            [this](const AppExecFwk::InnerEvent::Pointer &data) {
                return ProcessDataConnectionVoiceCallStartedOrEnded(data);
            } },
        { CellularDataEventCode::MSG_SM_RIL_ADAPTER_HOST_DIED,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessRilAdapterHostDied(data); } },
        { RadioEvent::RADIO_NR_STATE_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessNrStateChanged(data); } },
        { RadioEvent::RADIO_NR_FREQUENCY_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessNrFrequencyChanged(data); } },
        { RadioEvent::RADIO_RIL_SETUP_DATA_CALL,
            [this](const AppExecFwk::InnerEvent::Pointer &data) { return ProcessDataConnectionComplete(data); } },
    };
    inline static std::map<DisConnectionReason, PdpErrorReason> disconnReasonPdpErrorMap_ {
        { DisConnectionReason::REASON_NORMAL, PdpErrorReason::PDP_ERR_TO_NORMAL },
        { DisConnectionReason::REASON_GSM_AND_CALLING_ONLY, PdpErrorReason::PDP_ERR_TO_GSM_AND_CALLING_ONLY },
        { DisConnectionReason::REASON_RETRY_CONNECTION, PdpErrorReason::PDP_ERR_RETRY },
        { DisConnectionReason::REASON_CLEAR_CONNECTION, PdpErrorReason::PDP_ERR_TO_CLEAR_CONNECTION },
        { DisConnectionReason::REASON_CHANGE_CONNECTION, PdpErrorReason::PDP_ERR_TO_CHANGE_CONNECTION },
        { DisConnectionReason::REASON_PERMANENT_REJECT, PdpErrorReason::PDP_ERR_TO_PERMANENT_REJECT },
    };
    std::weak_ptr<CellularDataStateMachine> stateMachine_;
};
} // namespace Telephony
} // namespace OHOS
#endif // ACTIVE_H
