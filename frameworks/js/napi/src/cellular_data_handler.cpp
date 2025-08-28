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

#include "cellular_data_handler.h"

#include "cellular_data_constant.h"
#include "cellular_data_error.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_service.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_types.h"
#include "cellular_data_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "core_manager_inner.h"
#include "hitrace_meter.h"
#include "tel_ril_call_parcel.h"
#include "net_specifier.h"
#include "net_all_capabilities.h"
#include "radio_event.h"
#include "str_convert.h"
#include "string_ex.h"
#include "telephony_log_wrapper.h"
#include "telephony_ext_wrapper.h"
#include "telephony_permission.h"
#include "ipc_skeleton.h"
#include "connection_retry_policy.h"
#include "pdp_profile_data.h"
#ifdef BASE_POWER_IMPROVEMENT
#include "cellular_data_power_save_mode_subscriber.h"
#endif
namespace OHOS {
namespace Telephony {
using namespace AppExecFwk;
using namespace OHOS::EventFwk;
using namespace NetManagerStandard;
static const int32_t ESM_FLAG_INVALID = -1;
const std::string DEFAULT_DATA_ROAMING = "persist.telephony.defaultdataroaming";
#ifdef BASE_POWER_IMPROVEMENT
constexpr const char *PERMISSION_STARTUP_COMPLETED = "ohos.permission.RECEIVER_STARTUP_COMPLETED";
#endif
constexpr const char *PERSIST_EDM_MOBILE_DATA_POLICY = "persist.edm.mobile_data_policy";
constexpr const char *MOBILE_DATA_POLICY_FORCE_OPEN = "force_open";
constexpr const char *MOBILE_DATA_POLICY_DISALLOW = "disallow";
CellularDataHandler::CellularDataHandler(const EventFwk::CommonEventSubscribeInfo &sp, int32_t slotId)
    : TelEventHandler("CellularDataHandler"), CommonEventSubscriber(sp), slotId_(slotId)
{}

void CellularDataHandler::Init()
{
    lastApnItem_ = new ApnItem();
    apnManager_ = std::make_unique<ApnManager>().release();
    dataSwitchSettings_ = std::make_unique<DataSwitchSettings>(slotId_);
    connectionManager_ = std::make_unique<DataConnectionManager>(slotId_).release();
    settingObserver_ = new (std::nothrow) CellularDataSettingObserver(
        std::weak_ptr<TelEventHandler>(std::static_pointer_cast<TelEventHandler>(shared_from_this())));
    roamingObserver_ = new (std::nothrow) CellularDataRoamingObserver(
        std::weak_ptr<TelEventHandler>(std::static_pointer_cast<TelEventHandler>(shared_from_this())), slotId_);
    incallObserver_ = new (std::nothrow) CellularDataIncallObserver(
        std::weak_ptr<TelEventHandler>(std::static_pointer_cast<TelEventHandler>(shared_from_this())));
    cellularDataRdbObserver_ = new (std::nothrow) CellularDataRdbObserver(
        std::weak_ptr<TelEventHandler>(std::static_pointer_cast<TelEventHandler>(shared_from_this())));
    airplaneObserver_ = new (std::nothrow) CellularDataAirplaneObserver();
    if ((apnManager_ == nullptr) || (dataSwitchSettings_ == nullptr) || (connectionManager_ == nullptr)) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or dataSwitchSettings_ or connectionManager_ is null", slotId_);
        return;
    }
    connectionManager_->Init();
    apnManager_->InitApnHolders();
    dataSwitchSettings_->LoadSwitchValue();
    GetConfigurationFor5G();
    SetRilLinkBandwidths();
}

CellularDataHandler::~CellularDataHandler() {}

bool CellularDataHandler::ReleaseNet(const NetRequest &request)
{
    std::unique_ptr<NetRequest> netRequest = std::make_unique<NetRequest>();
    if (netRequest == nullptr) {
        TELEPHONY_LOGE("Netrequest is null");
        return false;
    }
    netRequest->capability = ApnManager::FindBestCapability(request.capability);
    netRequest->ident = request.ident;
    AppExecFwk::InnerEvent::Pointer event =
        InnerEvent::Get(CellularDataEventCode::MSG_REQUEST_NETWORK, netRequest, TYPE_RELEASE_NET);
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    return SendEvent(event);
}

bool CellularDataHandler::RequestNet(const NetRequest &request)
{
    std::unique_ptr<NetRequest> netRequest = std::make_unique<NetRequest>();
    if (netRequest == nullptr) {
        TELEPHONY_LOGE("Netrequest is null");
        return false;
    }
    netRequest->capability = ApnManager::FindBestCapability(request.capability);
    netRequest->ident = request.ident;
    netRequest->registerType = request.registerType;
    netRequest->bearTypes = request.bearTypes;
    netRequest->uid = request.uid;
    AppExecFwk::InnerEvent::Pointer event =
        InnerEvent::Get(CellularDataEventCode::MSG_REQUEST_NETWORK, netRequest, TYPE_REQUEST_NET);
    return SendEvent(event);
}

bool CellularDataHandler::AddUid(const NetRequest &request)
{
    int32_t id = ApnManager::FindApnIdByCapability(ApnManager::FindBestCapability(request.capability));

    if (!apnManager_) {
        TELEPHONY_LOGE("apnManager_ is nullptr");
        return false;
    }

    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is nullptr");
        return false;
    }
    apnHolder->AddUid(request.uid);

    return true;
}

bool CellularDataHandler::RemoveUid(const NetRequest &request)
{
    int32_t id = ApnManager::FindApnIdByCapability(ApnManager::FindBestCapability(request.capability));

    if (!apnManager_) {
        TELEPHONY_LOGE("apnManager_ is nullptr");
        return false;
    }

    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("apnHolder is nullptr");
        return false;
    }
    apnHolder->RemoveUid(request.uid);
    return true;
}

int32_t CellularDataHandler::SetCellularDataEnable(bool userDataOn)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null.", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    bool dataEnabled = true;
    int32_t result = dataSwitchSettings_->QueryUserDataStatus(dataEnabled);
    if (result != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: Query result: %{public}d", slotId_, result);
        return result;
    }
    if (dataEnabled == userDataOn) {
        TELEPHONY_LOGI("Slot%{public}d: The status of the cellular data switch has not changed", slotId_);
        return TELEPHONY_ERR_SUCCESS;
    }

#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (TELEPHONY_EXT_WRAPPER.sendDataSwitchChangeInfo_) {
        int32_t callingUid = IPCSkeleton::GetCallingUid();
        int32_t callingPid = IPCSkeleton::GetCallingPid();
        std::string bundleName = "";
        TelephonyPermission::GetBundleNameByUid(callingUid, bundleName);
        TELEPHONY_EXT_WRAPPER.sendDataSwitchChangeInfo_(bundleName.c_str(), callingPid, userDataOn);
    }
#endif
    return dataSwitchSettings_->SetUserDataOn(userDataOn);
}

int32_t CellularDataHandler::SetIntelligenceSwitchEnable(bool userSwitchOn)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null.", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    bool switchEnabled = false;
    int32_t result = dataSwitchSettings_->QueryIntelligenceSwitchStatus(switchEnabled);
    if (result != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: Query result: %{public}d", slotId_, result);
    }
    if (switchEnabled == userSwitchOn) {
        TELEPHONY_LOGI("Slot%{public}d: The status of the cellular data switch has not changed", slotId_);
        return TELEPHONY_ERR_SUCCESS;
    }
    return dataSwitchSettings_->SetIntelliSwitchOn(userSwitchOn);
}

int32_t CellularDataHandler::IsCellularDataEnabled(bool &dataEnabled) const
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    if (dataSwitchSettings_->GetLastQryRet() != TELEPHONY_ERR_SUCCESS) {
        return dataSwitchSettings_->QueryUserDataStatus(dataEnabled);
    }
    dataEnabled = dataSwitchSettings_->IsUserDataOn();
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataHandler::IsCellularDataRoamingEnabled(bool &dataRoamingEnabled) const
{
    if (slotId_ != CELLULAR_DATA_VSIM_SLOT_ID) {
        int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
        if (simId <= INVALID_SIM_ID) {
            TELEPHONY_LOGE("Slot%{public}d: invalid sim id %{public}d", slotId_, simId);
            return TELEPHONY_ERR_LOCAL_PTR_NULL;
        }
    }
    dataRoamingEnabled = defaultDataRoamingEnable_;
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    dataSwitchSettings_->QueryUserDataRoamingStatus(dataRoamingEnabled);
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataHandler::GetIntelligenceSwitchState(bool &switchState)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null.", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    bool switchEnabled = false;
    int32_t result = dataSwitchSettings_->QueryIntelligenceSwitchStatus(switchEnabled);
    if (result != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: Query result: %{public}d", slotId_, result);
    }
    TELEPHONY_LOGI("GetIntelligenceSwitchState: %{public}d    -- %{public}d", switchState, switchEnabled);
    switchState = switchEnabled;
    return result;
}

int32_t CellularDataHandler::SetCellularDataRoamingEnabled(bool dataRoamingEnabled)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null", slotId_);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    bool currentDataRoamEnabled = defaultDataRoamingEnable_;
    int32_t result = dataSwitchSettings_->QueryUserDataRoamingStatus(currentDataRoamEnabled);
    if (result != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("Slot%{public}d: Query result: %{public}d", slotId_, result);
    }
    if (currentDataRoamEnabled == dataRoamingEnabled) {
        TELEPHONY_LOGI("Slot%{public}d: The status of the data roam switch has not changed", slotId_);
        return TELEPHONY_ERR_SUCCESS;
    }
    return dataSwitchSettings_->SetUserDataRoamingOn(dataRoamingEnabled);
}

void CellularDataHandler::ClearAllConnections(DisConnectionReason reason)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return;
    }
    for (const sptr<ApnHolder> &apn : apnManager_->GetAllApnHolder()) {
        ClearConnection(apn, reason);
    }

    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager_ is null", slotId_);
        return;
    }
    connectionManager_->StopStallDetectionTimer();
    connectionManager_->EndNetStatistics();

    ResetDataFlowType();
}

void CellularDataHandler::ClearConnectionsOnUpdateApns(DisConnectionReason reason)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return;
    }
    bool isRoaming = false;
    int32_t result = IsCellularDataRoamingEnabled(isRoaming);
    if (result != TELEPHONY_ERR_SUCCESS) {
        isRoaming = false;
    }
    auto apnItem = apnManager_->GetRilAttachApn();
    if (apnItem != nullptr) {
        TELEPHONY_LOGI("update preferId=%{public}d", apnItem->attr_.profileId_);
        UpdateApnInfo(apnItem->attr_.profileId_);
    }
    if (!ApnHolder::IsCompatibleApnItem(lastApnItem_, apnItem, isRoaming)) {
        ClearAllConnections(reason);
        if (lastApnItem_ == nullptr) {
            lastApnItem_ = new ApnItem();
        }
        if (apnItem == nullptr) {
            return;
        }
        *lastApnItem_ = *apnItem;
    }
}

void CellularDataHandler::ResetDataFlowType()
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: in ClearAllConnections dataSwitchSettings_ is null", slotId_);
        return;
    }
    bool dataEnabled = dataSwitchSettings_->IsUserDataOn();
    if (!dataEnabled) {
        connectionManager_->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_NONE);
    }
}

void CellularDataHandler::ClearConnection(const sptr<ApnHolder> &apn, DisConnectionReason reason)
{
    if (apn == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = apn->GetCellularDataStateMachine();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGD("Slot%{public}d: stateMachine is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: The APN holder is of type %{public}s, reason:%{public}d",
        slotId_, apn->GetApnType().c_str(), reason);
    std::unique_ptr<DataDisconnectParams> object = std::make_unique<DataDisconnectParams>(apn->GetApnType(), reason);
    if (object == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: ClearConnection fail, object is null", slotId_);
        return;
    }
    ApnProfileState apnState = apn->GetApnState();
    if (apnState == ApnProfileState::PROFILE_STATE_IDLE ||
        apnState == ApnProfileState::PROFILE_STATE_DISCONNECTING ||
        apnState == ApnProfileState::PROFILE_STATE_RETRYING) {
        TELEPHONY_LOGE("Slot%{public}d: apn state has been idle, disconnecting, or retrying", slotId_);
        return;
    }
    apn->SetApnState(PROFILE_STATE_DISCONNECTING);
    CellularDataHiSysEvent::WriteDataConnectStateBehaviorEvent(slotId_, apn->GetApnType(),
        apn->GetCapability(), static_cast<int32_t>(PROFILE_STATE_DISCONNECTING));
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SM_DISCONNECT, object);
    stateMachine->SendEvent(event);
}

ApnProfileState CellularDataHandler::GetCellularDataState() const
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    return apnManager_->GetOverallApnState();
}

ApnProfileState CellularDataHandler::GetCellularDataState(const std::string &apnType) const
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    sptr<ApnHolder> apnHolder = apnManager_->GetApnHolder(apnType);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return ApnProfileState::PROFILE_STATE_IDLE;
    }
    return apnHolder->GetApnState();
}

