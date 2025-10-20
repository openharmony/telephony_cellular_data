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
#include "cellular_data_constant.h"
#include "cellular_data_event_code.h"
#include "cellular_data_incall_observer.h"
#include "cellular_data_rdb_observer.h"
#include "cellular_data_roaming_observer.h"
#include "cellular_data_setting_observer.h"
#include "cellular_data_airplane_observer.h"
#include "cellular_data_state_machine.h"
#include "common_event_manager.h"
#include "data_switch_settings.h"
#include "incall_data_state_machine.h"
#include "inner_event.h"
#include "radio_event.h"
#include "state_notification.h"
#include "tel_event_handler.h"
#include "tel_profile_util.h"
#include "tel_ril_data_parcel.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
const uint32_t KEEP_APN_ACTIVATE_PERIOD = 30 * 1000;
#ifdef BASE_POWER_IMPROVEMENT
class CellularDataPowerSaveModeSubscriber;
#endif
class CellularDataHandler : public TelEventHandler, public EventFwk::CommonEventSubscriber {
public:
    explicit CellularDataHandler(const EventFwk::CommonEventSubscribeInfo &sp, int32_t slotId);
    ~CellularDataHandler();
    void Init();
    bool ReleaseNet(const NetRequest &request);
    bool RequestNet(const NetRequest &request);
    bool AddUid(const NetRequest &request);
    bool RemoveUid(const NetRequest &request);
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    int32_t SetCellularDataEnable(bool userDataEnabled);
    int32_t SetIntelligenceSwitchEnable(bool userDataEnabled);
    int32_t IsCellularDataEnabled(bool &dataEnabled) const;
    int32_t IsCellularDataRoamingEnabled(bool &dataRoamingEnabled) const;
    int32_t SetCellularDataRoamingEnabled(bool dataRoamingEnabled);
    ApnProfileState GetCellularDataState() const;
    ApnProfileState GetCellularDataState(const std::string &apnType) const;
    void ClearConnection(const sptr<ApnHolder> &apnHolder, DisConnectionReason reason);
    void EstablishAllApnsIfConnectable();
    void ClearAllConnections(DisConnectionReason reason);
    void ClearConnectionsOnUpdateApns(DisConnectionReason reason);
    bool ChangeConnectionForDsds(bool enable);
    int32_t GetSlotId() const;
    bool HandleApnChanged();
    void HandleApnChanged(const AppExecFwk::InnerEvent::Pointer &event);
    int32_t GetCellularDataFlowType();
    void SetPolicyDataOn(bool enable);
    bool IsRestrictedMode() const;
    DisConnectionReason GetDisConnectionReason();
    bool HasInternetCapability(const int32_t cid) const;
    void GetDataConnApnAttr(ApnItem::Attribute &apnAttr) const;
    std::string GetDataConnIpType() const;
    int32_t GetDataRecoveryState();
    void SetRilAttachApn();
    void IsNeedDoRecovery(bool needDoRecovery) const;
    void RegisterDataSettingObserver();
    void UnRegisterDataSettingObserver();
    int32_t GetIntelligenceSwitchState(bool &switchState);
    void HandleUpdateNetInfo(const AppExecFwk::InnerEvent::Pointer &event);
    void ReleaseCellularDataConnection();
    bool UpdateNetworkInfo();
    bool IsSupportDunApn();
    void ConnectIfNeed(
        const AppExecFwk::InnerEvent::Pointer &event, sptr<ApnHolder> apnHolder, const NetRequest &request);
#ifdef BASE_POWER_IMPROVEMENT
    void SubscribeTelePowerEvent();
#endif

