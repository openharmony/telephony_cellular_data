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

#include <memory>
#include <string>

#include "event_handler.h"
#include "inner_event.h"
#include "radio_event.h"

#include "cellular_data_event_code.h"
#include "cellular_data_state_machine.h"

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
    bool ProcessDataConnectionRoamOn(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionRoamOff(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionVoiceCallStartedOrEnded(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessGetBandwidthsFromRil(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessNrStateChanged(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionComplete(const AppExecFwk::InnerEvent::Pointer &event);
    void RefreshConnectionBandwidths();
    void RefreshTcpBufferSizes();

private:
    using Fun = bool (Active::*)(const AppExecFwk::InnerEvent::Pointer &data);
    std::map<uint32_t, Fun> eventIdFunMap_ {
        {CellularDataEventCode::MSG_SM_CONNECT, &Active::ProcessConnectDone},
        {CellularDataEventCode::MSG_SM_DISCONNECT, &Active::ProcessDisconnectDone},
        {CellularDataEventCode::MSG_SM_DISCONNECT_ALL, &Active::ProcessDisconnectAllDone},
        {CellularDataEventCode::MSG_SM_LOST_CONNECTION, &Active::ProcessLostConnection},
        {CellularDataEventCode::MSG_SM_DATA_ROAM_ON, &Active::ProcessDataConnectionRoamOn},
        {CellularDataEventCode::MSG_SM_DATA_ROAM_OFF, &Active::ProcessDataConnectionRoamOff},
        {CellularDataEventCode::MSG_SM_VOICE_CALL_STARTED, &Active::ProcessDataConnectionVoiceCallStartedOrEnded},
        {CellularDataEventCode::MSG_SM_VOICE_CALL_ENDED, &Active::ProcessDataConnectionVoiceCallStartedOrEnded},
        {RadioEvent::RADIO_NR_STATE_CHANGED, &Active::ProcessNrStateChanged},
        {RadioEvent::RADIO_NR_FREQUENCY_CHANGED, &Active::ProcessNrFrequencyChanged},
        {CellularDataEventCode::MSG_GET_RIL_BANDWIDTH, &Active::ProcessGetBandwidthsFromRil},
        {RadioEvent::RADIO_RIL_SETUP_DATA_CALL, &Active::ProcessDataConnectionComplete},
    };
    std::weak_ptr<CellularDataStateMachine> stateMachine_;
};
} // namespace Telephony
} // namespace OHOS
#endif // ACTIVE_H