void CellularDataHandler::RadioPsConnectionAttached(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: ps attached", slotId_);
    if (event == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or apnManager_ is null", slotId_);
        return;
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RoamingStateOn(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: roaming on", slotId_);
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or dataSwitchSettings_ or apnManager_ is null", slotId_);
        return;
    }
    bool roamingState = false;
    if (CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0) {
        roamingState = true;
    }
    if (!roamingState) {
        TELEPHONY_LOGE("Slot%{public}d: device not currently roaming state", slotId_);
        return;
    }
    bool dataRoamingEnabled = false;
    int32_t res = IsCellularDataRoamingEnabled(dataRoamingEnabled);
    if (res != TELEPHONY_ERR_SUCCESS) {
        dataRoamingEnabled = false;
    }
    ApnProfileState apnState = apnManager_->GetOverallApnState();
    if ((apnState == ApnProfileState::PROFILE_STATE_CONNECTING || apnState == ApnProfileState::PROFILE_STATE_CONNECTED)
        && !dataRoamingEnabled) {
        ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::RoamingStateOff(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: roaming off", slotId_);
    if (event == nullptr || dataSwitchSettings_ == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or dataSwitchSettings_ or apnManager_ is null", slotId_);
        return;
    }
    bool dataRoamingEnabled = false;
    int32_t res = IsCellularDataRoamingEnabled(dataRoamingEnabled);
    if (res != TELEPHONY_ERR_SUCCESS) {
        dataRoamingEnabled = false;
    }
    if (!dataRoamingEnabled) {
        ApnProfileState apnState = apnManager_->GetOverallApnState();
        if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
            apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
            ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
        }
        EstablishAllApnsIfConnectable();
    }
}

void CellularDataHandler::PsRadioEmergencyStateOpen(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: emergency on", slotId_);
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_CONNECTED ||
        currentState == ApnProfileState::PROFILE_STATE_CONNECTING) {
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::PsRadioEmergencyStateClose(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: emergency off", slotId_);
    ApnProfileState currentState = apnManager_->GetOverallApnState();
    if (currentState == ApnProfileState::PROFILE_STATE_IDLE ||
        currentState == ApnProfileState::PROFILE_STATE_DISCONNECTING) {
        EstablishAllApnsIfConnectable();
    }
}

void CellularDataHandler::EstablishAllApnsIfConnectable()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return;
    }
    apnManager_->ClearAllApnBad();
    for (sptr<ApnHolder> apnHolder : apnManager_->GetSortApnHolder()) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: apn is null", slotId_);
            continue;
        }
        if (apnHolder->IsDataCallEnabled()) {
            UpdateCellularDataConnectState(apnHolder->GetApnType());
            ApnProfileState apnState = apnHolder->GetApnState();
            if (apnState == PROFILE_STATE_FAILED || apnState == PROFILE_STATE_RETRYING) {
                apnHolder->ReleaseDataConnection();
            }
            AttemptEstablishDataConnection(apnHolder);
        }
    }
}

bool CellularDataHandler::SetDataPermittedForMms(bool dataPermittedForMms)
{
    if (incallDataStateMachine_ != nullptr) {
        TELEPHONY_LOGI("Slot%{public}d: incall data active", slotId_);
        return false;
    }
    if (CheckDataPermittedByDsds()) {
        TELEPHONY_LOGI("Slot%{public}d: data permitted", slotId_);
        return false;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    SetDataPermitted(defSlotId, !dataPermittedForMms);
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (TELEPHONY_EXT_WRAPPER.isVSimEnabled_ && TELEPHONY_EXT_WRAPPER.isVSimEnabled_()) {
        SetDataPermitted(CELLULAR_DATA_VSIM_SLOT_ID, !dataPermittedForMms);
    }
#endif
    SetDataPermitted(slotId_, dataPermittedForMms);
    DelayedRefSingleton<CellularDataService>::GetInstance().ChangeConnectionForDsds(defSlotId, !dataPermittedForMms);
    return true;
}

bool CellularDataHandler::CheckDataPermittedByDsds()
{
    if (TELEPHONY_EXT_WRAPPER.getVSimSlotId_) {
        int vSimSlotId = INVALID_SLOT_ID;
        TELEPHONY_EXT_WRAPPER.getVSimSlotId_(vSimSlotId);
        if (vSimSlotId == CELLULAR_DATA_VSIM_SLOT_ID && slotId_ == CELLULAR_DATA_VSIM_SLOT_ID) {
            return true;
        }
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    int32_t dsdsMode = DSDS_MODE_V2;
    coreInner.GetDsdsMode(dsdsMode);
    if (defSlotId != slotId_ && dsdsMode == DSDS_MODE_V2) {
        TELEPHONY_LOGI("Slot%{public}d: default:%{public}d, current:%{public}d, dsdsMode:%{public}d", slotId_,
            defSlotId, slotId_, dsdsMode);
        return false;
    }
    return true;
}

bool CellularDataHandler::CheckCellularDataSlotId(sptr<ApnHolder> &apnHolder)
{
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return false;
    }
    if (IsVSimSlotId(slotId_)) {
        return true;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    std::string apnType = apnHolder->GetApnType();
    if (defSlotId != slotId_ && !apnType.compare(DATA_CONTEXT_ROLE_INTERNAL_DEFAULT)) {
        TELEPHONY_LOGD("Slot%{public}d: internalDefault:%{public}d, current:%{public}d", slotId_, defSlotId, slotId_);
        return false;
    }
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (slotId_ != CELLULAR_DATA_VSIM_SLOT_ID && !apnHolder->IsMmsType() &&
        TELEPHONY_EXT_WRAPPER.isVSimEnabled_ && TELEPHONY_EXT_WRAPPER.isVSimEnabled_() &&
        TELEPHONY_EXT_WRAPPER.isVSimInDisableProcess_ && !TELEPHONY_EXT_WRAPPER.isVSimInDisableProcess_()) {
        TELEPHONY_LOGE("slot%{public}d, VSimEnabled & not mms type, ret false", slotId_);
        return false;
    }
    if (TELEPHONY_EXT_WRAPPER.isDualCellularCardAllowed_) {
        if (TELEPHONY_EXT_WRAPPER.isDualCellularCardAllowed_()) {
            return true;
        }
    }
#endif

    if (defSlotId != slotId_ && !apnType.compare(DATA_CONTEXT_ROLE_DEFAULT)) {
        TELEPHONY_LOGD("Slot%{public}d: default:%{public}d, current:%{public}d", slotId_, defSlotId, slotId_);
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_CELLULAR_DATA_SLOT_ID_MISMATCH,
            "Default cellular data slot id is not current slot id");
        return false;
    }
    return true;
}

bool CellularDataHandler::CheckAttachAndSimState(sptr<ApnHolder> &apnHolder)
{
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return false;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    bool attached = coreInner.GetPsRegState(slotId_) == (int32_t)RegServiceState::REG_STATE_IN_SERVICE;
    bool isSimStateReadyOrLoaded = IsSimStateReadyOrLoaded();
    TELEPHONY_LOGD("Slot%{public}d: attached: %{public}d simState: %{public}d isRilApnAttached: %{public}d",
        slotId_, attached, isSimStateReadyOrLoaded, isRilApnAttached_);
    if (apnHolder->IsMmsType() && isSimStateReadyOrLoaded && !attached) {
        if (!HasInnerEvent(CellularDataEventCode::MSG_RESUME_DATA_PERMITTED_TIMEOUT)) {
            SendEvent(CellularDataEventCode::MSG_RESUME_DATA_PERMITTED_TIMEOUT,
                apnHolder->IsMmsType(), RESUME_DATA_PERMITTED_TIMEOUT);
        }
        SetDataPermittedForMms(true);
        return false;
    }
    bool isEmergencyApn = apnHolder->IsEmergencyType();
    if (!isEmergencyApn && !attached) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_PS_NOT_ATTACH, "It is not emergencyApn and not attached");
        return false;
    }
    if (!isEmergencyApn && !isSimStateReadyOrLoaded) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_SIM_NOT_READY, "It is not emergencyApn and sim not ready");
        return false;
    }
    return isEmergencyApn || isSimStateReadyOrLoaded;
}

bool CellularDataHandler::CheckRoamingState(sptr<ApnHolder> &apnHolder)
{
    if (dataSwitchSettings_ == nullptr || apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ or apnManager_ is null", slotId_);
        return false;
    }
    if (IsVSimSlotId(slotId_)) {
        return true;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    bool isEmergencyApn = apnHolder->IsEmergencyType();
    bool isMmsApn = apnHolder->IsMmsType();
    bool isBipApn = apnHolder->IsBipType();
    bool isAllowActiveData = dataSwitchSettings_->IsAllowActiveData();
    bool roamingState = coreInner.GetPsRoamingState(slotId_) > 0;
    bool dataRoamingEnabled = dataSwitchSettings_->IsUserDataRoamingOn();
    if (roamingState && !dataRoamingEnabled) {
        isAllowActiveData = false;
    } else if (isMmsApn || isBipApn) {
        isAllowActiveData = true;
    }
    if (isEmergencyApn) {
        isAllowActiveData = true;
    }

#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (TELEPHONY_EXT_WRAPPER.isApnAllowedActive_) {
        std::string apnHolderType = apnHolder->GetApnType();
        isAllowActiveData =
            TELEPHONY_EXT_WRAPPER.isApnAllowedActive_(slotId_, apnHolderType.c_str(), isAllowActiveData);
    }
#endif

    if (!isAllowActiveData) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_ROAMING_SWITCH_OFF_AND_ROAMING, "Data roaming is not on and is roaming");
        TELEPHONY_LOGD("Slot%{public}d: AllowActiveData:%{public}d lastCallState_:%{public}d", slotId_,
            isAllowActiveData, lastCallState_);
        return false;
    }
    if (IsRestrictedMode()) {
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_CALL_AND_DATA_NOT_CONCURRENCY,
            "CS call and data are not allowed concurrency");
        TELEPHONY_LOGD("Slot%{public}d: AllowActiveData:%{public}d lastCallState_:%{public}d", slotId_,
            isAllowActiveData, lastCallState_);
        return false;
    }
    return true;
}

bool CellularDataHandler::CheckApnState(sptr<ApnHolder> &apnHolder)
{
    if (apnManager_ == nullptr || apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or apnManager_ is null", slotId_);
        return false;
    }
    if (apnHolder->GetApnState() == PROFILE_STATE_DISCONNECTING &&
        !HasInnerEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION)) {
        TELEPHONY_LOGI("Slot%{public}d: APN holder is disconnecting", slotId_);
        int32_t id = apnManager_->FindApnIdByApnName(apnHolder->GetApnType());
        SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, id, ESTABLISH_DATA_CONNECTION_DELAY);
        return false;
    }
    if (apnHolder->GetApnState() == PROFILE_STATE_RETRYING) {
        TELEPHONY_LOGI("during retry, check state fail");
        return false;
    }
    if (apnHolder->GetApnState() == PROFILE_STATE_FAILED) {
        apnHolder->SetApnState(PROFILE_STATE_IDLE);
    }
    if (apnHolder->GetApnState() != PROFILE_STATE_IDLE) {
        TELEPHONY_LOGD("Slot%{public}d: APN holder is not idle, apn state is %{public}d",
            slotId_, apnHolder->GetApnState());
        return false;
    }
    std::vector<sptr<ApnItem>> matchedApns = apnManager_->FilterMatchedApns(apnHolder->GetApnType(), slotId_);
    if (matchedApns.empty()) {
        TELEPHONY_LOGE("Slot%{public}d: AttemptEstablishDataConnection:matchedApns is empty", slotId_);
        return false;
    }
    apnHolder->SetAllMatchedApns(matchedApns);
    return true;
}

void CellularDataHandler::AttemptEstablishDataConnection(sptr<ApnHolder> &apnHolder)
{
    if ((airplaneObserver_ != nullptr) && (airplaneObserver_->IsAirplaneModeOn())) {
        return;
    }
    if (!CheckCellularDataSlotId(apnHolder) || !CheckAttachAndSimState(apnHolder) || !CheckRoamingState(apnHolder)) {
        return;
    }
    DelayedSingleton<CellularDataHiSysEvent>::GetInstance()->SetCellularDataActivateStartTime();
    StartTrace(HITRACE_TAG_OHOS, "ActivateCellularData");
    if (!CheckApnState(apnHolder)) {
        FinishTrace(HITRACE_TAG_OHOS);
        return;
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    coreInner.GetPsRadioTech(slotId_, radioTech);
    if (!EstablishDataConnection(apnHolder, radioTech)) {
        TELEPHONY_LOGE("Slot%{public}d: Establish data connection fail", slotId_);
    }
    FinishTrace(HITRACE_TAG_OHOS);
    DelayedSingleton<CellularDataHiSysEvent>::GetInstance()->JudgingDataActivateTimeOut(slotId_, SWITCH_ON);
}

std::shared_ptr<CellularDataStateMachine> CellularDataHandler::FindIdleCellularDataConnection() const
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager_ is null", slotId_);
        return nullptr;
    }
    std::vector<std::shared_ptr<CellularDataStateMachine>> allMachines = connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &connect : allMachines) {
        if (connect == nullptr || apnManager_ == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: CellularDataHandler:stateMachine or apnManager_ is null", slotId_);
            return nullptr;
        }
        if (connect->IsInactiveState() && apnManager_->IsDataConnectionNotUsed(connect)) {
            return connect;
        }
    }
    return nullptr;
}

std::shared_ptr<CellularDataStateMachine> CellularDataHandler::CreateCellularDataConnect()
{
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine = std::make_shared<CellularDataStateMachine>(
        connectionManager_, std::static_pointer_cast<TelEventHandler>(shared_from_this()));
    if (cellularDataStateMachine == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataStateMachine is null", slotId_);
        return nullptr;
    }
    return cellularDataStateMachine;
}

