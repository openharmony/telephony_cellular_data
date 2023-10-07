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

#include "incall_data_state_machine.h"

#include "cellular_data_settings_rdb_helper.h"
#include "core_manager_inner.h"
#include "telephony_log_wrapper.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
void IncallDataStateMachine::UpdateCallState(int32_t state)
{
    callState_ = state;
}

int32_t IncallDataStateMachine::GetCallState() const
{
    return callState_;
}

bool IncallDataStateMachine::HasAnyConnectedState() const
{
    if (apnManager_ != nullptr) {
        return apnManager_->HasAnyConnectedState();
    }
    return false;
}

int32_t IncallDataStateMachine::GetSlotId() const
{
    return slotId_;
}

bool IncallDataStateMachine::IsInCallDataSwitchOn()
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        return false;
    }
    Uri uri(CELLULAR_DATA_SETTING_DATA_INCALL_URI);
    int value = static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
    if (settingHelper->GetValue(uri, CELLULAR_DATA_COLUMN_INCALL, value) != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetValue failed!");
        return false;
    }
    TELEPHONY_LOGI("Slot%{public}d: value=%{public}d", slotId_, value);
    return value == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED);
}

bool IncallDataStateMachine::IsSlaveCanActiveData()
{
    int32_t dsdsMode = DSDS_MODE_V2;
    CoreManagerInner::GetInstance().GetDsdsMode(dsdsMode);
    if (dsdsMode >= DSDS_MODE_V3) {
        TELEPHONY_LOGI("Slot%{public}d: not dsds 2.0", slotId_);
        return false;
    }
    int32_t primarySlotId = INVALID_SLOT_ID;
    CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
    if (primarySlotId == INVALID_SLOT_ID || primarySlotId == slotId_) {
        TELEPHONY_LOGI("Slot%{public}d: not secondary sim card", slotId_);
        return false;
    }
    bool hasPrimarySimCard = false;
    CoreManagerInner::GetInstance().HasSimCard(primarySlotId, hasPrimarySimCard);
    if (!hasPrimarySimCard) {
        TELEPHONY_LOGI("Slot%{public}d: no primary sim card", slotId_);
        return false;
    }
    ImsRegInfo voiceInfo;
    CoreManagerInner::GetInstance().GetImsRegStatus(slotId_, ImsServiceType::TYPE_VOICE, voiceInfo);
    ImsRegInfo videoInfo;
    CoreManagerInner::GetInstance().GetImsRegStatus(slotId_, ImsServiceType::TYPE_VIDEO, videoInfo);
    if (voiceInfo.imsRegState != ImsRegState::IMS_REGISTERED && videoInfo.imsRegState != ImsRegState::IMS_REGISTERED) {
        TELEPHONY_LOGI("Slot%{public}d: not ims call", slotId_);
        return false;
    }
    if (callState_ == static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE) ||
        callState_ == static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED)) {
        TELEPHONY_LOGI("Slot%{public}d: not in call", slotId_);
        return false;
    }
    return CanActiveDataByRadioTech();
}

bool IncallDataStateMachine::CanActiveDataByRadioTech()
{
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, radioTech);
    switch (static_cast<RadioTech>(radioTech)) {
        case RadioTech::RADIO_TECHNOLOGY_WCDMA:
            // fall_through
        case RadioTech::RADIO_TECHNOLOGY_HSPA:
            // fall_through
        case RadioTech::RADIO_TECHNOLOGY_HSPAP:
            // fall_through
        case RadioTech::RADIO_TECHNOLOGY_LTE:
            // fall_through
        case RadioTech::RADIO_TECHNOLOGY_LTE_CA:
            // fall_through
        case RadioTech::RADIO_TECHNOLOGY_NR:
            return true;
        default:
            return false;
    }
}

void IncallDataStateMachine::Init(int32_t callState)
{
    idleState_ = new (std::nothrow) IdleState(std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "IdleState");
    slaveActiveState_ = new (std::nothrow)
        SlaveActiveState(std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "SlaveActiveState");
    activatingSlaveState_ = new (std::nothrow)
        ActivatingSlaveState(std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "ActivatingSlaveState");
    activatedSlaveState_ = new (std::nothrow)
        ActivatedSlaveState(std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "ActivatedSlaveState");
    deactivatingSlaveState_ = new (std::nothrow)
        DeactivatingSlaveState(std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "DeactivatingSlaveState");
    if (idleState_ == nullptr || slaveActiveState_ == nullptr || activatingSlaveState_ == nullptr ||
        activatedSlaveState_ == nullptr || deactivatingSlaveState_ == nullptr) {
        TELEPHONY_LOGE("memory allocation failed");
        return;
    }
    slaveActiveState_->SetParentState(idleState_);
    activatingSlaveState_->SetParentState(slaveActiveState_);
    activatedSlaveState_->SetParentState(slaveActiveState_);
    deactivatingSlaveState_->SetParentState(idleState_);
    callState_ = callState;
    StateMachine::SetOriginalState(idleState_);
    StateMachine::Start();
}

