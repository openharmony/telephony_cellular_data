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
#include "inner_event.h"
#include "refbase.h"
#include "state_machine.h"
#include "tel_event_handler.h"

namespace OHOS {
namespace Telephony {
class IncallDataStateMachine : public StateMachine, public std::enable_shared_from_this<IncallDataStateMachine> {
public:
    IncallDataStateMachine(
        int32_t slotId, std::weak_ptr<TelEventHandler> &&cellularDataHandler, sptr<ApnManager> &apnManager)
        : StateMachine("IncallDataStateMachine"), cellularDataHandler_(std::move(cellularDataHandler)),
          apnManager_(apnManager), slotId_(slotId)
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
    sptr<State> secondaryActiveState_;
    sptr<State> activatingSecondaryState_;
    sptr<State> activatedSecondaryState_;
    sptr<State> deactivatingSecondaryState_;
    sptr<State> currentState_;
    std::weak_ptr<TelEventHandler> cellularDataHandler_;
    sptr<ApnManager> apnManager_;

private:
    void SetCurrentState(const sptr<State> &&state);
    bool IsIncallDataSwitchOn();
    bool IsSecondaryCanActiveData();
    bool CanActiveDataByRadioTech();

private:
    friend class IdleState;
    friend class SecondaryActiveState;
    friend class ActivatingSecondaryState;
    friend class ActivatedSecondaryState;
    friend class DeactivatingSecondaryState;
    int32_t slotId_ = INVALID_SLOT_ID;
    int32_t callState_ = static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE);
};

class IdleState : public State {
public:
    IdleState(std::weak_ptr<IncallDataStateMachine> &&incallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(incallData))
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

class SecondaryActiveState : public State {
public:
    SecondaryActiveState(std::weak_ptr<IncallDataStateMachine> &&incallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(incallData))
    {}
    virtual ~SecondaryActiveState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    bool ProcessSettingsOn(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessCallEnded(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessSettingsOff(const AppExecFwk::InnerEvent::Pointer &event);
    bool ProcessDsdsChanged(const AppExecFwk::InnerEvent::Pointer &event);

private:
    using Fun = bool (SecondaryActiveState::*)(const AppExecFwk::InnerEvent::Pointer &data);
    std::map<uint32_t, Fun> eventIdFunMap_ {
        { CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_ON, &SecondaryActiveState::ProcessSettingsOn },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_ENDED, &SecondaryActiveState::ProcessCallEnded },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_OFF, &SecondaryActiveState::ProcessSettingsOff },
        { CellularDataEventCode::MSG_SM_INCALL_DATA_DSDS_CHANGED, &SecondaryActiveState::ProcessDsdsChanged },
    };
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};

class ActivatingSecondaryState : public State {
public:
    ActivatingSecondaryState(std::weak_ptr<IncallDataStateMachine> &&incallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(incallData))
    {}
    virtual ~ActivatingSecondaryState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};

class ActivatedSecondaryState : public State {
public:
    ActivatedSecondaryState(std::weak_ptr<IncallDataStateMachine> &&incallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(incallData))
    {}
    virtual ~ActivatedSecondaryState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};

class DeactivatingSecondaryState : public State {
public:
    DeactivatingSecondaryState(std::weak_ptr<IncallDataStateMachine> &&incallData, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(incallData))
    {}
    virtual ~DeactivatingSecondaryState() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::weak_ptr<IncallDataStateMachine> stateMachine_;
};
} // namespace Telephony
} // namespace OHOS
#endif // INCALL_DATA_STATE_MACHINE_H