bool CellularDataHandler::EstablishDataConnection(sptr<ApnHolder> &apnHolder, int32_t radioTech)
{
    int32_t profileId = GetCurrentApnId();
    sptr<ApnItem> apnItem = apnManager_->GetApnItemById(profileId);
    if (apnItem == nullptr) {
        TELEPHONY_LOGD("profileId: %{public}d: apnItem is null", profileId);
        apnItem = apnHolder->GetNextRetryApn();
    }
    if (apnItem == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnItem is null", slotId_);
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> cellularDataStateMachine = nullptr;
    if ((apnHolder->GetApnType() != DATA_CONTEXT_ROLE_DEFAULT) &&
        (apnHolder->GetApnType() != DATA_CONTEXT_ROLE_INTERNAL_DEFAULT)) {
        cellularDataStateMachine = CheckForCompatibleDataConnection(apnHolder);
        if (cellularDataStateMachine != nullptr) {
            sptr<ApnItem> dcApnItem = cellularDataStateMachine->GetApnItem();
            if (dcApnItem != nullptr) {
                apnItem = dcApnItem;
            }
        }
    }
    if (cellularDataStateMachine == nullptr) {
        if (IsSingleConnectionEnabled(radioTech)) {
            if (HasAnyHigherPriorityConnection(apnHolder)) {
                TELEPHONY_LOGE("Slot%{public}d: has higher priority connection", slotId_);
                return false;
            }
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
                apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
                ClearAllConnections(DisConnectionReason::REASON_CHANGE_CONNECTION);
                return false;
            }
        }
        cellularDataStateMachine = FindIdleCellularDataConnection();
        if (cellularDataStateMachine == nullptr) {
            cellularDataStateMachine = CreateCellularDataConnect();
            if (cellularDataStateMachine == nullptr) {
                TELEPHONY_LOGE("Slot%{public}d: cellularDataStateMachine is null", slotId_);
                return false;
            }
            cellularDataStateMachine->Init();
            if (connectionManager_ == nullptr) {
                TELEPHONY_LOGE("Slot%{public}d: connectionManager_ is null", slotId_);
                return false;
            }
            connectionManager_->AddConnectionStateMachine(cellularDataStateMachine);
        }
    } else {
        return HandleCompatibleDataConnection(cellularDataStateMachine, apnHolder);
    }
    cellularDataStateMachine->SetCapability(apnHolder->GetCapability());
    apnHolder->SetCurrentApn(apnItem);
    apnHolder->SetApnState(PROFILE_STATE_CONNECTING);
    CellularDataHiSysEvent::WriteDataConnectStateBehaviorEvent(slotId_, apnHolder->GetApnType(),
        apnHolder->GetCapability(), static_cast<int32_t>(PROFILE_STATE_CONNECTING));
    apnHolder->SetCellularDataStateMachine(cellularDataStateMachine);
    bool roamingState = CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0;
    bool userDataRoaming = dataSwitchSettings_->IsUserDataRoamingOn();
    UpdateCellularDataConnectState(apnHolder->GetApnType());
    std::unique_ptr<DataConnectionParams> object = std::make_unique<DataConnectionParams>(
        apnHolder, apnItem->attr_.profileId_, radioTech, roamingState, userDataRoaming, true);
    TELEPHONY_LOGI("Slot%{public}d: MSG_SM_CONNECT profileId:%{public}d type:%{public}s networkType:%{public}d",
        slotId_, apnItem->attr_.profileId_, apnHolder->GetApnType().c_str(), radioTech);
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_SM_CONNECT, object);
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    cellularDataStateMachine->SendEvent(event);
    SetApnActivateStart(apnHolder->GetApnType());
    return true;
}

bool CellularDataHandler::HandleCompatibleDataConnection(
    std::shared_ptr<CellularDataStateMachine> stateMachine, const sptr<ApnHolder> &apnHolder)
{
    TELEPHONY_LOGI("Enter HandleCompatibleDataConnection");
    int32_t oldApnId = ApnManager::FindApnIdByCapability(stateMachine->GetCapability());
    int32_t newApnId = ApnManager::FindApnIdByCapability(apnHolder->GetCapability());
    apnHolder->ReleaseAllCellularData();
    if (stateMachine->IsActiveState()) {
        TELEPHONY_LOGI("set reuse apnId[%{public}d] for apnId[%{public}d]", newApnId, oldApnId);
        stateMachine->SetReuseApnCap(apnHolder->GetCapability());
        stateMachine->SetIfReuseSupplierId(true);
        return true;
    }
    SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, newApnId, ESTABLISH_DATA_CONNECTION_DELAY);
    return false;
}

void CellularDataHandler::EstablishDataConnectionComplete(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    std::shared_ptr<SetupDataCallResultInfo> resultInfo = event->GetSharedObject<SetupDataCallResultInfo>();
    if ((resultInfo != nullptr) && (apnManager_ != nullptr)) {
        TELEPHONY_LOGI("EstablishDataConnectionComplete reason: %{public}d, flag: %{public}d",
            resultInfo->reason, resultInfo->flag);
        SetApnActivateEnd(resultInfo);
        sptr<ApnHolder> apnHolder = apnManager_->GetApnHolder(apnManager_->FindApnNameByApnId(resultInfo->flag));
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: flag:%{public}d complete apnHolder is null", slotId_, resultInfo->flag);
            return;
        }
        apnHolder->SetApnState(PROFILE_STATE_CONNECTED);
        CellularDataHiSysEvent::WriteDataConnectStateBehaviorEvent(slotId_, apnHolder->GetApnType(),
            apnHolder->GetCapability(), static_cast<int32_t>(PROFILE_STATE_CONNECTED));
        apnHolder->InitialApnRetryCount();
        if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_DEFAULT) {
            TELEPHONY_LOGI("default apn has connected, to setup internal_default apn");
            SendEvent(CellularDataEventCode::MSG_RETRY_TO_SETUP_DATACALL, DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID, 0);
        }
        DataConnCompleteUpdateState(apnHolder, resultInfo);
    }
}

void CellularDataHandler::DataConnCompleteUpdateState(const sptr<ApnHolder> &apnHolder,
    const std::shared_ptr<SetupDataCallResultInfo> &resultInfo)
{
    std::shared_ptr<CellularDataStateMachine> stateMachine = apnHolder->GetCellularDataStateMachine();
    if (stateMachine != nullptr) {
        std::string proxyIpAddress = "";
        sptr<ApnItem> attachApn = apnManager_->GetRilAttachApn();
        if (attachApn != nullptr) {
            proxyIpAddress = attachApn->attr_.proxyIpAddress_;
        }
        stateMachine->UpdateHttpProxy(proxyIpAddress);
        stateMachine->UpdateNetworkInfo(*resultInfo);
    } else {
        apnHolder->SetApnState(PROFILE_STATE_IDLE);
        TELEPHONY_LOGE(
            "Slot%{public}d:update network info stateMachine(%{public}d) is null", slotId_, resultInfo->flag);
    }
    if (connectionManager_ != nullptr) {
        connectionManager_->StartStallDetectionTimer();
        connectionManager_->BeginNetStatistics();
    }
    if (!physicalConnectionActiveState_) {
        physicalConnectionActiveState_ = true;
        CoreManagerInner::GetInstance().DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
    }
    if (incallDataStateMachine_ != nullptr) {
        InnerEvent::Pointer incallEvent = InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_CONNECTED);
        incallDataStateMachine_->SendEvent(incallEvent);
    }
    UpdateCellularDataConnectState(apnHolder->GetApnType());
    if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_DEFAULT) {
        TELEPHONY_LOGI("update default apn info");
        UpdateApnInfo(apnHolder->GetCurrentApn()->attr_.profileId_);
    }
    if (apnHolder->IsMmsType()) {
        RemoveEvent(CellularDataEventCode::MSG_RESUME_DATA_PERMITTED_TIMEOUT);
    }
}

std::shared_ptr<DataShare::DataShareHelper> CellularDataHandler::CreatorDataShareHelper()
{
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        TELEPHONY_LOGE("saManager is nullptr.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        TELEPHONY_LOGE("remoteObj is nullptr.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, CELLULAR_DATA_RDB_URI);
}

bool CellularDataHandler::GetCurrentDataShareApnInfo(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    const int32_t simId, int32_t &profileIdValue)
{
    Uri preferApnUri(std::string(CELLULAR_DATA_RDB_PREFER) + "?simId=" + std::to_string(simId));
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet =
        dataShareHelper->Query(preferApnUri, predicates, columns);
    if (resultSet == nullptr) {
        TELEPHONY_LOGI("Query CurrentDataShareApnInfo resultSet is nullptr.");
        return false;
    }
    int count = 0;
    resultSet->GetRowCount(count);
    if (count <= 0) {
        TELEPHONY_LOGI("GetRowCount is NULL.");
        resultSet->Close();
        return false;
    }
    int columnIndex = 0;
    resultSet->GoToFirstRow();
    resultSet->GetColumnIndex(PdpProfileData::PROFILE_ID, columnIndex);
    resultSet->GetInt(columnIndex, profileIdValue);
    resultSet->Close();
    return true;
}

void CellularDataHandler::UpdateApnInfo(const int32_t profileId)
{
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreatorDataShareHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is nullptr.");
        return;
    }
    int32_t profileIdValue = 0;
    if (!GetCurrentDataShareApnInfo(dataShareHelper, simId, profileIdValue)) {
        TELEPHONY_LOGE("GetCurrentDataShareApnInfo fail.");
        dataShareHelper->Release();
        return;
    }
    if (profileIdValue != profileId) {
        DataShare::DataSharePredicates predicates;
        DataShare::DataShareValuesBucket values;
        double profileIdAsDouble = static_cast<double>(profileId);
        double simIdAsDouble = static_cast<double>(simId);
        values.Put(PdpProfileData::PROFILE_ID, profileIdAsDouble);
        values.Put(PdpProfileData::SIM_ID, simIdAsDouble);
        Uri uri(CELLULAR_DATA_RDB_PREFER);
        int32_t result = dataShareHelper->Update(uri, predicates, values);
        if (result < TELEPHONY_ERR_SUCCESS) {
            TELEPHONY_LOGE("UpdateApnInfo fail! result:%{public}d", result);
        }
    }
    dataShareHelper->Release();
}

int32_t CellularDataHandler::GetCurrentApnId()
{
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return 0;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreatorDataShareHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is nullptr.");
        return 0;
    }
    int32_t profileIdValue = 0;
    if (!GetCurrentDataShareApnInfo(dataShareHelper, simId, profileIdValue)) {
        TELEPHONY_LOGE("GetCurrentDataShareApnInfo fail.");
    }
    dataShareHelper->Release();
    return profileIdValue;
}

int32_t CellularDataHandler::GetSlotId() const
{
    return slotId_;
}

void CellularDataHandler::DisconnectDataComplete(const InnerEvent::Pointer &event)
{
    if (event == nullptr || apnManager_ == nullptr || connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event or apnManager or connectionManager_ is null", slotId_);
        return;
    }
    auto netInfo = event->GetSharedObject<SetupDataCallResultInfo>();
    if (netInfo == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: netInfo is null", slotId_);
        return;
    }
    int32_t apnId = netInfo->flag;
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(apnId);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null, apnId is %{public}d", slotId_, apnId);
        return;
    }
    DisConnectionReason reason = ConnectionRetryPolicy::ConvertPdpErrorToDisconnReason(netInfo->reason);
    auto stateMachine = apnHolder->GetCellularDataStateMachine();
    if (stateMachine == nullptr) {
        apnHolder->SetApnState(PROFILE_STATE_IDLE);
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    stateMachine->UpdateNetworkInfo(*netInfo);
    connectionManager_->RemoveActiveConnectionByCid(stateMachine->GetCid());
    apnHolder->SetCellularDataStateMachine(nullptr);
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    CellularDataHiSysEvent::WriteDataConnectStateBehaviorEvent(slotId_, apnHolder->GetApnType(),
        apnHolder->GetCapability(), static_cast<int32_t>(PROFILE_STATE_IDLE));
    UpdateCellularDataConnectState(apnHolder->GetApnType());
    UpdatePhysicalConnectionState(connectionManager_->isNoActiveConnection());
    if (apnHolder->IsDataCallEnabled()) {
        RetryOrClearConnection(apnHolder, reason, netInfo);
    } else {
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
        NotifyReqCellularData(false);
#endif
    }
    if (!apnManager_->HasAnyConnectedState()) {
        connectionManager_->StopStallDetectionTimer();
        connectionManager_->EndNetStatistics();
        if (incallDataStateMachine_ != nullptr) {
            auto incallEvent = InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DATA_DISCONNECTED);
            incallDataStateMachine_->SendEvent(incallEvent);
        }
    }
    if (reason == DisConnectionReason::REASON_CHANGE_CONNECTION) {
        HandleSortConnection();
    }
    HandleDisconnectDataCompleteForMmsType(apnHolder);
}

void CellularDataHandler::HandleDisconnectDataCompleteForMmsType(sptr<ApnHolder> &apnHolder)
{
    if (apnHolder->IsMmsType()) {
        SetDataPermittedForMms(false);
        RemoveEvent(CellularDataEventCode::MSG_RESUME_DATA_PERMITTED_TIMEOUT);
    }
#ifdef BASE_POWER_IMPROVEMENT
    if (strEnterSubscriber_ != nullptr && strEnterSubscriber_->powerSaveFlag_) {
        strEnterSubscriber_->FinishTelePowerEvent();
        strEnterSubscriber_->powerSaveFlag_ = false;
    }
#endif
}