void IncallDataStateMachine::SetCurrentState(const sptr<State> &&state)
{
    currentState_ = std::move(state);
}

sptr<State> IncallDataStateMachine::GetCurrentState() const
{
    return currentState_;
}

void IdleState::StateBegin()
{
    TELEPHONY_LOGI("Enter Idle State");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(sptr<State>(this));
    if (stateMachine->GetCallState() == static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE) ||
        stateMachine->GetCallState() == static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED)) {
        std::shared_ptr<AppExecFwk::EventHandler> eventHandler = stateMachine->cellularDataHandler_.lock();
        if (eventHandler != nullptr) {
            eventHandler->SendEvent(CellularDataEventCode::MSG_INCALL_DATA_COMPLETE);
        }
    }
}

void IdleState::StateEnd()
{
    TELEPHONY_LOGI("Exit Idle State");
    isActive_ = false;
}

bool IdleState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return NOT_PROCESSED;
    }
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    uint32_t eventCode = event->GetInnerEventId();
    std::map<uint32_t, Fun>::iterator it = eventIdFunMap_.find(eventCode);
    if (it != eventIdFunMap_.end()) {
        return (this->*(it->second))(event);
    }
    return NOT_PROCESSED;
}

bool IdleState::ProcessCallStarted(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("IdleState::MSG_SM_INCALL_DATA_CALL_STARTED");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    if (stateMachine->IsInCallDataSwitchOn() && stateMachine->IsSlaveCanActiveData()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        if (defaultSlotId != stateMachine->GetSlotId()) {
            stateMachine->TransitionTo(stateMachine->activatingSlaveState_);
            CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(stateMachine->GetSlotId());
        }
    }
    return PROCESSED;
}

bool IdleState::ProcessCallEnded(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("IdleState::MSG_SM_INCALL_DATA_CALL_ENDED");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    if (stateMachine->stateMachineEventHandler_ == nullptr) {
        TELEPHONY_LOGE("stateMachineEventHandler_ is null");
        return NOT_PROCESSED;
    }
    if (stateMachine->stateMachineEventHandler_->HasInnerEvent(
            CellularDataEventCode::MSG_SM_INCALL_DATA_DSDS_CHANGED)) {
        stateMachine->stateMachineEventHandler_->RemoveEvent(CellularDataEventCode::MSG_SM_INCALL_DATA_DSDS_CHANGED);
    }
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler = stateMachine->cellularDataHandler_.lock();
    if (eventHandler != nullptr) {
        eventHandler->SendEvent(CellularDataEventCode::MSG_INCALL_DATA_COMPLETE);
    }
    return PROCESSED;
}

bool IdleState::ProcessSettingsOn(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("IdleState::MSG_SM_INCALL_DATA_SETTINGS_ON");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    if (stateMachine->IsInCallDataSwitchOn() && stateMachine->IsSlaveCanActiveData()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        if (defaultSlotId != stateMachine->GetSlotId()) {
            stateMachine->TransitionTo(stateMachine->activatingSlaveState_);
            CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(stateMachine->GetSlotId());
        }
    }
    return PROCESSED;
}

bool IdleState::ProcessDsdsChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("IdleState::MSG_SM_INCALL_DATA_DSDS_CHANGED");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    if (stateMachine->IsInCallDataSwitchOn() && stateMachine->IsSlaveCanActiveData()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        if (defaultSlotId != stateMachine->GetSlotId()) {
            stateMachine->TransitionTo(stateMachine->activatingSlaveState_);
            CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(stateMachine->GetSlotId());
        }
    }
    return PROCESSED;
}

void SlaveActiveState::StateBegin()
{
    TELEPHONY_LOGI("Enter SlaveActive State");
    isActive_ = true;
}

void SlaveActiveState::StateEnd()
{
    TELEPHONY_LOGI("Exit SlaveActive State");
    isActive_ = false;
}

