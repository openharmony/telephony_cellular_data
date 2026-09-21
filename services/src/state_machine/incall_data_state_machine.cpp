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

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "radio_event.h"
#include "sim_state_type.h"

#include <sstream>
#include <vector>

namespace OHOS {
namespace Telephony {

namespace {
struct BackupCardInfo {
    uint32_t slotId;
    uint32_t simLabelIndex;
    bool isEsim;
};

void ParseSingleBackupCardInfo(const std::string &cardInfoStr, std::vector<BackupCardInfo> &cardInfos)
{
    std::istringstream iss(cardInfoStr);
    std::string token;
    BackupCardInfo card = {};
    int32_t value = 0;
    if (!std::getline(iss, token, ',')) {
        return;
    }
    if (!CellularDataUtils::ConvertStrToInt(token, value) || value < 0) {
        return;
    }
    card.slotId = static_cast<uint32_t>(value);
    if (!std::getline(iss, token, ',')) {
        return;
    }
    if (!CellularDataUtils::ConvertStrToInt(token, value) || value < 0) {
        return;
    }
    card.simLabelIndex = static_cast<uint32_t>(value);
    if (!std::getline(iss, token, ',')) {
        return;
    }
    card.isEsim = (token == "1");
    cardInfos.push_back(card);
}

void ParseBackupCardInfos(const std::string &cardString, std::vector<BackupCardInfo> &cardInfos)
{
    size_t start = 0;
    size_t end = cardString.find(';');
    while (end != std::string::npos) {
        std::string cardInfoStr = cardString.substr(start, end - start);
        ParseSingleBackupCardInfo(cardInfoStr, cardInfos);
        start = end + 1;
        end = cardString.find(';', start);
    }
    if (start < cardString.size()) {
        std::string cardInfoStr = cardString.substr(start);
        ParseSingleBackupCardInfo(cardInfoStr, cardInfos);
    }
}

bool CheckIfBackupNetwork(int32_t slotId, const std::vector<BackupCardInfo> &cardInfos)
{
    SimLabel simLabel;
    CoreManagerInner::GetInstance().GetSimLabel(slotId, simLabel);
    for (const auto &cardInfo : cardInfos) {
        if (static_cast<int32_t>(cardInfo.simLabelIndex) == simLabel.index &&
            cardInfo.isEsim == (simLabel.simType == SimType::ESIM)) {
            return true;
        }
    }
    return false;
}
}

int32_t IncallDataStateMachine::GetTargetDataSlotId(int32_t defSlotId)
{
    // slotId=3 特殊处理（仅当 TSTS 模式启用时）
    if (slotId_ == CELLDATA_SLOT_ID_3 && CellularDataUtils::IsTstsModeEnabled()) {
        TELEPHONY_LOGI("Slot%{public}d: TSTS mode enabled, def slotId=%{public}d", slotId_, defSlotId);
        return defSlotId == CELLDATA_SLOT_ID_1 ? CELLDATA_SLOT_ID_0 : CELLDATA_SLOT_ID_1;
    }
    // 其他副卡，保持原有逻辑，激活自己的数据
    return slotId_;
}

void IncallDataStateMachine::SetPrimarySlot(int32_t targetSlotId)
{
    if (slotId_ == CELLDATA_SLOT_ID_3 && CellularDataUtils::IsTstsModeEnabled()) {
        TELEPHONY_LOGI("Slot%{public}d: TSTS mode, set primary slot to %{public}d", slotId_, targetSlotId);
        CoreManagerInner::GetInstance().SetPrimarySlot(targetSlotId, RADIO_SIM_SET_PRIMARY_SLOT, nullptr);
    }
}

bool IncallDataStateMachine::CheckBackupNetworkIfAllow(int32_t targetSlotId)
{
    std::string backupNetworkResult;
    Uri backupSimListUri(CELLULAR_DATA_SETTING_BACKUP_SIM_LIST_URI);
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr ||
        settingHelper->GetValue(
            backupSimListUri, BACKUP_SIM_LIST_COLUMN_ENABLE, backupNetworkResult) != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: backup sim list not configured", slotId_);
        return true;
    }
    std::vector<BackupCardInfo> cardInfos;
    ParseBackupCardInfos(backupNetworkResult, cardInfos);
    if (cardInfos.size() < static_cast<size_t>(VALID_MIN_BACKUP_NUM)) {
        TELEPHONY_LOGE("Slot%{public}d: invalid backup network num", slotId_);
        return true;
    }
    bool result = CheckIfBackupNetwork(targetSlotId, cardInfos);
    TELEPHONY_LOGI("Slot%{public}d: target slot %{public}d backup network if allow %{public}d",
        slotId_, targetSlotId, result);
    return result;
}

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

bool IncallDataStateMachine::IsIncallDataSwitchOn()
{
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("settingHelper null!");
        return false;
    }
    int32_t value = static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
    int32_t intelligenceNetworkValue = static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
    Uri intelligenceNetworkUri(CELLULAR_DATA_SETTING_INTELLIGENCE_NETWORK_URI);
    if (settingHelper->GetValue(
        intelligenceNetworkUri, INTELLIGENCE_NETWORK_COLUMN_ENABLE,
        intelligenceNetworkValue) != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetValue failed!");
        return false;
    }
    int32_t smartDualCardValue = static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
    Uri smartDualCardUri(CELLULAR_DATA_SETTING_INTELLIGENCE_SWITCH_URI);
    if (settingHelper->GetValue(
        smartDualCardUri, INTELLIGENCE_SWITCH_COLUMN_ENABLE, smartDualCardValue) != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetValue failed!");
        return false;
    }
    value = (intelligenceNetworkValue == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED) &&
        smartDualCardValue == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED)) ?
        static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED) :
        static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
    TELEPHONY_LOGI("Slot%{public}d: value=%{public}d", slotId_, value);
    return value == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED);
}