void CellularDataHandler::RetryOrClearConnection(const sptr<ApnHolder> &apnHolder, DisConnectionReason reason,
    const std::shared_ptr<SetupDataCallResultInfo> &netInfo)
{
    if (apnHolder == nullptr || netInfo == nullptr || apnManager_ == nullptr) {
        return;
    }
    ConnectionRetryPolicy::RestartRadioIfRequired(netInfo->reason, slotId_);
    if (reason == DisConnectionReason::REASON_CLEAR_CONNECTION) {
        TELEPHONY_LOGI("clear connection");
        ClearConnection(apnHolder, reason);
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
        NotifyReqCellularData(false);
#endif
    } else if (reason == DisConnectionReason::REASON_PERMANENT_REJECT) {
        TELEPHONY_LOGI("permannent reject, mark bad and clear connection");
        apnHolder->SetApnBadState(true);
        ClearConnection(apnHolder, DisConnectionReason::REASON_CLEAR_CONNECTION);
    } else if (reason == DisConnectionReason::REASON_RETRY_CONNECTION) {
        apnHolder->SetApnState(PROFILE_STATE_RETRYING);
        RetryScene scene = static_cast<RetryScene>(netInfo->retryScene);
        bool isRetrying = (apnManager_->GetOverallDefaultApnState() == ApnProfileState::PROFILE_STATE_RETRYING);
        int64_t delayTime = apnHolder->GetRetryDelay(netInfo->reason, netInfo->retryTime, scene, isRetrying);
        TELEPHONY_LOGI("cid=%{public}d, cause=%{public}d, suggest=%{public}d, delay=%{public}lld, scene=%{public}d",
            netInfo->cid, netInfo->reason, netInfo->retryTime, static_cast<long long>(delayTime), netInfo->retryScene);
        SendEvent(CellularDataEventCode::MSG_RETRY_TO_SETUP_DATACALL, netInfo->flag, delayTime);
    }
}

void CellularDataHandler::ResumeDataPermittedTimerOut(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("SlotId=%{public}d, ResumeDataPermittedTimerOut", slotId_);
    if (apnManager_ == nullptr) {
        SetDataPermittedForMms(false);
        return;
    }
    auto apnHolder = apnManager_->FindApnHolderById(DataContextRolesId::DATA_CONTEXT_ROLE_MMS_ID);
    if (apnHolder == nullptr || apnHolder->GetApnState() == PROFILE_STATE_IDLE) {
        TELEPHONY_LOGI("SlotId=%{public}d, mms resume data due time out", slotId_);
        SetDataPermittedForMms(false);
    }
}

void CellularDataHandler::RetryToSetupDatacall(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr || apnManager_ == nullptr) {
        return;
    }
    int32_t apnId = event->GetParam();
    auto apnHolder = apnManager_->FindApnHolderById(apnId);
    if (apnHolder == nullptr || apnHolder->GetApnState() != PROFILE_STATE_RETRYING) {
        return;
    }
    TELEPHONY_LOGI("apnId=%{public}d, state=%{public}d", apnId, apnHolder->GetApnState());
    apnHolder->SetApnState(PROFILE_STATE_IDLE);
    SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, apnId, 0);
}

void CellularDataHandler::UpdatePhysicalConnectionState(bool noActiveConnection)
{
    if (noActiveConnection && physicalConnectionActiveState_) {
        physicalConnectionActiveState_ = false;
        CoreManagerInner::GetInstance().DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
    } else if (!noActiveConnection && !physicalConnectionActiveState_) {
        physicalConnectionActiveState_ = true;
        CoreManagerInner::GetInstance().DcPhysicalLinkActiveUpdate(slotId_, physicalConnectionActiveState_);
    }
}

void CellularDataHandler::HandleSortConnection()
{
    ApnProfileState state = apnManager_->GetOverallApnState();
    if (state == PROFILE_STATE_IDLE || state == PROFILE_STATE_FAILED) {
        for (const sptr<ApnHolder> &sortApnHolder : apnManager_->GetSortApnHolder()) {
            if (sortApnHolder->IsDataCallEnabled()) {
                int32_t apnId = apnManager_->FindApnIdByApnName(sortApnHolder->GetApnType());
                TELEPHONY_LOGI("Slot%{public}d: HandleSortConnection the apn type is %{public}s", slotId_,
                    sortApnHolder->GetApnType().c_str());
                SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, apnId, 0);
                break;
            }
        }
    }
}

void CellularDataHandler::MsgEstablishDataConnection(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr || event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or event is null", slotId_);
        return;
    }
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(event->GetParam());
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
        return;
    }
    TELEPHONY_LOGD("Slot%{public}d: APN holder type:%{public}s call:%{public}d", slotId_,
        apnHolder->GetApnType().c_str(), apnHolder->IsDataCallEnabled());
    if (apnHolder->IsDataCallEnabled()) {
        AttemptEstablishDataConnection(apnHolder);
    } else {
        TELEPHONY_LOGI("MsgEstablishDataConnection IsDataCallEnabled is false");
        DisConnectionReason reason = DisConnectionReason::REASON_CHANGE_CONNECTION;
        int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
        CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, radioTech);
        if (!IsSingleConnectionEnabled(radioTech)) {
            reason = DisConnectionReason::REASON_CLEAR_CONNECTION;
        }
        ClearConnection(apnHolder, reason);
    }
}

#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
bool CellularDataHandler::IsSimRequestNetOnVSimEnabled(int32_t reqType, bool isMmsType) const
{
    if (reqType == TYPE_REQUEST_NET) {
        if (slotId_ != CELLULAR_DATA_VSIM_SLOT_ID &&
            TELEPHONY_EXT_WRAPPER.isVSimEnabled_ && TELEPHONY_EXT_WRAPPER.isVSimEnabled_() && !isMmsType &&
            TELEPHONY_EXT_WRAPPER.isVSimInDisableProcess_ && !TELEPHONY_EXT_WRAPPER.isVSimInDisableProcess_()) {
            TELEPHONY_LOGE("Slot%{public}d, VSimEnabled & not mms type", slotId_);
            return true;
        }
    }
    return false;
}

bool CellularDataHandler::NotifyReqCellularData(bool isCellularDataRequested)
{
    if (TELEPHONY_EXT_WRAPPER.dynamicLoadNotifyReqCellularDataStatus_) {
        TELEPHONY_EXT_WRAPPER.dynamicLoadNotifyReqCellularDataStatus_(isCellularDataRequested);
        TELEPHONY_LOGE("NotifyReqCellularData isCellularDataRequested %{public}d", isCellularDataRequested);
        return true;
    }
    return false;
}
#endif

void CellularDataHandler::SetNetRequest(NetRequest &request, const std::unique_ptr<NetRequest> &netRequest)
{
    request.ident = netRequest->ident;
    request.capability = netRequest->capability;
    request.registerType = netRequest->registerType;
    request.bearTypes = netRequest->bearTypes;
    request.uid = netRequest->uid;
}

void CellularDataHandler::SendEstablishDataConnectionEvent(int32_t id)
{
    InnerEvent::Pointer innerEvent = InnerEvent::Get(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, id);
    if (!SendEvent(innerEvent)) {
        TELEPHONY_LOGE("Slot%{public}d: send data connection event failed", slotId_);
    }
}

void CellularDataHandler::ConnectIfNeed(
    const InnerEvent::Pointer &event, sptr<ApnHolder> apnHolder, const NetRequest &request)
{
    if (event == nullptr || apnHolder == nullptr) {
        return;
    }
    if (event->GetParam() == TYPE_REQUEST_NET) {
        TELEPHONY_LOGI("try to activate Cellular");
        apnHolder->RequestCellularData(request);
        int32_t id = ApnManager::FindApnIdByCapability(request.capability);
        SendEstablishDataConnectionEvent(id);
    }
}
#ifdef BASE_POWER_IMPROVEMENT
void CellularDataHandler::SubscribeTelePowerEvent()
{
    if (strEnterSubscriber_ == nullptr) {
        strEnterSubscriber_ = CreateCommonSubscriber(ENTER_STR_TELEPHONY_NOTIFY, CommonEventPriority::FIRST_PRIORITY);
    }
    if (strExitSubscriber_ == nullptr) {
        strExitSubscriber_ = CreateCommonSubscriber(EXIT_STR_TELEPHONY_NOTIFY, CommonEventPriority::THIRD_PRIORITY);
    }

    if (strEnterSubscriber_ != nullptr) {
        bool enterStrSubRet = EventFwk::CommonEventManager::SubscribeCommonEvent(strEnterSubscriber_);
        TELEPHONY_LOGI("subscribeResult = %{public}d", enterStrSubRet);
    }
    if (strExitSubscriber_ != nullptr) {
        bool exitStrSubRet = EventFwk::CommonEventManager::SubscribeCommonEvent(strExitSubscriber_);
        TELEPHONY_LOGI("subscribeResult = %{public}d", exitStrSubRet);
    }
}
#endif

void CellularDataHandler::MsgRequestNetwork(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr || event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ or event is null", slotId_);
        return;
    }
    std::unique_ptr<NetRequest> netRequest = event->GetUniqueObject<NetRequest>();
    if (netRequest == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: netRequest is null", slotId_);
        return;
    }
    NetRequest request;
    SetNetRequest(request, netRequest);
    int32_t id = ApnManager::FindApnIdByCapability(request.capability);
    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is null.", slotId_);
        return;
    }
    WriteEventCellularRequest(request, event->GetParam());
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (TELEPHONY_EXT_WRAPPER.judgeOtherRequestHolding_ &&
        TELEPHONY_EXT_WRAPPER.judgeOtherRequestHolding_(request, apnHolder->GetUidStatus())) {
        ConnectIfNeed(event, apnHolder, request);
        return;
    }
    if (IsSimRequestNetOnVSimEnabled(event->GetParam(), apnHolder->IsMmsType())) {
        return;
    }
#endif

    bool isAllCellularDataAllowed = true;
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    if (TELEPHONY_EXT_WRAPPER.isAllCellularDataAllowed_) {
        isAllCellularDataAllowed =
            TELEPHONY_EXT_WRAPPER.isAllCellularDataAllowed_(request, apnHolder->GetUidStatus());
    }
#endif
    if (isAllCellularDataAllowed) {
        TELEPHONY_LOGD("allow cellular data");
        if (event->GetParam() == TYPE_REQUEST_NET) {
            apnHolder->RequestCellularData(request);
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
            NotifyReqCellularData(true);
#endif
        } else {
            if (apnHolder->IsReqUidsEmpty()) {
                apnHolder->ReleaseAllCellularData();
            }
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
            NotifyReqCellularData(false);
#endif
        }
    } else {
        if (event->GetParam() == TYPE_REQUEST_NET) {
            TELEPHONY_LOGD("not allow reqeust cellular data because of in controled");
            return;
        } else {
            TELEPHONY_LOGI("release all cellular data");
            apnHolder->ReleaseAllCellularData();
        }
    }
    SendEstablishDataConnectionEvent(id);
}

bool CellularDataHandler::WriteEventCellularRequest(NetRequest request, int32_t state)
{
    if (request.capability == NetCap::NET_CAPABILITY_INTERNET &&
        (request.bearTypes & (1ULL << NetBearType::BEARER_CELLULAR)) != 0) {
        CellularDataHiSysEvent::WriteCellularRequestBehaviorEvent(
            request.uid, request.ident, request.registerType, state);
            return true;
    }
    return false;
}

void CellularDataHandler::ProcessEvent(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null!", slotId_);
        return;
    }
    uint32_t eventCode = event->GetInnerEventId();
    std::map<uint32_t, Fun>::iterator it = eventIdMap_.find(eventCode);
    if (it != eventIdMap_.end()) {
        it->second(event);
    }
}

void CellularDataHandler::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    int32_t slotId = want.GetIntParam("slotId", 0);
    TELEPHONY_LOGI("[slot%{public}d] action=%{public}s code=%{public}d", slotId_, action.c_str(), data.GetCode());
    if (EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED == action) {
        if (slotId_ != slotId) {
            return;
        }
        int32_t state = want.GetIntParam("state", CALL_STATUS_UNKNOWN);
        if (state == CALL_STATUS_UNKNOWN) {
            TELEPHONY_LOGE("Slot%{public}d: unknown call state=%{public}d", slotId, state);
            return;
        }
        HandleCallChanged(state);
    } else if (action == CommonEventSupport::COMMON_EVENT_SIM_CARD_DEFAULT_DATA_SUBSCRIPTION_CHANGED) {
        HandleDefaultDataSubscriptionChanged();
    } else if (action == CommonEventSupport::COMMON_EVENT_OPERATOR_CONFIG_CHANGED) {
        if (slotId_ != slotId) {
            return;
        }
        GetConfigurationFor5G();
    } else if (action == CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        HandleScreenStateChanged(true);
    } else if (action == CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        HandleScreenStateChanged(false);
    } else if (action == CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY) {
        RegisterDataSettingObserver();
    } else {
        TELEPHONY_LOGI("Slot%{public}d: action=%{public}s code=%{public}d", slotId_, action.c_str(), data.GetCode());
    }
}

void CellularDataHandler::HandleScreenStateChanged(bool isScreenOn) const
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null!", slotId_);
        return;
    }
    connectionManager_->HandleScreenStateChanged(isScreenOn);
}

bool CellularDataHandler::IsSimStateReadyOrLoaded()
{
    SimState simState = SimState::SIM_STATE_UNKNOWN;
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    coreInner.GetSimState(slotId_, simState);
    return (simState == SimState::SIM_STATE_READY || simState == SimState::SIM_STATE_LOADED);
}

