/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef INCALL_DATA_STATE_MACHINE_H
#define INCALL_DATA_STATE_MACHINE_H

#include <map>
#include <memory>

#include "apn_manager.h"
#include "cellular_data_constant.h"
#include "event_handler.h"
#include "inner_event.h"
#include "refbase.h"
#include "state_machine.h"

namespace OHOS {
namespace Telephony {
class IncallDataStateMachine : public StateMachine, public std::enable_shared_from_this<IncallDataStateMachine> {
public:
    IncallDataStateMachine(int32_t slotId, std::weak_ptr<AppExecFwk::EventHandler> &&cellularDataHandler,
        const std::shared_ptr<AppExecFwk::EventRunner> &runner, sptr<ApnManager> &apnManager)
        : StateMachine(runner), cellularDataHandler_(std::move(cellularDataHandler)), apnManager_(apnManager),
          slotId_(slotId)
    {}
    ~IncallDataStateMachine() = default;
    sptr<State> GetCurrentState() const;
    int32_t GetSlotId() const;
    int32_t GetCallState() const;
    bool HasAnyConnectedState() const;
    void UpdateCallState(int32_t state);
    void Init(int32_t callState);

protected:
    sptr<State> idleState_;
    sptr<State> slaveActiveState_;
    sptr<State> activatingSlaveState_;
    sptr<State> activatedSlaveState_;
    sptr<State> deactivatingSlaveState_;
    sptr<State> currentState_;
    std::weak_ptr<AppExecFwk::EventHandler> cellularDataHandler_;
    sptr<ApnManager> apnManager_;

private:
    void SetCurrentState(const sptr<State> &&state);
    bool IsInCallDataSwitchOn();
    bool IsSlaveCanActiveData();
    bool CanActiveDataByRadioTech();

private:
    friend class IdleState;
    friend class SlaveActiveState;
    friend class ActivatingSlaveState;
    friend class ActivatedSlaveState;
    friend class DeactivatingSlaveState;
    int32_t slotId_ = INVALID_SLOT_ID;
    int32_t callState_ = static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE);
};

class IdleState : public State {
public:
    IdleState(std::weak_ptr<IncallDataStateMachine> &&inCallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(inCallData))
    {}
    virtual ~IdleState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    bool ProcessCallStarted(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessCallEnded(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessSettingsOn(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDsdsChanged(const AppExecFwk::InnerEvent::Pointer &event);

private:
    using Fun = bool (IdleState::*)(const AppExecFwk::InnerEvent::Pointer &data);
    std::map<uint32_t, Fun> eventIdFunMap_ {
        { CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_STARTED, &IdleState::ProcessCallStarted },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_ENDED, &IdleState::ProcessCallEnded },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_ON, &IdleState::ProcessSettingsOn },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_DSDS_CHANGED, &IdleState::ProcessDsdsChanged },
    };
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};

class SlaveActiveState : public State {
public:
    SlaveActiveState(std::weak_ptr<IncallDataStateMachine> &&inCallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(inCallData))
    {}
    virtual ~SlaveActiveState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    bool ProcessSettingsOn(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessCallEnded(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessSettingsOff(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDsdsChanged(const AppExecFwk::InnerEvent::Pointer &event);

private:
    using Fun = bool (SlaveActiveState::*)(const AppExecFwk::InnerEvent::Pointer &data);
    std::map<uint32_t, Fun> eventIdFunMap_ {
        { CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_ON, &SlaveActiveState::ProcessSettingsOn },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_ENDED, &SlaveActiveState::ProcessCallEnded },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_OFF, &SlaveActiveState::ProcessSettingsOff },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_DSDS_CHANGED, &SlaveActiveState::ProcessDsdsChanged },
    };
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};

class ActivatingSlaveState : public State {
public:
    ActivatingSlaveState(std::weak_ptr<IncallDataStateMachine> &&inCallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(inCallData))
    {}
    virtual ~ActivatingSlaveState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};

class ActivatedSlaveState : public State {
public:
    ActivatedSlaveState(std::weak_ptr<IncallDataStateMachine> &&inCallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(inCallData))
    {}
    virtual ~ActivatedSlaveState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};

class DeactivatingSlaveState : public State {
public:
    DeactivatingSlaveState(std::weak_ptr<IncallDataStateMachine> &&inCallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(inCallData))
    {}
    virtual ~DeactivatingSlaveState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};
} // namespace Telephony
} // namespace OHOS
#endif // INCALL_DATA_STATE_MACHINE_H