bool IncallDataStateMachine::IsSecondaryCanActiveData()
{
    int32_t dsdsModeValue = DSDS_MODE_V2;
    CoreManagerInner::GetInstance().GetDsdsMode(dsdsModeValue);
    int32_t primarySlotId = INVALID_SLOT_ID;
    CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
    int32_t dsdsMode = CellularDataUtils::GetDsdsModeForSlots(primarySlotId, slotId_, dsdsModeValue);
    if (dsdsMode >= DSDS_MODE_V3) {
        TELEPHONY_LOGI("Slot%{public}d: not dsds 2.0", slotId_);
        return false;
    }
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

void IncallDataStateMachine::Init(int32_t callState, int32_t slotId,
    std::weak_ptr<TelEventHandler> &&cellularDataHandler, sptr<ApnManager> &apnManager)
{
    slotId_ = slotId;
    cellularDataHandler_ = std::move(cellularDataHandler);
    apnManager_ = apnManager;
    idleState_ = std::make_shared<IdleState>(std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "IdleState");
    secondaryActiveState_ = std::make_shared<SecondaryActiveState>(
        std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "SecondaryActiveState");
    activatingSecondaryState_ = std::make_shared<ActivatingSecondaryState>(
        std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "ActivatingSecondaryState");
    activatedSecondaryState_ = std::make_shared<ActivatedSecondaryState>(
        std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "ActivatedSecondaryState");
    deactivatingSecondaryState_ = std::make_shared<DeactivatingSecondaryState>(
        std::weak_ptr<IncallDataStateMachine>(shared_from_this()), "DeactivatingSecondaryState");
    if (idleState_ == nullptr || secondaryActiveState_ == nullptr || activatingSecondaryState_ == nullptr ||
        activatedSecondaryState_ == nullptr || deactivatingSecondaryState_ == nullptr) {
        TELEPHONY_LOGE("memory allocation failed");
        return;
    }
    activatingSecondaryState_->SetParentState(secondaryActiveState_);
    activatedSecondaryState_->SetParentState(secondaryActiveState_);
    callState_ = callState;
    StateMachine::SetOriginalState(idleState_);
    StateMachine::Start();
}

void IncallDataStateMachine::SetCurrentState(std::shared_ptr<State> state)
{
    currentState_ = state;
}

std::shared_ptr<State> IncallDataStateMachine::GetCurrentState() const
{
    return currentState_;
}

bool IncallDataStateMachine::IsSecondaryActiveState() const
{
    return currentState_ == activatingSecondaryState_ || currentState_ == activatedSecondaryState_;
}

void IncallDataStateMachine::DeInit()
{
    idleState_ = nullptr;
    secondaryActiveState_ = nullptr;
    activatingSecondaryState_ = nullptr;
    activatedSecondaryState_ = nullptr;
    deactivatingSecondaryState_ = nullptr;
    currentState_ = nullptr;
    cellularDataHandler_.reset();
    apnManager_ = nullptr;

    slotId_ = INVALID_SLOT_ID;
    callState_ = static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE);
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
    stateMachine->SetCurrentState(shared_from_this());
    if (stateMachine->GetCallState() == static_cast<int32_t>(TelCallStatus::CALL_STATUS_IDLE) ||
        stateMachine->GetCallState() == static_cast<int32_t>(TelCallStatus::CALL_STATUS_DISCONNECTED)) {
        std::shared_ptr<TelEventHandler> eventHandler = stateMachine->cellularDataHandler_.lock();
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
        return it->second(event);
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
    if (stateMachine->IsIncallDataSwitchOn() && stateMachine->IsSecondaryCanActiveData()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        // LCOV_EXCL_START
        int32_t targetSlotId = stateMachine->GetTargetDataSlotId(defaultSlotId);
        if (defaultSlotId != targetSlotId && !stateMachine->CheckBackupNetworkIfAllow(targetSlotId)) {
            TELEPHONY_LOGI("Slot%{public}d: target slot %{public}d not allowed to switch",
                stateMachine->GetSlotId(), targetSlotId);
            return PROCESSED;
        }
        if (defaultSlotId != targetSlotId) {
            stateMachine->TransitionTo(stateMachine->activatingSecondaryState_);
            CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(targetSlotId);
            stateMachine->SetPrimarySlot(targetSlotId);
        }
        // LCOV_EXCL_STOP
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
    std::shared_ptr<TelEventHandler> eventHandler = stateMachine->cellularDataHandler_.lock();
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
    if (stateMachine->IsIncallDataSwitchOn() && stateMachine->IsSecondaryCanActiveData()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        // LCOV_EXCL_START
        int32_t targetSlotId = stateMachine->GetTargetDataSlotId(defaultSlotId);
        if (defaultSlotId != targetSlotId) {
            stateMachine->TransitionTo(stateMachine->activatingSecondaryState_);
            CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(targetSlotId);
            stateMachine->SetPrimarySlot(targetSlotId);
        }
        // LCOV_EXCL_STOP
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
    if (stateMachine->IsIncallDataSwitchOn() && stateMachine->IsSecondaryCanActiveData()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        // LCOV_EXCL_START
        int32_t targetSlotId = stateMachine->GetTargetDataSlotId(defaultSlotId);
        if (defaultSlotId != targetSlotId) {
            stateMachine->TransitionTo(stateMachine->activatingSecondaryState_);
            CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(targetSlotId);
            stateMachine->SetPrimarySlot(targetSlotId);
        }
        // LCOV_EXCL_STOP
    }
    return PROCESSED;
}

void SecondaryActiveState::StateBegin()
{
    TELEPHONY_LOGI("Enter SecondaryActive State");
    isActive_ = true;
}

void SecondaryActiveState::StateEnd()
{
    TELEPHONY_LOGI("Exit SecondaryActive State");
    isActive_ = false;
}

bool SecondaryActiveState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
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
        return it->second(event);
    }
    return NOT_PROCESSED;
}

bool SecondaryActiveState::ProcessSettingsOn(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SecondaryActiveState::MSG_SM_INCALL_DATA_SETTINGS_ON");
    return PROCESSED;
}

bool SecondaryActiveState::ProcessCallEnded(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SecondaryActiveState::MSG_SM_INCALL_DATA_CALL_ENDED");
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
            stateMachine->TransitionTo(stateMachine->deactivatingSecondaryState_);
        } else {
            stateMachine->TransitionTo(stateMachine->idleState_);
        }
    }
    return PROCESSED;
}