void CellularDataHandler::UpdateCellularDataConnectState(const std::string &apnType)
{
    int32_t networkType = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, networkType);
    if (apnType == DATA_CONTEXT_ROLE_DEFAULT || apnType == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
        ApnProfileState apnState = apnManager_->GetOverallDefaultApnState();
        StateNotification::GetInstance().UpdateCellularDataConnectState(slotId_, apnState, networkType);
    }
}

void CellularDataHandler::HandleSettingSwitchChanged(const InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    bool setting_switch = event->GetParam();
    TELEPHONY_LOGI("Slot%{public}d: setting switch = %{public}d", slotId_, setting_switch);
}

void CellularDataHandler::HandleDBSettingIncallChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    if (incallDataStateMachine_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: incallDataStateMachine_ is null", slotId_);
        return;
    }
    int64_t value = event->GetParam();
    if (value == static_cast<int64_t>(DataSwitchCode::CELLULAR_DATA_ENABLED)) {
        InnerEvent::Pointer incallEvent = InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_ON);
        incallDataStateMachine_->SendEvent(incallEvent);
    } else {
        InnerEvent::Pointer incallEvent = InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_SETTINGS_OFF);
        incallDataStateMachine_->SendEvent(incallEvent);
    }
}

std::shared_ptr<IncallDataStateMachine> CellularDataHandler::CreateIncallDataStateMachine(int32_t callState)
{
    std::shared_ptr<IncallDataStateMachine> incallDataStateMachine = std::make_shared<IncallDataStateMachine>(slotId_,
        std::weak_ptr<TelEventHandler>(std::static_pointer_cast<TelEventHandler>(shared_from_this())), apnManager_);
    if (incallDataStateMachine == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: incallDataStateMachine is null", slotId_);
        return nullptr;
    }
    incallDataStateMachine->Init(callState);
    return incallDataStateMachine;
}

void CellularDataHandler::IncallDataComplete(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: MSG_INCALL_DATA_COMPLETE", slotId_);
    if (incallDataStateMachine_ != nullptr) {
        incallDataStateMachine_->DeInit();
    }
}

void CellularDataHandler::HandleCallChanged(int32_t state)
{
    TELEPHONY_LOGI("Slot%{public}d: lastState:%{public}d, state:%{public}d", slotId_, lastCallState_, state);
    if (lastCallState_ == state) {
        TELEPHONY_LOGE("Slot%{public}d: call state=%{public}d, not changed", slotId_, state);
        return;
    }
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null!", slotId_);
        return;
    }
    lastCallState_ = state;
    connectionManager_->UpdateCallState(state);
    ImsRegInfo voiceInfo;
    CoreManagerInner::GetInstance().GetImsRegStatus(slotId_, ImsServiceType::TYPE_VOICE, voiceInfo);
    ImsRegInfo videoInfo;
    CoreManagerInner::GetInstance().GetImsRegStatus(slotId_, ImsServiceType::TYPE_VIDEO, videoInfo);
    if (voiceInfo.imsRegState == ImsRegState::IMS_REGISTERED || videoInfo.imsRegState == ImsRegState::IMS_REGISTERED
        || (incallDataStateMachine_ != nullptr && incallDataStateMachine_->IsSecondaryActiveState())) {
        HandleImsCallChanged(state);
    } else {
        HandleVoiceCallChanged(state);
    }
}

void CellularDataHandler::HandleImsCallChanged(int32_t state)
{
    if (state == TelCallStatus::CALL_STATUS_DIALING || state == TelCallStatus::CALL_STATUS_INCOMING) {
        if (incallDataStateMachine_ == nullptr) {
            incallDataStateMachine_ = CreateIncallDataStateMachine(state);
        } else {
            if (incallDataStateMachine_->GetCurrentState() == nullptr) {
                incallDataStateMachine_->Init(state);
            }
        }
    }
    if (incallDataStateMachine_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: incallDataStateMachine_ is null!", slotId_);
        return;
    }
    incallDataStateMachine_->UpdateCallState(state);
    if (state == TelCallStatus::CALL_STATUS_DIALING || state == TelCallStatus::CALL_STATUS_INCOMING) {
        InnerEvent::Pointer incallEvent = InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_STARTED);
        incallDataStateMachine_->SendEvent(incallEvent);
    }
    if (state == TelCallStatus::CALL_STATUS_DISCONNECTED || state == TelCallStatus::CALL_STATUS_IDLE) {
        InnerEvent::Pointer incallEvent = InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_CALL_ENDED);
        incallDataStateMachine_->SendEvent(incallEvent);
    }
}

void CellularDataHandler::HandleVoiceCallChanged(int32_t state)
{
    if (apnManager_ == nullptr || connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager or connectionManager is null!", slotId_);
        return;
    }
    // next to check if radio technology support voice and data at same time.
    int32_t psRadioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, psRadioTech);
    bool support = (psRadioTech == static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_GSM));
    if (state != TelCallStatus::CALL_STATUS_IDLE && state != TelCallStatus::CALL_STATUS_DISCONNECTED) {
        if (apnManager_->HasAnyConnectedState() && support) {
            connectionManager_->EndNetStatistics();
            connectionManager_->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
            connectionManager_->StopStallDetectionTimer();
            disconnectionReason_ = DisConnectionReason::REASON_GSM_AND_CALLING_ONLY;
        }
    } else {
        if (apnManager_->HasAnyConnectedState() && support) {
            connectionManager_->StartStallDetectionTimer();
            connectionManager_->BeginNetStatistics();
        }
        disconnectionReason_ = DisConnectionReason::REASON_NORMAL;
        TELEPHONY_LOGI("Slot%{public}d: HandleVoiceCallChanged EstablishAllApnsIfConnectable", slotId_);
        EstablishAllApnsIfConnectable();
    }
    TELEPHONY_LOGI("Slot%{public}d: disconnectionReason_=%{public}d", slotId_, disconnectionReason_);
}

void CellularDataHandler::HandleDefaultDataSubscriptionChanged()
{
    TELEPHONY_LOGI("Slot%{public}d", slotId_);
    if (CheckDataPermittedByDsds()) {
        SetDataPermitted(slotId_, true);
    } else {
        SetDataPermitted(slotId_, false);
    }
    if (dataSwitchSettings_ != nullptr) {
        dataSwitchSettings_->LoadSwitchValue();
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    if (defSlotId == slotId_) {
        SendEvent(CellularDataEventCode::MSG_ESTABLISH_ALL_APNS_IF_CONNECTABLE);
    } else {
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::ReleaseAllNetworkRequest()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ is null", slotId_);
        return;
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        if (apnHolder == nullptr) {
            continue;
        }
        apnHolder->ReleaseAllCellularData();
        apnHolder->ReleaseAllUids();
    }
}

void CellularDataHandler::HandleSimStateChanged()
{
    SimState simState = SimState::SIM_STATE_UNKNOWN;
    CoreManagerInner::GetInstance().GetSimState(slotId_, simState);
    TELEPHONY_LOGI("Slot%{public}d: sim state is :%{public}d", slotId_, simState);
    if (simState == SimState::SIM_STATE_READY) {
        std::u16string iccId;
        CoreManagerInner::GetInstance().GetSimIccId(slotId_, iccId);
        if (lastIccId_ != u"" && lastIccId_ == iccId) {
            EstablishAllApnsIfConnectable();
        }
    } else if (simState != SimState::SIM_STATE_LOADED) {
        isRilApnAttached_ = false;
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        if (simState == SimState::SIM_STATE_NOT_PRESENT) {
            CellularDataNetAgent::GetInstance().UnregisterNetSupplierForSimUpdate(slotId_);
            ReleaseAllNetworkRequest();
            UnRegisterDataSettingObserver();
        }
    }
}

void CellularDataHandler::HandleRecordsChanged()
{
    std::u16string iccId;
    CoreManagerInner::GetInstance().GetSimIccId(slotId_, iccId);
    if (iccId == u"") {
        TELEPHONY_LOGI("iccId nullptr");
        return;
    }
    if (iccId != lastIccId_) {
        if (dataSwitchSettings_ != nullptr) {
            dataSwitchSettings_->SetPolicyDataOn(true);
        }
        lastIccId_ = iccId;
    }
    GetConfigurationFor5G();
    CreateApnItem();
    SetRilAttachApn();
    ClearConnectionsOnUpdateApns(DisConnectionReason::REASON_CHANGE_CONNECTION);
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::HandleSimEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    auto slotId = event->GetParam();
    if (slotId != slotId_) {
        return;
    }
    if (dataSwitchSettings_ != nullptr) {
        dataSwitchSettings_->LoadSwitchValue();
    }
    auto eventId = event->GetInnerEventId();
    TELEPHONY_LOGI("Slot%{public}d, event:%{public}d", slotId_, eventId);
    switch (eventId) {
        case RadioEvent::RADIO_SIM_STATE_CHANGE:
            HandleSimStateChanged();
            break;
        case RadioEvent::RADIO_SIM_RECORDS_LOADED:
            HandleRecordsChanged();
            break;
        case RadioEvent::RADIO_NV_REFRESH_FINISHED: {
            SetRilAttachApn();
            break;
        }
        case RadioEvent::RADIO_SIM_ACCOUNT_LOADED:
            HandleSimAccountLoaded();
            break;
        default:
            break;
    }
}

void CellularDataHandler::HandleEstablishAllApnsIfConnectable(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::HandleSimAccountLoaded()
{
    CellularDataNetAgent::GetInstance().UnregisterNetSupplierForSimUpdate(slotId_);
    bool registerRes = CellularDataNetAgent::GetInstance().RegisterNetSupplier(slotId_);
    if (!registerRes) {
        TELEPHONY_LOGE("Slot%{public}d register supplierid fail", slotId_);
    }
    if (slotId_ == 0) {
        CellularDataNetAgent::GetInstance().UnregisterPolicyCallback();
        CellularDataNetAgent::GetInstance().RegisterPolicyCallback();
    }
    RegisterDataSettingObserver();
    if (dataSwitchSettings_ != nullptr) {
        dataSwitchSettings_->LoadSwitchValue();
    }
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    CreateApnItem();
    if (defSlotId == slotId_) {
        EstablishAllApnsIfConnectable();
        ApnProfileState apnState = apnManager_->GetOverallApnState();
        if (registerRes && apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
            UpdateNetworkInfo();
        }
    } else {
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::CreateApnItem()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ is null", slotId_);
        return;
    }
    int32_t result = 0;
    for (int32_t i = 0; i < DEFAULT_READ_APN_TIME; ++i) {
        result = apnManager_->CreateAllApnItemByDatabase(slotId_);
        if (result != 0) {
            break;
        }
    }
    if (result == 0 && !HasInnerEvent(CellularDataEventCode::MSG_RETRY_TO_CREATE_APN)) {
        if (retryCreateApnTimes_ < APN_CREATE_RETRY_TIMES) {
            retryCreateApnTimes_++;
            auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_RETRY_TO_CREATE_APN);
            SendEvent(event, RETRY_DELAY_TIME);
        } else {
            retryCreateApnTimes_ = 0;
        }
    } else if (result != 0) {
        retryCreateApnTimes_ = 0;
        if (HasInnerEvent(CellularDataEventCode::MSG_RETRY_TO_CREATE_APN)) {
            RemoveEvent(CellularDataEventCode::MSG_RETRY_TO_CREATE_APN);
        }
    }
}

bool CellularDataHandler::HandleApnChanged()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("apnManager is null");
        return false;
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        TELEPHONY_LOGI("Slot%{public}d: apn type:%{public}s state:%{public}d", slotId_, apnHolder->GetApnType().c_str(),
            apnHolder->GetApnState());
    }
    InnerEvent::Pointer event = InnerEvent::Get(CellularDataEventCode::MSG_APN_CHANGED);
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: get apn changed event is null", slotId_);
        return false;
    }
    return SendEvent(event);
}

void CellularDataHandler::HandleApnChanged(const InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ is null", slotId_);
        return;
    }
    CreateApnItem();
    SetRilAttachApn();
    ClearConnectionsOnUpdateApns(DisConnectionReason::REASON_CLEAR_CONNECTION);
    apnManager_->ClearAllApnBad();
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        if (apnHolder == nullptr) {
            continue;
        }
        int32_t id = apnManager_->FindApnIdByApnName(apnHolder->GetApnType());
        if (apnHolder->GetApnState() == PROFILE_STATE_RETRYING) {
            apnHolder->InitialApnRetryCount();
            apnHolder->SetApnState(PROFILE_STATE_IDLE);
            RemoveEvent(CellularDataEventCode::MSG_RETRY_TO_SETUP_DATACALL);
        }
        SendEvent(CellularDataEventCode::MSG_ESTABLISH_DATA_CONNECTION, id, ESTABLISH_DATA_CONNECTION_DELAY);
    }
}

int32_t CellularDataHandler::GetCellularDataFlowType()
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connection manager is null", slotId_);
        return 0;
    }
    return connectionManager_->GetDataFlowType();
}