bool SlaveActiveState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return NOT_PROCESSED;
    }
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    uint32_t eventCode = event->GetInnerEventId();
    std::map<uint32_t, Fun>::iterator it = eventIdFunMap_.find(eventCode);
    if (it != eventIdFunMap_.end()) {
        return (this->*(it->second))(event);
    }
    return NOT_PROCESSED;
}

bool SlaveActiveState::ProcessSettingsOn(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SlaveActiveState::MSG_SM_INCALL_DATA_SETTINGS_ON");
    return PROCESSED;
}

bool SlaveActiveState::ProcessCallEnded(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SlaveActiveState::MSG_SM_INCALL_DATA_CALL_ENDED");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    if (stateMachine->GetCallState() == static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE) ||
        stateMachine->GetCallState() == static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED)) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        int32_t primarySlotId = INVALID_SLOT_ID;
        CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
        if (defaultSlotId != primarySlotId) {
            stateMachine->TransitionTo(stateMachine->deactivatingSlaveState_);
        } else {
            stateMachine->TransitionTo(stateMachine->idleState_);
        }
    }
    return PROCESSED;
}

bool SlaveActiveState::ProcessSettingsOff(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SlaveActiveState::MSG_SM_INCALL_DATA_SETTINGS_OFF");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    if (!stateMachine->IsInCallDataSwitchOn()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        int32_t primarySlotId = INVALID_SLOT_ID;
        CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
        if (defaultSlotId != primarySlotId) {
            stateMachine->TransitionTo(stateMachine->deactivatingSlaveState_);
        } else {
            stateMachine->TransitionTo(stateMachine->idleState_);
        }
    }
    return PROCESSED;
}

bool SlaveActiveState::ProcessDsdsChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SlaveActiveState::MSG_SM_INCALL_DATA_DSDS_CHANGED");
    return PROCESSED;
}

void ActivatingSlaveState::StateBegin()
{
    TELEPHONY_LOGI("Enter ActivatingSlave State");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(sptr<State>(this));
}

void ActivatingSlaveState::StateEnd()
{
    TELEPHONY_LOGI("Exit ActivatingSlave State");
    isActive_ = false;
}

bool ActivatingSlaveState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return NOT_PROCESSED;
    }
    uint32_t eventCode = event->GetInnerEventId();
    if (eventCode == CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED) {
        std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
        if (stateMachine == nullptr) {
            TELEPHONY_LOGE("stateMachine is null");
            return NOT_PROCESSED;
        }
        stateMachine->TransitionTo(stateMachine->activatedSlaveState_);
        return PROCESSED;
    }
    return NOT_PROCESSED;
}

void ActivatedSlaveState::StateBegin()
{
    TELEPHONY_LOGI("Enter ActivatedSlave State");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(sptr<State>(this));
}

void ActivatedSlaveState::StateEnd()
{
    TELEPHONY_LOGI("Exit ActivatedSlave State");
    isActive_ = false;
}

bool ActivatedSlaveState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    return NOT_PROCESSED;
}

void DeactivatingSlaveState::StateBegin()
{
    TELEPHONY_LOGI("Enter DeactivatingSlave State");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(sptr<State>(this));
    int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    int32_t primarySlotId = INVALID_SLOT_ID;
    CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
    if (defaultSlotId != primarySlotId) {
        CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(primarySlotId);
    } else {
        stateMachine->TransitionTo(stateMachine->idleState_);
    }
    if (!stateMachine->HasAnyConnectedState()) {
        stateMachine->TransitionTo(stateMachine->idleState_);
    }
}

void DeactivatingSlaveState::StateEnd()
{
    TELEPHONY_LOGI("Exit DeactivatingSlave State");
    isActive_ = false;
}

bool DeactivatingSlaveState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return NOT_PROCESSED;
    }
    uint32_t eventCode = event->GetInnerEventId();
    if (eventCode == CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED) {
        std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
        if (stateMachine == nullptr) {
            TELEPHONY_LOGE("stateMachine is null");
            return NOT_PROCESSED;
        }
        int32_t primarySlotId = INVALID_SLOT_ID;
        CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
        if (stateMachine->GetSlotId() != primarySlotId) {
            stateMachine->TransitionTo(stateMachine->idleState_);
        }
        return PROCESSED;
    }
    if (eventCode == CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_ON) {
        return PROCESSED;
    }
    return NOT_PROCESSED;
}
} // namespace Telephony
} // namespace OHOS