bool SecondaryActiveState::ProcessSettingsOff(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SecondaryActiveState::MSG_SM_INCALL_DATA_SETTINGS_OFF");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return NOT_PROCESSED;
    }
    if (!stateMachine->IsIncallDataSwitchOn()) {
        int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
        int32_t primarySlotId = INVALID_SLOT_ID;
        CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
        if (defaultSlotId != primarySlotId) {
            stateMachine->TransitionTo(stateMachine->deactivatingSecondaryState_);
        } else {
            stateMachine->TransitionTo(stateMachine->idleState_);
        }
    }
    return PROCESSED;
}

bool SecondaryActiveState::ProcessDsdsChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SecondaryActiveState::MSG_SM_INCALL_DATA_DSDS_CHANGED");
    return PROCESSED;
}

void ActivatingSecondaryState::StateBegin()
{
    TELEPHONY_LOGI("Enter ActivatingSecondary State");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(shared_from_this());
}

void ActivatingSecondaryState::StateEnd()
{
    TELEPHONY_LOGI("Exit ActivatingSecondary State");
    isActive_ = false;
}

bool ActivatingSecondaryState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
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
        stateMachine->TransitionTo(stateMachine->activatedSecondaryState_);
        return PROCESSED;
    }
    return NOT_PROCESSED;
}

void ActivatedSecondaryState::StateBegin()
{
    TELEPHONY_LOGI("Enter ActivatedSecondary State");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(shared_from_this());
}

void ActivatedSecondaryState::StateEnd()
{
    TELEPHONY_LOGI("Exit ActivatedSecondary State");
    isActive_ = false;
}

bool ActivatedSecondaryState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    return NOT_PROCESSED;
}

void DeactivatingSecondaryState::StateBegin()
{
    TELEPHONY_LOGI("Enter DeactivatingSecondary State");
    std::shared_ptr<IncallDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    isActive_ = true;
    stateMachine->SetCurrentState(shared_from_this());
    int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    int32_t primarySlotId = INVALID_SLOT_ID;
    CoreManagerInner::GetInstance().GetPrimarySlotId(primarySlotId);
    if (defaultSlotId != primarySlotId) {
        CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(primarySlotId);
        stateMachine->SetPrimarySlot(primarySlotId);
    } else {
        stateMachine->TransitionTo(stateMachine->idleState_);
    }
    if (!stateMachine->HasAnyConnectedState()) {
        stateMachine->TransitionTo(stateMachine->idleState_);
    }
}

void DeactivatingSecondaryState::StateEnd()
{
    TELEPHONY_LOGI("Exit DeactivatingSecondary State");
    isActive_ = false;
}

bool DeactivatingSecondaryState::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
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