void CellularDataHandler::HandleRadioStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (apnManager_ == nullptr || event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: radio off or not available apnManager or event is null!", slotId_);
        return;
    }
    std::shared_ptr<Int32Parcel> object = event->GetSharedObject<Int32Parcel>();
    if (object == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: object is nullptr!", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: Radio changed with state: %{public}d", slotId_, object->data);
    switch (object->data) {
        case CORE_SERVICE_POWER_OFF:
        case CORE_SERVICE_POWER_NOT_AVAILABLE: {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            TELEPHONY_LOGI("Slot%{public}d: apn state is %{public}d", slotId_, apnState);
            if (apnState != ApnProfileState::PROFILE_STATE_IDLE) {
                ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
            }
            break;
        }
        case CORE_SERVICE_POWER_ON:
            SetRilLinkBandwidths();
            EstablishAllApnsIfConnectable();
            break;
        default:
            TELEPHONY_LOGI("Slot%{public}d: un-handle state:%{public}d", slotId_, object->data);
            break;
    }
}

void CellularDataHandler::HandleDsdsModeChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null!", slotId_);
        return;
    }
    std::shared_ptr<Int32Parcel> object = event->GetSharedObject<Int32Parcel>();
    if (object == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: object is null!", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: DSDS changed with mode: %{public}d", slotId_, object->data);
    int32_t dsdsMode = DSDS_MODE_V2;
    CoreManagerInner::GetInstance().GetDsdsMode(dsdsMode);
    if (object->data == dsdsMode) {
        TELEPHONY_LOGE("Slot%{public}d: DSDS mode is the same!", slotId_);
        return;
    }
    if (object->data < DSDS_MODE_V2) {
        TELEPHONY_LOGE("Slot%{public}d: DSDS mode is illegal!", slotId_);
        return;
    }
    CoreManagerInner::GetInstance().SetDsdsMode(object->data);
    int32_t defaultSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    int32_t simNum = CoreManagerInner::GetInstance().GetMaxSimCount();
    bool dataEnableStatus = true;
    IsCellularDataEnabled(dataEnableStatus);
    for (int32_t i = 0; i < simNum; ++i) {
        if (defaultSlotId != i && object->data == DSDS_MODE_V2) {
            SetDataPermitted(i, false);
        } else {
            if (dataEnableStatus) {
                SetDataPermitted(i, true);
            }
            DelayedRefSingleton<CellularDataService>::GetInstance().ChangeConnectionForDsds(i, true);
        }
    }
    if (incallDataStateMachine_ != nullptr) {
        InnerEvent::Pointer incallEvent = InnerEvent::Get(CellularDataEventCode::MSG_SM_INCALL_DATA_DSDS_CHANGED);
        incallDataStateMachine_->SendEvent(incallEvent);
    }
}

void CellularDataHandler::ClearConnectionIfRequired()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ is null", slotId_);
        return;
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
            continue;
        }
        ApnProfileState apnState = apnHolder->GetApnState();
        std::string apnType = apnHolder->GetApnType();
        std::vector<sptr<ApnItem>> matchedApns = apnManager_->FilterMatchedApns(apnType, slotId_);
        if (matchedApns.empty()) {
            TELEPHONY_LOGE("Slot%{public}d: matchedApns is empty", slotId_);
            continue;
        }
        bool roamingState = CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0;
        if (!apnHolder->IsSameMatchedApns(matchedApns, roamingState)) {
            apnHolder->SetAllMatchedApns(matchedApns);
            if (apnState != ApnProfileState::PROFILE_STATE_IDLE &&
                apnState != ApnProfileState::PROFILE_STATE_FAILED) {
                TELEPHONY_LOGI("Slot%{public}d: the connection of APN type:%{public}s will be cleared.",
                    slotId_, apnType.c_str());
                ClearConnection(apnHolder, DisConnectionReason::REASON_RETRY_CONNECTION);
            }
        }
    }
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager_ is null", slotId_);
        return;
    }
    if (!apnManager_->HasAnyConnectedState()) {
        connectionManager_->StopStallDetectionTimer();
        connectionManager_->EndNetStatistics();
        ResetDataFlowType();
    }
}

void CellularDataHandler::PsDataRatChanged(const InnerEvent::Pointer &event)
{
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    coreInner.GetPsRadioTech(slotId_, radioTech);
    TELEPHONY_LOGI("Slot%{public}d: radioTech is %{public}d", slotId_, radioTech);
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings is null", slotId_);
        return;
    }
    bool dataEnabled = dataSwitchSettings_->IsUserDataOn();
    if (!dataEnabled) {
        TELEPHONY_LOGE("Slot%{public}d: data enable is close", slotId_);
        return;
    }
    bool attached = coreInner.GetPsRegState(slotId_) == (int32_t)RegServiceState::REG_STATE_IN_SERVICE;
    if (!attached) {
        TELEPHONY_LOGE("Slot%{public}d: attached is false", slotId_);
        return;
    }
    ClearConnectionIfRequired();
    EstablishAllApnsIfConnectable();
}

void CellularDataHandler::SetPolicyDataOn(bool enable)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings is null", slotId_);
        return;
    }
    bool policyDataOn = dataSwitchSettings_->IsPolicyDataOn();
    if (policyDataOn != enable) {
        dataSwitchSettings_->SetPolicyDataOn(enable);
        if (enable) {
            SendEvent(CellularDataEventCode::MSG_ESTABLISH_ALL_APNS_IF_CONNECTABLE);
        } else {
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        }
    }
}

bool CellularDataHandler::IsRestrictedMode() const
{
    int32_t networkType = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, networkType);
    bool support = (networkType == (int32_t)RadioTech::RADIO_TECHNOLOGY_GSM);
    bool inCall = (lastCallState_ != TelCallStatus::CALL_STATUS_IDLE &&
                   lastCallState_ != TelCallStatus::CALL_STATUS_DISCONNECTED);
    TELEPHONY_LOGD("Slot%{public}d: radio technology is gsm only:%{public}d and call is busy:%{public}d", slotId_,
        support, inCall);
    return inCall && support;
}

DisConnectionReason CellularDataHandler::GetDisConnectionReason()
{
    return disconnectionReason_;
}

void CellularDataHandler::SetDataPermitted(int32_t slotId, bool dataPermitted)
{
    TELEPHONY_LOGI("Slot%{public}d: dataPermitted is %{public}d.", slotId, dataPermitted);
    int32_t maxSimCount = CoreManagerInner::GetInstance().GetMaxSimCount();
    if (maxSimCount <= 1) {
        TELEPHONY_LOGE("Slot%{public}d: maxSimCount is: %{public}d", slotId_, maxSimCount);
        return;
    }
    bool hasSimCard = false;
    CoreManagerInner::GetInstance().HasSimCard(slotId, hasSimCard);
    if (!hasSimCard && !IsVSimSlotId(slotId)) {
        TELEPHONY_LOGE("Slot%{public}d: no sim :%{public}d", slotId_, slotId);
        return;
    }
    CoreManagerInner::GetInstance().SetDataPermitted(
        slotId, CellularDataEventCode::MSG_SET_DATA_PERMITTED, dataPermitted, shared_from_this());
}

void CellularDataHandler::SetDataPermittedResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    std::shared_ptr<TelRilResponseInfo<int32_t>> rilInfo = event->GetSharedObject<TelRilResponseInfo<int32_t>>();
    if (rilInfo == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: RadioResponseInfo is null", slotId_);
        return;
    }
    if (rilInfo->errorNo != 0) {
        TELEPHONY_LOGE("Slot%{public}d: SetDataPermitted error", slotId_);
    }
}

bool CellularDataHandler::GetEsmFlagFromOpCfg()
{
    int32_t esmFlagFromOpCfg = ESM_FLAG_INVALID;
    OperatorConfig configsForEsmFlag;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, configsForEsmFlag);
    if (configsForEsmFlag.intValue.find(KEY_PLMN_ESM_FLAG_INT) != configsForEsmFlag.intValue.end()) {
        esmFlagFromOpCfg = configsForEsmFlag.intValue[KEY_PLMN_ESM_FLAG_INT];
    }
    if (esmFlagFromOpCfg < 0 || esmFlagFromOpCfg > 1) {
        TELEPHONY_LOGE("esmFlag value is invalid");
    }
    return (esmFlagFromOpCfg != 0);
}

void CellularDataHandler::SetRilAttachApn()
{
    DataProfile dataProfile;
    if (!GetEsmFlagFromOpCfg()) {
        dataProfile.profileId = 0;
        dataProfile.apn = "";
        dataProfile.protocol = "";
        dataProfile.verType = 0;
        dataProfile.userName = "";
        dataProfile.password = "";
        dataProfile.roamingProtocol = "";
    } else {
        sptr<ApnItem> attachApn = apnManager_->GetRilAttachApn();
        if (attachApn == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: attachApn is null", slotId_);
            return;
        }
        dataProfile.profileId = attachApn->attr_.profileId_;
        dataProfile.apn = attachApn->attr_.apn_;
        dataProfile.protocol = attachApn->attr_.protocol_;
        dataProfile.verType = attachApn->attr_.authType_;
        dataProfile.userName = attachApn->attr_.user_;
        dataProfile.password = attachApn->attr_.password_;
        dataProfile.roamingProtocol = attachApn->attr_.roamingProtocol_;
    }
    TELEPHONY_LOGI("Slot%{public}d: DataProfile profileId = %{public}d", slotId_, dataProfile.profileId);
    CoreManagerInner::GetInstance().SetInitApnInfo(
        slotId_, CellularDataEventCode::MSG_SET_RIL_ATTACH_APN, dataProfile, shared_from_this());
}

void CellularDataHandler::SetRilAttachApnResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: event is null", slotId_);
        return;
    }
    std::shared_ptr<TelRilResponseInfo<int32_t>> rilInfo = event->GetSharedObject<TelRilResponseInfo<int32_t>>();
    if (rilInfo == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: RadioResponseInfo is null", slotId_);
        return;
    }
    if (rilInfo->errorNo != 0) {
        TELEPHONY_LOGE("Slot%{public}d: SetRilAttachApn error", slotId_);
    }
    isRilApnAttached_ = (rilInfo->errorNo == 0);
    TELEPHONY_LOGI("Slot%{public}d: isRilApnAttached_ = %{public}d", slotId_, isRilApnAttached_);
}

bool CellularDataHandler::HasAnyHigherPriorityConnection(const sptr<ApnHolder> &apnHolder)
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return false;
    }
    std::vector<sptr<ApnHolder>> sortApnHolders = apnManager_->GetSortApnHolder();
    if (sortApnHolders.empty()) {
        TELEPHONY_LOGE("Slot%{public}d: SortApnHolder is null", slotId_);
        return false;
    }
    for (const sptr<ApnHolder> &sortApnHolder : sortApnHolders) {
        if (sortApnHolder->GetPriority() > apnHolder->GetPriority()) {
            if (sortApnHolder->IsDataCallEnabled() &&
                (sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTED ||
                    sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_CONNECTING ||
                    sortApnHolder->GetApnState() == ApnProfileState::PROFILE_STATE_DISCONNECTING)) {
                CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId_, SWITCH_ON,
                    CellularDataErrorCode::DATA_ERROR_HAS_HIGHER_PRIORITY_CONNECTION,
                    "There is higher priority connection");
                return true;
            }
        }
    }
    return false;
}

bool CellularDataHandler::HasInternetCapability(const int32_t cid) const
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> activeStateMachine = connectionManager_->GetActiveConnectionByCid(cid);
    if (activeStateMachine == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: get activeStateMachine by cid fail", slotId_);
        return false;
    }
    uint64_t netCapability = activeStateMachine->GetCapability();
    if (netCapability == NetCap::NET_CAPABILITY_INTERNET) {
        return true;
    }
    return false;
}

bool CellularDataHandler::ChangeConnectionForDsds(bool enable)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null.", slotId_);
        return false;
    }
    if (enable) {
        dataSwitchSettings_->SetInternalDataOn(true);
        SendEvent(CellularDataEventCode::MSG_ESTABLISH_ALL_APNS_IF_CONNECTABLE);
    } else {
        dataSwitchSettings_->SetInternalDataOn(false);
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
    return true;
}

void CellularDataHandler::GetConfigurationFor5G()
{
    // get 5G configurations
    unMeteredAllNsaConfig_ = ParseOperatorConfig(u"allmeterednas");
    unMeteredNrNsaMmwaveConfig_ = ParseOperatorConfig(u"meterednrnsammware");
    unMeteredNrNsaSub6Config_ = ParseOperatorConfig(u"meteredNrnsasub6");
    unMeteredAllNrsaConfig_ = ParseOperatorConfig(u"meteredallnrsa");
    unMeteredNrsaMmwaveConfig_ = ParseOperatorConfig(u"meterednrsammware");
    unMeteredNrsaSub6Config_ = ParseOperatorConfig(u"meterednrsasub6");
    unMeteredRoamingConfig_ = ParseOperatorConfig(u"meteredroaming");
    GetDefaultConfiguration();
}

bool CellularDataHandler::ParseOperatorConfig(const std::u16string &configName)
{
    OperatorConfig configsFor5G;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, configsFor5G);
    if (configsFor5G.configValue.count(configName) > 0) {
        std::string flag = Str16ToStr8(configsFor5G.configValue[configName]);
        TELEPHONY_LOGI("Slot%{public}d: parse operator 5G config: %{public}s", slotId_, flag.c_str());
        if (flag == "true") {
            return true;
        }
    }
    return false;
}

void CellularDataHandler::GetSinglePdpEnabledFromOpCfg()
{
    OperatorConfig configsForSinglePdp;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, configsForSinglePdp);
    if (configsForSinglePdp.boolValue.find(KEY_SINGLE_PDP_ENABLED_BOOL) != configsForSinglePdp.boolValue.end()) {
        multipleConnectionsEnabled_ = !configsForSinglePdp.boolValue[KEY_SINGLE_PDP_ENABLED_BOOL];
    }
    return;
}

