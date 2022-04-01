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

#ifndef DEFAULT_H
#define DEFAULT_H

#include <memory>
#include <string>

#include "event_handler.h"
#include "inner_event.h"

#include "cellular_data_event_code.h"
#include "cellular_data_state_machine.h"

namespace OHOS {
namespace Telephony {
class Default : public State {
public:
    Default(std::weak_ptr<CellularDataStateMachine> &&cellularService, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(cellularService))
    {}
    virtual ~Default() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    bool ProcessConnectDone(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDisconnectDone(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDisconnectAllDone(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionDrsOrRatChanged(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionRoamOn(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDataConnectionRoamOff(const AppExecFwk::InnerEvent::Pointer &event);

private:
    using Fun = bool (Default::*)(const AppExecFwk::InnerEvent::Pointer &data);
    std::map<uint32_t, Fun> eventIdFunMap_ {
        {CellularDataEventCode::MSG_SM_CONNECT, &Default::ProcessConnectDone},
        {CellularDataEventCode::MSG_SM_DISCONNECT, &Default::ProcessDisconnectDone},
        {CellularDataEventCode::MSG_SM_DISCONNECT_ALL, &Default::ProcessDisconnectAllDone},
        {CellularDataEventCode::MSG_SM_DRS_OR_RAT_CHANGED, &Default::ProcessDataConnectionDrsOrRatChanged},
        {CellularDataEventCode::MSG_SM_DATA_ROAM_ON, &Default::ProcessDataConnectionRoamOn},
        {CellularDataEventCode::MSG_SM_DATA_ROAM_OFF, &Default::ProcessDataConnectionRoamOff},
    };
    std::weak_ptr<CellularDataStateMachine> stateMachine_;
};
} // namespace Telephony
} // namespace OHOS
#endif // DEFAULT_H
