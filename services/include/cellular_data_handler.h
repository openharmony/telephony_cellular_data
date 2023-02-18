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

#ifndef CELLULAR_DATA_HANDLER_H
#define CELLULAR_DATA_HANDLER_H

#include <atomic>
#include <memory>

#include "apn_manager.h"
#include "cellular_data_event_code.h"
#include "cellular_data_roaming_observer.h"
#include "cellular_data_setting_observer.h"
#include "cellular_data_state_machine.h"
#include "common_event_manager.h"
#include "data_switch_settings.h"
#include "event_handler.h"
#include "hril_data_parcel.h"
#include "inner_event.h"
#include "radio_event.h"
#include "state_notification.h"
#include "tel_profile_util.h"

namespace OHOS {
namespace Telephony {
class CellularDataHandler : public AppExecFwk::EventHandler, public EventFwk::CommonEventSubscriber {
public:
    explicit CellularDataHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
        const EventFwk::CommonEventSubscribeInfo &sp, int32_t slotId);
    ~CellularDataHandler();
    void Init();
    bool ReleaseNet(const NetRequest &request);
    bool RequestNet(const NetRequest &request);
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    int32_t SetCellularDataEnable(bool userDataEnabled);
    int32_t IsCellularDataEnabled(bool &dataEnabled) const;
    int32_t IsCellularDataRoamingEnabled(bool &dataRoamingEnabled) const;
    int32_t SetCellularDataRoamingEnabled(bool dataRoamingEnabled);
    ApnProfileState GetCellularDataState() const;
    ApnProfileState GetCellularDataState(const std::string &apnType) const;
    void ClearConnection(const sptr<ApnHolder> &apnHolder, DisConnectionReason reason);
    void EstablishAllApnsIfConnectable();
    void ClearAllConnections(DisConnectionReason reason);
    int32_t GetSlotId() const;
    bool HandleApnChanged();
    void HandleApnChanged(const AppExecFwk::InnerEvent::Pointer &event);
    int32_t GetCellularDataFlowType();
    void SetPolicyDataOn(bool enable);
    bool IsRestrictedMode() const;
    DisConnectionReason GetDisConnectionReason();
    bool HasInternetCapability(const int32_t cid) const;
    void SetDataPermitted(bool dataPermitted);

private:
    std::shared_ptr<CellularDataStateMachine> CreateCellularDataConnect();
    std::shared_ptr<CellularDataStateMachine> FindIdleCellularDataConnection() const;
    bool CheckCellularDataSlotId();
    bool CheckAttachAndSimState(sptr<ApnHolder> &apnHolder);
    bool CheckRoamingState(sptr<ApnHolder> &apnHolder);
    bool CheckApnState(sptr<ApnHolder> &apnHolder);
    void AttemptEstablishDataConnection(sptr<ApnHolder> &apnHolder);
    bool EstablishDataConnection(sptr<ApnHolder> &apnHolder, int32_t radioTech);
    void RadioPsConnectionAttached(const AppExecFwk::InnerEvent::Pointer &event);
    void RadioPsConnectionDetached(const AppExecFwk::InnerEvent::Pointer &event);
    void RoamingStateOn(const AppExecFwk::InnerEvent::Pointer &event);
    void RoamingStateOff(const AppExecFwk::InnerEvent::Pointer &event);
    void PsRadioEmergencyStateOpen(const AppExecFwk::InnerEvent::Pointer &event);
    void PsRadioEmergencyStateClose(const AppExecFwk::InnerEvent::Pointer &event);
    void EstablishDataConnectionComplete(const AppExecFwk::InnerEvent::Pointer &event);
    void DisconnectDataComplete(const AppExecFwk::InnerEvent::Pointer &event);
    void MsgEstablishDataConnection(const AppExecFwk::InnerEvent::Pointer &event);
    void MsgRequestNetwork(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSettingSwitchChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleVoiceCallChanged(int32_t state);
    void HandleSimStateOrRecordsChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSimAccountLoaded(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRadioStateChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void PsDataRatChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void SetRilAttachApn();
    void SetRilAttachApnResponse(const AppExecFwk::InnerEvent::Pointer &event);
    bool HasAnyHigherPriorityConnection(const sptr<ApnHolder> &apnHolder);
    void GetConfigurationFor5G();
    void GetDefaultConfiguration();
    bool ParseOperatorConfig(const std::u16string &configName);
    void HandleRadioNrStateChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRadioNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void GetDefaultUpLinkThresholdsConfig();
    void GetDefaultDownLinkThresholdsConfig();
    void SetRilLinkBandwidths();
    void HandleDBSettingEnableChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleDBSettingRoamingChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSortConnection();
    void SetDataPermittedResponse(const AppExecFwk::InnerEvent::Pointer &event);
    void SyncDataPermitted();
    void RegisterDataSettingObserver();
    void UnRegisterDataSettingObserver();

private:
    sptr<ApnManager> apnManager_;
    std::unique_ptr<DataSwitchSettings> dataSwitchSettings_;
    sptr<DataConnectionManager> connectionManager_;
    std::u16string lastIccId_;
    int32_t lastCallState_ = (int32_t)TelCallStatus::CALL_STATUS_IDLE;
    const int32_t slotId_;
    DisConnectionReason disconnectionReason_ = DisConnectionReason::REASON_NORMAL;
    std::shared_ptr<AppExecFwk::EventRunner> stateMachineEventLoop_;
    bool unMeteredAllNsaConfig_ = false;
    bool unMeteredNrNsaMmwaveConfig_ = false;
    bool unMeteredNrNsaSub6Config_ = false;
    bool unMeteredAllNrsaConfig_ = false;
    bool unMeteredNrsaMmwaveConfig_ = false;
    bool unMeteredNrsaSub6Config_ = false;
    bool unMeteredRoamingConfig_ = false;
    int defaultMobileMtuConfig_ = 0;
    bool defaultPreferApn_ = true;
    bool physicalConnectionActiveState_ = false;
    bool multipleConnectionsEnabled_ = false;
    std::vector<std::string> upLinkThresholds_;
    std::vector<std::string> downLinkThresholds_;
    sptr<CellularDataSettingObserver> settingObserver_;
    sptr<CellularDataRoamingObserver> roamingObserver_;

    using Fun = void (CellularDataHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<uint32_t, Fun> eventIdMap_ {
        {RadioEvent::RADIO_PS_CONNECTION_ATTACHED, &CellularDataHandler::RadioPsConnectionAttached},
        {RadioEvent::RADIO_PS_CONNECTION_DETACHED, &CellularDataHandler::RadioPsConnectionDetached},
        {RadioEvent::RADIO_PS_ROAMING_OPEN, &CellularDataHandler::RoamingStateOn},
        {RadioEvent::RADIO_PS_ROAMING_CLOSE, &CellularDataHandler::RoamingStateOff},
        {RadioEvent::RADIO_EMERGENCY_STATE_OPEN, &CellularDataHandler::PsRadioEmergencyStateOpen},
        {RadioEvent::RADIO_EMERGENCY_STATE_CLOSE, &CellularDataHandler::PsRadioEmergencyStateClose},
        {CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION_COMPLETE,
            &CellularDataHandler::EstablishDataConnectionComplete},
        {CellularDataEventCode::MSG_DISCONNECT_DATA_COMPLETE, &CellularDataHandler::DisconnectDataComplete},
        {CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, &CellularDataHandler::MsgEstablishDataConnection},
        {CellularDataEventCode::MSG_SETTING_SWITCH, &CellularDataHandler::HandleSettingSwitchChanged},
        {CellularDataEventCode::MSG_REQUEST_NETWORK, &CellularDataHandler::MsgRequestNetwork},
        {RadioEvent::RADIO_STATE_CHANGED, &CellularDataHandler::HandleRadioStateChanged},
        {RadioEvent::RADIO_SIM_STATE_CHANGE, &CellularDataHandler::HandleSimStateOrRecordsChanged},
        {RadioEvent::RADIO_SIM_RECORDS_LOADED, &CellularDataHandler::HandleSimStateOrRecordsChanged},
        {RadioEvent::RADIO_SIM_ACCOUNT_LOADED, &CellularDataHandler::HandleSimAccountLoaded},
        {RadioEvent::RADIO_PS_RAT_CHANGED, &CellularDataHandler::PsDataRatChanged},
        {CellularDataEventCode::MSG_APN_CHANGED, &CellularDataHandler::HandleApnChanged},
        {CellularDataEventCode::MSG_SET_RIL_ATTACH_APN, &CellularDataHandler::SetRilAttachApnResponse},
        {RadioEvent::RADIO_NR_STATE_CHANGED, &CellularDataHandler::HandleRadioNrStateChanged},
        {RadioEvent::RADIO_NR_FREQUENCY_CHANGED, &CellularDataHandler::HandleRadioNrFrequencyChanged},
        {CellularDataEventCode::MSG_DB_SETTING_ENABLE_CHANGED, &CellularDataHandler::HandleDBSettingEnableChanged},
        {CellularDataEventCode::MSG_DB_SETTING_ROAMING_CHANGED, &CellularDataHandler::HandleDBSettingRoamingChanged},
    };
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_HANDLER_H