bool CellularDataHandler::IsSingleConnectionEnabled(int32_t radioTech)
{
    std::vector<int32_t> singlePdpRadio;
    OperatorConfig configsForSinglePdpRadioType;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, configsForSinglePdpRadioType);
    if (configsForSinglePdpRadioType.intArrayValue.count(KEY_SINGLE_PDP_RADIO_TYPE_INT_ARRAY) >0) {
        singlePdpRadio = configsForSinglePdpRadioType.intArrayValue[KEY_SINGLE_PDP_RADIO_TYPE_INT_ARRAY];
    }
    if (singlePdpRadio.empty()) {
        TELEPHONY_LOGI("single pdp radio type array is empty");
    }
    if (std::find(singlePdpRadio.begin(), singlePdpRadio.end(), radioTech) != singlePdpRadio.end()) {
        TELEPHONY_LOGI("radio type array is matched single pdp type");
        multipleConnectionsEnabled_ = false;
        return !multipleConnectionsEnabled_;
    }
    GetSinglePdpEnabledFromOpCfg();
    return !multipleConnectionsEnabled_;
}

void CellularDataHandler::GetDefaultDataEnableConfig()
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null", slotId_);
        return;
    }
    bool dataEnbaled = true;
    int32_t ret = dataSwitchSettings_->QueryAnySimDetectedStatus(
        static_cast<int32_t>(DataSimDetectedCode::SIM_DETECTED_DISABLED));
    const int32_t defSlotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    if (ret == TELEPHONY_ERR_SUCCESS || defSlotId != slotId_) {
        return;
    }
    OperatorConfig config;
    if (ret == TELEPHONY_ERR_DATABASE_READ_EMPTY) {
        CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, config);
        if (config.boolValue.find(KEY_DEFAULT_DATA_ENABLE_BOOL) != config.boolValue.end()) {
            dataEnbaled = config.boolValue[KEY_DEFAULT_DATA_ENABLE_BOOL];
            TELEPHONY_LOGI("Slot%{public}d: OperatorConfig dataEnable_ = %{public}d", slotId_, dataEnbaled);
            dataSwitchSettings_->SetUserDataOn(dataEnbaled);
        }
        dataSwitchSettings_->SetAnySimDetected(static_cast<int32_t>(DataSimDetectedCode::SIM_DETECTED_ENABLED));
    }
}

void CellularDataHandler::GetDefaultDataRoamingConfig()
{
    defaultDataRoamingEnable_ = false;
    OperatorConfig configsForDataRoaming;
    CoreManagerInner::GetInstance().GetOperatorConfigs(slotId_, configsForDataRoaming);
    if (configsForDataRoaming.boolValue.find(KEY_DEFAULT_DATA_ROAMING_BOOL) != configsForDataRoaming.boolValue.end()) {
        defaultDataRoamingEnable_ = configsForDataRoaming.boolValue[KEY_DEFAULT_DATA_ROAMING_BOOL];
        TELEPHONY_LOGI("Slot%{public}d: OperatorConfig defaultDataRoamingEnable_ = %{public}d", slotId_,
            defaultDataRoamingEnable_);
    } else {
        std::string defaultDataRoaming = DEFAULT_DATA_ROAMING + std::to_string(slotId_);
        int32_t dataRoaming = static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED);
        dataRoaming = GetIntParameter(defaultDataRoaming.c_str(), dataRoaming);
        defaultDataRoamingEnable_ =
            (dataRoaming == static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED));
        TELEPHONY_LOGI(
            "Slot%{public}d: defaultDataRoamingEnable_ from prop is %{public}d", slotId_, defaultDataRoamingEnable_);
    }
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null", slotId_);
        return;
    }
    bool dataRoamingEnabled = false;
    int32_t ret = dataSwitchSettings_->QueryUserDataRoamingStatus(dataRoamingEnabled);
    if (ret != TELEPHONY_ERR_SUCCESS && defaultDataRoamingEnable_ != dataSwitchSettings_->IsUserDataRoamingOn()) {
        dataSwitchSettings_->UpdateUserDataRoamingOn(defaultDataRoamingEnable_);
        if (apnManager_ == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: apnManager_ is null", slotId_);
            return;
        }
        bool roamingState = false;
        if (CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0) {
            roamingState = true;
        }
        if (roamingState) {
            ApnProfileState apnState = apnManager_->GetOverallApnState();
            if (apnState == ApnProfileState::PROFILE_STATE_CONNECTING ||
                apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
                ClearAllConnections(DisConnectionReason::REASON_RETRY_CONNECTION);
            }
            EstablishAllApnsIfConnectable();
        } else {
            TELEPHONY_LOGI("Slot%{public}d: Not roaming(%{public}d), not doing anything", slotId_, roamingState);
        }
    }
}

void CellularDataHandler::GetDefaultConfiguration()
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return;
    }
    connectionManager_->GetDefaultBandWidthsConfig();
    connectionManager_->GetDefaultTcpBufferConfig();
    GetDefaultUpLinkThresholdsConfig();
    GetDefaultDownLinkThresholdsConfig();
    defaultMobileMtuConfig_ = CellularDataUtils::GetDefaultMobileMtuConfig();
    TELEPHONY_LOGI("Slot%{public}d: defaultMobileMtuConfig_ = %{public}d", slotId_, defaultMobileMtuConfig_);
    defaultPreferApn_ = CellularDataUtils::GetDefaultPreferApnConfig();
    TELEPHONY_LOGI("Slot%{public}d: defaultPreferApn_ is %{public}d", slotId_, defaultPreferApn_);
    multipleConnectionsEnabled_ = CellularDataUtils::GetDefaultMultipleConnectionsConfig();
    GetSinglePdpEnabledFromOpCfg();
    GetDefaultDataRoamingConfig();
    GetDefaultDataEnableConfig();
    TELEPHONY_LOGI("Slot%{public}d: multipleConnectionsEnabled_ = %{public}d, defaultDataRoamingEnable_ = %{public}d",
        slotId_, multipleConnectionsEnabled_, defaultDataRoamingEnable_);
}

void CellularDataHandler::HandleRadioNrStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: receive event", slotId_);
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(RadioEvent::RADIO_NR_STATE_CHANGED);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::HandleRadioNrFrequencyChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: receive event", slotId_);
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode = InnerEvent::Get(RadioEvent::RADIO_NR_FREQUENCY_CHANGED);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::GetDefaultUpLinkThresholdsConfig()
{
    std::lock_guard<std::mutex> guard(mtx_);
    upLinkThresholds_.clear();
    char upLinkConfig[UP_DOWN_LINK_SIZE] = { 0 };
    GetParameter(CONFIG_UPLINK_THRESHOLDS, CAPACITY_THRESHOLDS_FOR_UPLINK, upLinkConfig, UP_DOWN_LINK_SIZE);
    TELEPHONY_LOGI("Slot%{public}d: upLinkThresholds = %{public}s", slotId_, upLinkConfig);
    upLinkThresholds_ = CellularDataUtils::Split(upLinkConfig, ",");
}

void CellularDataHandler::GetDefaultDownLinkThresholdsConfig()
{
    std::lock_guard<std::mutex> guard(mtx_);
    downLinkThresholds_.clear();
    char downLinkConfig[UP_DOWN_LINK_SIZE] = { 0 };
    GetParameter(CONFIG_DOWNLINK_THRESHOLDS, CAPACITY_THRESHOLDS_FOR_DOWNLINK, downLinkConfig, UP_DOWN_LINK_SIZE);
    TELEPHONY_LOGI("Slot%{public}d: downLinkThresholds_ = %{public}s", slotId_, downLinkConfig);
    downLinkThresholds_ = CellularDataUtils::Split(downLinkConfig, ",");
}

void CellularDataHandler::SetRilLinkBandwidths()
{
    std::lock_guard<std::mutex> guard(mtx_);
    LinkBandwidthRule linkBandwidth;
    CoreManagerInner::GetInstance().GetPsRadioTech(slotId_, linkBandwidth.rat);
    linkBandwidth.delayMs = DELAY_SET_RIL_BANDWIDTH_MS;
    linkBandwidth.delayUplinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    linkBandwidth.delayDownlinkKbps = DELAY_SET_RIL_UP_DOWN_BANDWIDTH_MS;
    for (std::string upLinkThreshold : upLinkThresholds_) {
        linkBandwidth.maximumUplinkKbps.push_back(atoi(upLinkThreshold.c_str()));
    }
    for (std::string downLinkThreshold : downLinkThresholds_) {
        linkBandwidth.maximumDownlinkKbps.push_back(atoi(downLinkThreshold.c_str()));
    }
    CoreManagerInner::GetInstance().SetLinkBandwidthReportingRule(
        slotId_, CellularDataEventCode::MSG_SET_RIL_BANDWIDTH, linkBandwidth, shared_from_this());
}

void CellularDataHandler::HandleDBSettingEnableChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: HandleDBSettingEnableChanged enter.", slotId_);
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null.", slotId_);
        return;
    }
    if (TELEPHONY_EXT_WRAPPER.isVirtualModemConnected_ && TELEPHONY_EXT_WRAPPER.isVirtualModemConnected_()) {
        TELEPHONY_LOGI("dc is connected, do nothing");
        return;
    }
    bool dataEnabled = true;
    dataSwitchSettings_->QueryUserDataStatus(dataEnabled);
    CoreManagerInner &coreInner = CoreManagerInner::GetInstance();
    const int32_t defSlotId = coreInner.GetDefaultCellularDataSlotId();
    std::string dataPolicy = system::GetParameter(PERSIST_EDM_MOBILE_DATA_POLICY, "");
    if (dataEnabled && defSlotId == slotId_) {
        if (dataPolicy == MOBILE_DATA_POLICY_DISALLOW) {
            TELEPHONY_LOGE("Slot%{public}d: policy is disallow, not allow establish all apns", slotId_);
            return;
        }
        EstablishAllApnsIfConnectable();
    } else {
        if (dataPolicy == MOBILE_DATA_POLICY_FORCE_OPEN) {
            TELEPHONY_LOGE("Slot%{public}d: policy is force_open, not allow clear connections", slotId_);
            return;
        }
        ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
    }
}

void CellularDataHandler::HandleDBSettingRoamingChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (dataSwitchSettings_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: dataSwitchSettings_ is null", slotId_);
        return;
    }
    bool dataRoamingEnabled = dataSwitchSettings_->IsUserDataRoamingOn();
    bool roamingState = false;
    if (CoreManagerInner::GetInstance().GetPsRoamingState(slotId_) > 0) {
        roamingState = true;
    }
    TELEPHONY_LOGI("Slot%{public}d: roamingState%{public}d, dataRoamingEnabled%{public}d.",
        slotId_, roamingState, dataRoamingEnabled);
    if (roamingState) {
        if (dataRoamingEnabled) {
            EstablishAllApnsIfConnectable();
        } else {
            ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        }
    }
}

void CellularDataHandler::UnRegisterDataSettingObserver()
{
    if (settingObserver_ == nullptr || roamingObserver_ == nullptr || incallObserver_ == nullptr ||
        cellularDataRdbObserver_ == nullptr || airplaneObserver_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: settingObserver_ or roamingObserver_ or incallObserver_ or "
                       "cellularDataRdbObserver_ or airplaneObserver_ is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: settingHelper is null", slotId_);
        return;
    }
    Uri dataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    settingHelper->UnRegisterSettingsObserver(dataEnableUri, settingObserver_);

    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return;
    }
    Uri dataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    settingHelper->UnRegisterSettingsObserver(dataRoamingUri, roamingObserver_);
    Uri dataIncallUri(CELLULAR_DATA_SETTING_INTELLIGENCE_NETWORK_URI);
    settingHelper->UnRegisterSettingsObserver(dataIncallUri, incallObserver_);
    Uri airplaneUri(CELLULAR_DATA_AIRPLANE_MODE_URI);
    settingHelper->UnRegisterSettingsObserver(airplaneUri, airplaneObserver_);

    std::shared_ptr<CellularDataRdbHelper> cellularDataRdbHelper = CellularDataRdbHelper::GetInstance();
    if (cellularDataRdbHelper == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataRdbHelper is null", slotId_);
        return;
    }
    cellularDataRdbHelper->UnRegisterObserver(cellularDataRdbObserver_);
}

void CellularDataHandler::RegisterDataSettingObserver()
{
    if (settingObserver_ == nullptr || roamingObserver_ == nullptr || incallObserver_ == nullptr ||
        cellularDataRdbObserver_ == nullptr || airplaneObserver_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: settingObserver_ or roamingObserver_ or incallObserver_ or "
                       "cellularDataRdbObserver_ or airplaneObserver_ is null", slotId_);
        return;
    }
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: settingHelper is null", slotId_);
        return;
    }
    Uri dataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    settingHelper->RegisterSettingsObserver(dataEnableUri, settingObserver_);

    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId_);
    if (simId <= INVALID_SIM_ID) {
        TELEPHONY_LOGE("Slot%{public}d: failed due to invalid sim id %{public}d", slotId_, simId);
        return;
    }
    Uri dataRoamingUri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(simId));
    settingHelper->RegisterSettingsObserver(dataRoamingUri, roamingObserver_);
    Uri dataIncallUri(CELLULAR_DATA_SETTING_INTELLIGENCE_NETWORK_URI);
    settingHelper->RegisterSettingsObserver(dataIncallUri, incallObserver_);
    Uri airplaneUri(CELLULAR_DATA_AIRPLANE_MODE_URI);
    settingHelper->RegisterSettingsObserver(airplaneUri, airplaneObserver_);

    std::shared_ptr<CellularDataRdbHelper> cellularDataRdbHelper = CellularDataRdbHelper::GetInstance();
    if (cellularDataRdbHelper == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: cellularDataRdbHelper is null", slotId_);
        return;
    }
    cellularDataRdbHelper->RegisterObserver(cellularDataRdbObserver_);
}