    ApnActivateReportInfo GetDefaultActReportInfo();
    ApnActivateReportInfo GetInternalActReportInfo();
#ifdef BASE_POWER_IMPROVEMENT
    std::shared_ptr<CellularDataPowerSaveModeSubscriber> strEnterSubscriber_ = nullptr;
    std::shared_ptr<CellularDataPowerSaveModeSubscriber> strExitSubscriber_ = nullptr;
#endif

private:
    std::shared_ptr<CellularDataStateMachine> CreateCellularDataConnect();
    std::shared_ptr<CellularDataStateMachine> FindIdleCellularDataConnection() const;
    bool CheckCellularDataSlotId(sptr<ApnHolder> &apnHolder);
    bool CheckAttachAndSimState(sptr<ApnHolder> &apnHolder);
    bool CheckRoamingState(sptr<ApnHolder> &apnHolder);
    bool CheckApnState(sptr<ApnHolder> &apnHolder);
    bool IsMultiDefaultApn(const sptr<ApnHolder> &apnHolder);
    bool CheckMultiApnState(sptr<ApnHolder> &apnHolder);
    void AttemptEstablishDataConnection(sptr<ApnHolder> &apnHolder);
    bool EstablishDataConnection(sptr<ApnHolder> &apnHolder, int32_t radioTech);
    void RadioPsConnectionAttached(const AppExecFwk::InnerEvent::Pointer &event);
    void RoamingStateOn(const AppExecFwk::InnerEvent::Pointer &event);
    void RoamingStateOff(const AppExecFwk::InnerEvent::Pointer &event);
    void PsRadioEmergencyStateOpen(const AppExecFwk::InnerEvent::Pointer &event);
    void PsRadioEmergencyStateClose(const AppExecFwk::InnerEvent::Pointer &event);
    void EstablishDataConnectionComplete(const AppExecFwk::InnerEvent::Pointer &event);
    void DisconnectDataComplete(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleDisconnectDataCompleteForMmsType(sptr<ApnHolder> &apnHolder);
    void MsgEstablishDataConnection(const AppExecFwk::InnerEvent::Pointer &event);
    void MsgRequestNetwork(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSettingSwitchChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleVoiceCallChanged(int32_t state);
    void HandleDefaultDataSubscriptionChanged();
    void HandleSimStateChanged();
    void HandleRecordsChanged();
    void HandleDsdsModeChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSimEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSimAccountLoaded();
    void HandleRadioStateChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void PsDataRatChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void SetRilAttachApnResponse(const AppExecFwk::InnerEvent::Pointer &event);
    bool HasAnyHigherPriorityConnection(const sptr<ApnHolder> &apnHolder);
    void GetConfigurationFor5G();
    void GetDefaultConfiguration();
    void GetDefaultDataRoamingConfig();
    void GetDefaultDataEnableConfig();
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
    void SetDataPermitted(int32_t slotId, bool dataPermitted);
    bool CheckDataPermittedByDsds();
    bool SetDataPermittedForMms(bool dataPermittedForMms);
    std::shared_ptr<IncallDataStateMachine> CreateIncallDataStateMachine(int32_t callState);
    void HandleDBSettingIncallChanged(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleCallChanged(int32_t state);
    void HandleImsCallChanged(int32_t state);
    void IncallDataComplete(const AppExecFwk::InnerEvent::Pointer &event);
    void ResetDataFlowType();
    void ClearConnectionIfRequired();
    void ReleaseAllNetworkRequest();
    bool GetEsmFlagFromOpCfg();
    void GetSinglePdpEnabledFromOpCfg();
    bool IsSingleConnectionEnabled(int32_t radioTech);
    void OnRilAdapterHostDied(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleFactoryReset(const AppExecFwk::InnerEvent::Pointer &event);
    void OnCleanAllDataConnectionsDone(const AppExecFwk::InnerEvent::Pointer &event);
    void ResumeDataPermittedTimerOut(const AppExecFwk::InnerEvent::Pointer &event);
    void CreateApnItem();
    void UpdatePhysicalConnectionState(bool noActiveConnection);
    bool IsVSimSlotId(int32_t slotId);
    std::shared_ptr<CellularDataStateMachine> CheckForCompatibleDataConnection(sptr<ApnHolder> &apnHolder);
    bool IsGsm();
    bool IsCdma();
    void HandleScreenStateChanged(bool isScreenOn) const;
    void HandleEstablishAllApnsIfConnectable(const AppExecFwk::InnerEvent::Pointer &event);
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    bool IsSimRequestNetOnVSimEnabled(int32_t reqType, bool isMmsType) const;
    bool NotifyReqCellularData(bool isCellularDataRequested);
#endif
    void SetNetRequest(NetRequest &request, const std::unique_ptr<NetRequest> &netRequest);
    void SendEstablishDataConnectionEvent(int32_t id);
    bool IsSimStateReadyOrLoaded();
    void UpdateCellularDataConnectState(const std::string &apnType);
    void RetryToSetupDatacall(const AppExecFwk::InnerEvent::Pointer &event);
    void RetryOrClearConnection(const sptr<ApnHolder> &apnHolder, DisConnectionReason reason,
        const std::shared_ptr<SetupDataCallResultInfo> &netInfo);
    std::shared_ptr<DataShare::DataShareHelper> CreatorDataShareHelper();
    bool GetCurrentDataShareApnInfo(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
        const int32_t simId, int32_t &profileIdValue);
    void UpdateApnInfo(const int32_t profileId);
    int32_t GetCurrentApnId();
    bool WriteEventCellularRequest(NetRequest request, int32_t state);
    void DataConnCompleteUpdateState(const sptr<ApnHolder> &apnHolder,
        const std::shared_ptr<SetupDataCallResultInfo> &resultInfo);
    bool HandleCompatibleDataConnection(std::shared_ptr<CellularDataStateMachine> stateMachine,
        const sptr<ApnHolder> &apnHolder);
    int64_t GetCurTime();
    void SetApnActivateStart(const std::string &apnType);
    void SetApnActivateEnd(const std::shared_ptr<SetupDataCallResultInfo> &resultInfo);
    void EraseApnActivateList();
    ApnActivateReportInfo GetApnActReportInfo(uint32_t apnId);

private:
    sptr<ApnManager> apnManager_;
    std::unique_ptr<DataSwitchSettings> dataSwitchSettings_;
    sptr<DataConnectionManager> connectionManager_;
    std::u16string lastIccId_;
    int32_t lastCallState_ = (int32_t)TelCallStatus::CALL_STATUS_IDLE;
    const int32_t slotId_;
    DisConnectionReason disconnectionReason_ = DisConnectionReason::REASON_NORMAL;
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
    bool defaultDataRoamingEnable_ = false;
    bool isRilApnAttached_ = false;
    std::mutex mtx_;
    std::mutex apnActivateListMutex_;
    std::vector<std::string> upLinkThresholds_;
    std::vector<std::string> downLinkThresholds_;
    sptr<CellularDataSettingObserver> settingObserver_;
    sptr<CellularDataRoamingObserver> roamingObserver_;
    sptr<CellularDataIncallObserver> incallObserver_;
    sptr<CellularDataRdbObserver> cellularDataRdbObserver_;
    sptr<CellularDataAirplaneObserver> airplaneObserver_;
    std::shared_ptr<IncallDataStateMachine> incallDataStateMachine_;
    sptr<ApnItem> lastApnItem_ = nullptr;
    std::vector<ApnActivateInfo> apnActivateChrList_;
    uint64_t defaultApnActTime_ = 0;
    uint64_t internalApnActTime_ = 0;
    int32_t retryCreateApnTimes_ = 0;

    using Fun = std::function<void(const AppExecFwk::InnerEvent::Pointer &event)>;
    std::map<uint32_t, Fun> eventIdMap_ {
        { RadioEvent::RADIO_PS_CONNECTION_ATTACHED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { RadioPsConnectionAttached(event); } },
        { RadioEvent::RADIO_PS_ROAMING_OPEN,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { RoamingStateOn(event); } },
        { RadioEvent::RADIO_PS_ROAMING_CLOSE,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { RoamingStateOff(event); } },
        { RadioEvent::RADIO_EMERGENCY_STATE_OPEN,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { PsRadioEmergencyStateOpen(event); } },
        { RadioEvent::RADIO_EMERGENCY_STATE_CLOSE,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { PsRadioEmergencyStateClose(event); } },
        { CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION_COMPLETE,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { EstablishDataConnectionComplete(event); } },
        { CellularDataEventCode::MSG_DISCONNECT_DATA_COMPLETE,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { DisconnectDataComplete(event); } },
        { CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { MsgEstablishDataConnection(event); } },
        { CellularDataEventCode::MSG_SETTING_SWITCH,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleSettingSwitchChanged(event); } },
        { CellularDataEventCode::MSG_REQUEST_NETWORK,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { MsgRequestNetwork(event); } },
        { RadioEvent::RADIO_STATE_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleRadioStateChanged(event); } },
        { RadioEvent::RADIO_DSDS_MODE_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleDsdsModeChanged(event); } },
        { RadioEvent::RADIO_SIM_STATE_CHANGE,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleSimEvent(event); } },
        { RadioEvent::RADIO_SIM_RECORDS_LOADED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleSimEvent(event); } },
        { RadioEvent::RADIO_SIM_ACCOUNT_LOADED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleSimEvent(event); } },
        { RadioEvent::RADIO_PS_RAT_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { PsDataRatChanged(event); } },
        { CellularDataEventCode::MSG_APN_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleApnChanged(event); } },
        { CellularDataEventCode::MSG_SET_RIL_ATTACH_APN,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { SetRilAttachApnResponse(event); } },
        { RadioEvent::RADIO_NR_STATE_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleRadioNrStateChanged(event); } },
        { RadioEvent::RADIO_NR_FREQUENCY_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleRadioNrFrequencyChanged(event); } },
        { CellularDataEventCode::MSG_DB_SETTING_ENABLE_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleDBSettingEnableChanged(event); } },
        { CellularDataEventCode::MSG_DB_SETTING_ROAMING_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleDBSettingRoamingChanged(event); } },
        { CellularDataEventCode::MSG_DB_SETTING_INCALL_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleDBSettingIncallChanged(event); } },
        { CellularDataEventCode::MSG_INCALL_DATA_COMPLETE,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { IncallDataComplete(event); } },
        { RadioEvent::RADIO_RIL_ADAPTER_HOST_DIED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { OnRilAdapterHostDied(event); } },
        { RadioEvent::RADIO_FACTORY_RESET,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleFactoryReset(event); } },
        { RadioEvent::RADIO_CLEAN_ALL_DATA_CONNECTIONS,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { OnCleanAllDataConnectionsDone(event); } },
        { CellularDataEventCode::MSG_DATA_CALL_LIST_CHANGED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleUpdateNetInfo(event); } },
        { RadioEvent::RADIO_NV_REFRESH_FINISHED,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleSimEvent(event); } },
        { CellularDataEventCode::MSG_RETRY_TO_SETUP_DATACALL,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { RetryToSetupDatacall(event); } },
        { CellularDataEventCode::MSG_ESTABLISH_ALL_APNS_IF_CONNECTABLE,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleEstablishAllApnsIfConnectable(event); } },
        { CellularDataEventCode::MSG_RESUME_DATA_PERMITTED_TIMEOUT,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { ResumeDataPermittedTimerOut(event); } },
        { CellularDataEventCode::MSG_RETRY_TO_CREATE_APN,
            [this](const AppExecFwk::InnerEvent::Pointer &event) { HandleApnChanged(event); } },
    };
#ifdef BASE_POWER_IMPROVEMENT
    std::shared_ptr<CellularDataPowerSaveModeSubscriber> CreateCommonSubscriber(
        const std::string &event, int32_t priority);
    void SendPowerSaveModeEvent(uint32_t eventId, std::shared_ptr<CellularDataPowerSaveModeSubscriber> &subscriber);
#endif
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_DATA_HANDLER_H