void CellularDataHandler::OnRilAdapterHostDied(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return;
    }
    TELEPHONY_LOGI("Slot%{public}d: receive event", slotId_);
    std::vector<std::shared_ptr<CellularDataStateMachine>> stateMachines =
        connectionManager_->GetAllConnectionMachine();
    std::shared_ptr<SetupDataCallResultInfo> setupDataCallResultInfo = std::make_shared<SetupDataCallResultInfo>();
    for (const std::shared_ptr<CellularDataStateMachine> &cellularDataStateMachine : stateMachines) {
        InnerEvent::Pointer eventCode =
            InnerEvent::Get(CellularDataEventCode::MSG_SM_RIL_ADAPTER_HOST_DIED, setupDataCallResultInfo);
        cellularDataStateMachine->SendEvent(eventCode);
    }
}

void CellularDataHandler::GetDataConnApnAttr(ApnItem::Attribute &apnAttr) const
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: GetDataConnApnAttr:apnManager is null", slotId_);
        return;
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
            continue;
        }
        if (apnHolder->IsDataCallEnabled()) {
            sptr<ApnItem> apnItem = apnHolder->GetCurrentApn();
            if (apnItem == nullptr) {
                continue;
            }
            apnAttr = apnItem->attr_;
            return;
        }
    }
}

std::string CellularDataHandler::GetDataConnIpType() const
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: GetDataConnApnAttr:apnManager is null", slotId_);
        return "";
    }
    for (const sptr<ApnHolder> &apnHolder : apnManager_->GetAllApnHolder()) {
        if (apnHolder == nullptr) {
            TELEPHONY_LOGE("Slot%{public}d: apnHolder is null", slotId_);
            continue;
        }
        if (apnHolder->IsDataCallEnabled()) {
            auto stateMachine = apnHolder->GetCellularDataStateMachine();
            if (stateMachine == nullptr) {
                TELEPHONY_LOGE("Slot%{public}d: stateMachine is null", slotId_);
                continue;
            }
            return stateMachine->GetIpType();
        }
    }
    return "";
}

int32_t CellularDataHandler::GetDataRecoveryState()
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager is null", slotId_);
        return -1;
    }
    return connectionManager_->GetDataRecoveryState();
}

void CellularDataHandler::HandleFactoryReset(const InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: factory reset", slotId_);
    SetCellularDataEnable(true);
    SetCellularDataRoamingEnabled(defaultDataRoamingEnable_);
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager_ is null", slotId_);
        return;
    }
    apnManager_->ResetApns(slotId_);
}

void CellularDataHandler::IsNeedDoRecovery(bool needDoRecovery) const
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: in IsNeedDoRecovery connectionManager_ is null", slotId_);
        return;
    }
    connectionManager_->IsNeedDoRecovery(needDoRecovery);
}

void CellularDataHandler::OnCleanAllDataConnectionsDone(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: receive OnCleanAllDataConnectionsDone event", slotId_);
}

bool CellularDataHandler::IsVSimSlotId(int32_t slotId)
{
    if (TELEPHONY_EXT_WRAPPER.getVSimSlotId_) {
        int vSimSlotId = INVALID_SLOT_ID;
        TELEPHONY_EXT_WRAPPER.getVSimSlotId_(vSimSlotId);
        return vSimSlotId == slotId;
    }
    return false;
}

std::shared_ptr<CellularDataStateMachine> CellularDataHandler::CheckForCompatibleDataConnection(
    sptr<ApnHolder> &apnHolder)
{
    std::shared_ptr<CellularDataStateMachine> potentialDc = nullptr;
    if (apnManager_ == nullptr || connectionManager_ == nullptr) {
        return potentialDc;
    }
    std::vector<sptr<ApnItem>> dunApnList;
    if (apnHolder->GetApnType() == DATA_CONTEXT_ROLE_DUN) {
        apnManager_->FetchDunApns(dunApnList, slotId_);
    }
    auto allDCs = connectionManager_->GetAllConnectionMachine();
    bool isRoaming = false;
    int32_t result = IsCellularDataRoamingEnabled(isRoaming);
    if (result != TELEPHONY_ERR_SUCCESS) {
        isRoaming = false;
    }
    for (const auto& curDc : allDCs) {
        sptr<ApnItem> apnItem = curDc->GetApnItem();
        for (const auto& dunItem : dunApnList) {
            if (!ApnHolder::IsCompatibleApnItem(apnItem, dunItem, isRoaming)) {
                continue;
            }
            if (curDc->IsActiveState()) {
                return curDc;
            } else if (curDc->IsActivatingState()) {
                potentialDc = curDc;
            } else if (curDc->IsDisconnectingState()) {
                if (potentialDc == nullptr) {
                    potentialDc = curDc;
                }
            }
        }
        if (dunApnList.size() == 0 && apnItem->CanDealWithType(apnHolder->GetApnType())) {
            if (curDc->IsActiveState()) {
                return curDc;
            } else if (curDc->IsActivatingState()) {
                potentialDc = curDc;
            } else if (curDc->IsDisconnectingState() && potentialDc == nullptr) {
                potentialDc = curDc;
            }
        }
    }
    return potentialDc;
}

void CellularDataHandler::HandleUpdateNetInfo(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Slot%{public}d: HandleUpdateNetInfo", slotId_);
    if (event == nullptr || apnManager_ == nullptr) {
        TELEPHONY_LOGE("event or apnManager_ is null");
        return;
    }

    std::shared_ptr<SetupDataCallResultInfo> netInfo = event->GetSharedObject<SetupDataCallResultInfo>();
    if (netInfo == nullptr) {
        TELEPHONY_LOGE("info is null");
        return;
    }

    sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(netInfo->flag);
    if (apnHolder == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: flag:%{public}d complete apnHolder is null", slotId_, netInfo->flag);
        return;
    }
    if (apnHolder->GetApnState() != PROFILE_STATE_CONNECTING && apnHolder->GetApnState() != PROFILE_STATE_CONNECTED) {
        TELEPHONY_LOGE("Slot%{public}d: apnHolder is not connecting or connected", slotId_);
        return;
    }
    auto stateMachine = apnHolder->GetCellularDataStateMachine();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return;
    }
    stateMachine->UpdateNetworkInfo(*netInfo);
}

bool CellularDataHandler::IsGsm()
{
    bool isGsm = false;
    CoreManagerInner::GetInstance().IsGsm(slotId_, isGsm);
    return isGsm;
}

bool CellularDataHandler::IsCdma()
{
    bool isCdma = false;
    CoreManagerInner::GetInstance().IsCdma(slotId_, isCdma);
    return isCdma;
}

void CellularDataHandler::ReleaseCellularDataConnection()
{
    int32_t id = ApnManager::FindApnIdByCapability(OHOS::NetManagerStandard::NET_CAPABILITY_INTERNET);
    if (!apnManager_) {
        TELEPHONY_LOGE("apnManager_ is nullptr");
        return;
    }
    OHOS::sptr<ApnHolder> apnHolder = apnManager_->FindApnHolderById(id);
    if (apnHolder->GetUidStatus() == HasSystemUse::HAS) {
        TELEPHONY_LOGI("system using, can not release");
        return;
    }
    ClearConnection(apnHolder, DisConnectionReason::REASON_CLEAR_CONNECTION);
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    NotifyReqCellularData(false);
#endif
}

bool CellularDataHandler::UpdateNetworkInfo()
{
    if (connectionManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: connectionManager_ is null", slotId_);
        return false;
    }
    auto stateMachines = connectionManager_->GetAllConnectionMachine();
    for (const std::shared_ptr<CellularDataStateMachine> &cellularDataStateMachine : stateMachines) {
        auto eventCode = InnerEvent::Get(CellularDataEventCode::MSG_SM_UPDATE_NETWORK_INFO);
        cellularDataStateMachine->SendEvent(eventCode);
    }
    return true;
}

bool CellularDataHandler::IsSupportDunApn()
{
    if (apnManager_ == nullptr) {
        TELEPHONY_LOGE("Slot%{public}d: apnManager is null", slotId_);
        return false;
    }
    std::vector<sptr<ApnItem>> dunApnList;
    apnManager_->FetchDunApns(dunApnList, slotId_);
    ApnProfileState apnState = apnManager_->GetOverallDefaultApnState();
    TELEPHONY_LOGI("Slot%{public}d: IsSupportDun=%{public}d, apnState=%{public}d", slotId_, !dunApnList.empty(),
        apnState);
    return (!dunApnList.empty() && apnState == ApnProfileState::PROFILE_STATE_CONNECTED);
}

ApnActivateReportInfo CellularDataHandler::GetDefaultActReportInfo()
{
    return GetApnActReportInfo(DATA_CONTEXT_ROLE_DEFAULT_ID);
}

ApnActivateReportInfo CellularDataHandler::GetInternalActReportInfo()
{
    return GetApnActReportInfo(DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID);
}

ApnActivateReportInfo CellularDataHandler::GetApnActReportInfo(uint32_t apnId)
{
    struct ApnActivateReportInfo info;
    uint32_t totalDuration = 0;
    uint32_t totalActTimes = 0;
    uint32_t totalActSuccTimes = 0;
    uint32_t topReason = 0;
    uint32_t topReasonCnt = 0;
    std::map<uint32_t, uint32_t> errorMap;
    std::lock_guard<std::mutex> lock(apnActivateListMutex_);
    EraseApnActivateList();
    for (uint32_t i = 0; i < apnActivateChrList_.size(); i++) {
        ApnActivateInfo info = apnActivateChrList_[i];
        if (info.apnId != apnId) {
            continue;
        }
        totalDuration += info.duration;
        totalActTimes++;
        if (info.reason == 0) {
            totalActSuccTimes++;
            continue;
        }
        if (errorMap.find(info.reason) != errorMap.end()) {
            errorMap[info.reason] = errorMap[info.reason]+1;
        } else {
            errorMap[info.reason] = 1;
        }
    }
    apnActivateListMutex_.unlock();
    info.actTimes = totalActTimes;
    info.actSuccTimes = totalActSuccTimes;
    info.averDuration = totalActSuccTimes == 0 ? 0 : totalDuration / totalActSuccTimes;
    std::map<uint32_t, uint32_t>::iterator iter;
    for (iter = errorMap.begin(); iter != errorMap.end(); ++iter) {
        if (iter->second > topReasonCnt) {
            topReason = iter->first;
            topReasonCnt = iter->second;
        }
    }
    info.topReason = topReason;
    TELEPHONY_LOGI("GetApnActReportInfo,%{public}d,%{public}d,%{public}d,%{public}d,%{public}d,",
        totalDuration, totalActTimes, totalActSuccTimes, topReason, topReasonCnt);
    return info;
}

void CellularDataHandler::SetApnActivateStart(const std::string &apnType)
{
    uint64_t currentTime = GetCurTime();
    if (apnType == DATA_CONTEXT_ROLE_DEFAULT) {
        defaultApnActTime_ = currentTime;
    }
    if (apnType == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT) {
        internalApnActTime_ = currentTime;
    }
}

void CellularDataHandler::SetApnActivateEnd(const std::shared_ptr<SetupDataCallResultInfo> &resultInfo)
{
    struct ApnActivateInfo info;
    info.actSuccTime = GetCurTime();
    info.reason = resultInfo->reason;
    info.apnId = resultInfo->flag;
    if (resultInfo->flag == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT_ID
        && internalApnActTime_ != 0) {
        info.duration = info.actSuccTime - internalApnActTime_;
    }
    if (resultInfo->flag == DATA_CONTEXT_ROLE_DEFAULT_ID
        && defaultApnActTime_ != 0) {
        info.duration = info.actSuccTime - defaultApnActTime_;
    }
    std::lock_guard<std::mutex> lock(apnActivateListMutex_);
    EraseApnActivateList();
    apnActivateChrList_.push_back(info);
    apnActivateListMutex_.unlock();
}

void CellularDataHandler::EraseApnActivateList()
{
    int64_t currentTime = GetCurTime();
    for (std::vector<ApnActivateInfo>::iterator iter = apnActivateChrList_.begin();
        iter != apnActivateChrList_.end();) {
        if ((currentTime - iter->actSuccTime) > KEEP_APN_ACTIVATE_PERIOD) {
            apnActivateChrList_.erase(iter);
        } else {
            iter++;
        }
    }
}

int64_t CellularDataHandler::GetCurTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
        .time_since_epoch()).count();
}
#ifdef BASE_POWER_IMPROVEMENT
std::shared_ptr<CellularDataPowerSaveModeSubscriber> CellularDataHandler::CreateCommonSubscriber(
    const std::string &event, int32_t priority)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(event);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    subscribeInfo.SetPriority(priority);
    subscribeInfo.SetPermission(PERMISSION_STARTUP_COMPLETED);
    std::weak_ptr<CellularDataHandler> handler = std::static_pointer_cast<CellularDataHandler>(shared_from_this());
    return std::make_shared<CellularDataPowerSaveModeSubscriber>(subscribeInfo, handler);
}
#endif
} // namespace Telephony
} // namespace OHOS
