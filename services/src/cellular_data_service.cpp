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

#include "cellular_data_service.h"

#include <cinttypes>

#include "cellular_data_dump_helper.h"
#include "cellular_data_error.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_net_agent.h"
#include "core_manager_inner.h"
#include "data_connection_monitor.h"
#include "net_specifier.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "telephony_ext_wrapper.h"
#include "telephony_common_utils.h"
#include "telephony_log_wrapper.h"
#include "telephony_permission.h"
#include "data_service_ext_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;

bool g_registerResult = SystemAbility::MakeAndRegisterAbility(&DelayedRefSingleton<CellularDataService>::GetInstance());
CellularDataService::CellularDataService()
    : SystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID, true), registerToService_(false),
      state_(ServiceRunningState::STATE_NOT_START)
{}

CellularDataService::~CellularDataService() = default;

void CellularDataService::OnStart()
{
    beginTime_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TELEPHONY_LOGE("CellularDataService has already started.");
        return;
    }
    if (!Init()) {
        TELEPHONY_LOGE("failed to init CellularDataService");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    endTime_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    TELEPHONY_LOGI("start service success.");
}

void CellularDataService::OnStop()
{
    TELEPHONY_LOGI("ready to stop service.");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    UnRegisterAllNetSpecifier();
}

int32_t CellularDataService::Dump(std::int32_t fd, const std::vector<std::u16string> &args)
{
    std::vector<std::string> argsInStr;
    std::string result;
    CellularDataDumpHelper dumpHelper;
    if (fd < 0) {
        TELEPHONY_LOGE("Dump fd invalid");
        return TELEPHONY_ERR_FAIL;
    }
    for (const std::u16string &arg : args) {
        TELEPHONY_LOGI("Dump args: %s", Str16ToStr8(arg).c_str());
        argsInStr.emplace_back(Str16ToStr8(arg));
    }
    if (dumpHelper.Dump(argsInStr, result)) {
        int32_t ret = dprintf(fd, "%s", result.c_str());
        if (ret < 0) {
            TELEPHONY_LOGE("dprintf to dump fd failed");
            return TELEPHONY_ERR_FAIL;
        }
        return TELEPHONY_SUCCESS;
    }
    TELEPHONY_LOGW("dumpHelper failed");
    return TELEPHONY_ERR_FAIL;
}

int32_t CellularDataService::GetIntelligenceSwitchState(bool &switchState)
{
    if (!TelephonyPermission::CheckCallerIsSystemApp()) {
        TELEPHONY_LOGE("Non-system applications use system APIs!");
        return TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API;
    }
    if (!TelephonyPermission::CheckPermission(Permission::GET_TELEPHONY_STATE)) {
        int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(
            slotId, switchState, CellularDataErrorCode::DATA_ERROR_PERMISSION_ERROR, Permission::SET_TELEPHONY_STATE);
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(DEFAULT_SIM_SLOT_ID);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", DEFAULT_SIM_SLOT_ID);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataController->GetIntelligenceSwitchState(switchState);
}

bool CellularDataService::Init()
{
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapper();
#endif
#ifdef OHOS_BUILD_ENABLE_DATA_SERVICE_EXT
    DATA_SERVICE_EXT_WRAPPER.InitDataServiceExtWrapper();
#endif
    InitModule();
    if (!registerToService_) {
        bool ret = Publish(DelayedRefSingleton<CellularDataService>::GetInstance().AsObject());
        if (!ret) {
            TELEPHONY_LOGE("Publish failed!");
            return false;
        }
        registerToService_ = true;
    }
    std::lock_guard<std::mutex> guard(mapLock_);
    for (const std::pair<const int32_t, std::shared_ptr<CellularDataController>> &it : cellularDataControllers_) {
        if (it.second != nullptr) {
            it.second->AsynchronousRegister();
        } else {
            TELEPHONY_LOGE("CellularDataController is null");
        }
    }
    isInitSuccess_ = true;
    return true;
}

int32_t CellularDataService::IsCellularDataEnabled(bool &dataEnabled)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(DEFAULT_SIM_SLOT_ID);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", DEFAULT_SIM_SLOT_ID);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataController->IsCellularDataEnabled(dataEnabled);
}

int32_t CellularDataService::EnableCellularData(bool enable)
{
    if (!TelephonyPermission::CheckCallerIsSystemApp()) {
        TELEPHONY_LOGE("Non-system applications use system APIs!");
        return TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API;
    }
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(
            slotId, enable, CellularDataErrorCode::DATA_ERROR_PERMISSION_ERROR, Permission::SET_TELEPHONY_STATE);
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(DEFAULT_SIM_SLOT_ID);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", DEFAULT_SIM_SLOT_ID);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataController->SetCellularDataEnable(enable);
}

int32_t CellularDataService::EnableIntelligenceSwitch(bool enable)
{
    if (!TelephonyPermission::CheckCallerIsSystemApp()) {
        TELEPHONY_LOGE("Non-system applications use system APIs!");
        return TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API;
    }
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(
            slotId, enable, CellularDataErrorCode::DATA_ERROR_PERMISSION_ERROR, Permission::SET_TELEPHONY_STATE);
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(DEFAULT_SIM_SLOT_ID);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", DEFAULT_SIM_SLOT_ID);
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataController->SetIntelligenceSwitchEnable(enable);
}

int32_t CellularDataService::GetCellularDataState()
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t dataState = CellularDataStateAdapter(cellularDataController->GetCellularDataState());
    DisConnectionReason reason = cellularDataController->GetDisConnectionReason();
    if (reason == DisConnectionReason::REASON_GSM_AND_CALLING_ONLY && cellularDataController->IsRestrictedMode()) {
        dataState = static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED);
    }
    return dataState;
}

int32_t CellularDataService::GetApnState(int32_t slotId, const std::string &apnType)
{
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t apnState = static_cast<int32_t>(cellularDataController->GetCellularDataState(apnType));
    return apnState;
}

int32_t CellularDataService::GetDataRecoveryState()
{
    int32_t state = 0;
    std::lock_guard<std::mutex> guard(mapLock_);
    for (const auto &controller : cellularDataControllers_) {
        auto cellularDataController = controller.second;
        if (cellularDataController == nullptr) {
            continue;
        }
        int32_t curState = cellularDataController->GetDataRecoveryState();
        state = (curState > state) ? curState : state;
    }
    return state;
}

int32_t CellularDataService::IsCellularDataRoamingEnabled(const int32_t slotId, bool &dataRoamingEnabled)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    return cellularDataController->IsCellularDataRoamingEnabled(dataRoamingEnabled);
}

int32_t CellularDataService::EnableCellularDataRoaming(const int32_t slotId, bool enable)
{
    if (!TelephonyPermission::CheckCallerIsSystemApp()) {
        TELEPHONY_LOGE("Non-system applications use system APIs!");
        return TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API;
    }
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return TELEPHONY_ERR_SLOTID_INVALID;
    }
    int32_t result = cellularDataController->SetCellularDataRoamingEnabled(enable);
    if (result == TELEPHONY_ERR_SUCCESS) {
        CellularDataHiSysEvent::WriteRoamingConnectStateBehaviorEvent(enable);
    }
    return result;
}

void CellularDataService::ClearCellularDataControllers()
{
    std::lock_guard<std::mutex> guard(mapLock_);
    cellularDataControllers_.clear();
}

void CellularDataService::InitModule()
{
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    netAgent.ClearNetSupplier();
    ClearCellularDataControllers();
    std::vector<uint64_t> netCapabilities;
    netCapabilities.push_back(NetCap::NET_CAPABILITY_INTERNET);
    netCapabilities.push_back(NetCap::NET_CAPABILITY_MMS);
    netCapabilities.push_back(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT);
    netCapabilities.push_back(NetCap::NET_CAPABILITY_SUPL);
    netCapabilities.push_back(NetCap::NET_CAPABILITY_DUN);
    netCapabilities.push_back(NetCap::NET_CAPABILITY_IA);
    netCapabilities.push_back(NetCap::NET_CAPABILITY_XCAP);
    int32_t simNum = CoreManagerInner::GetInstance().GetMaxSimCount();
    for (int32_t i = 0; i < simNum; ++i) {
        AddNetSupplier(i, netAgent, netCapabilities);
    }
}

void CellularDataService::AddCellularDataControllers(int32_t slotId,
    std::shared_ptr<CellularDataController> cellularDataController)
{
    std::lock_guard<std::mutex> guard(mapLock_);
    cellularDataControllers_.insert(
        std::pair<int32_t, std::shared_ptr<CellularDataController>>(slotId, cellularDataController));
    if (slotId == CELLULAR_DATA_VSIM_SLOT_ID) {
        // The SIM card is registered in the Init function. After the AsynchronousRegister function is invoked,
        // the initialization is successful based on the delay message.
        // The preceding functions need to be manually called because the VSIM initialization is delayed.
        cellularDataController->AsynchronousRegister();
    }
}

void CellularDataService::AddNetSupplier(int32_t slotId, CellularDataNetAgent &netAgent,
    std::vector<uint64_t> &netCapabilities)
{
    std::shared_ptr<CellularDataController> cellularDataController = std::make_shared<CellularDataController>(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("CellularDataService init module failed cellularDataController is null.");
        return;
    }
    AddCellularDataControllers(slotId, cellularDataController);
    for (uint64_t capability : netCapabilities) {
        NetSupplier netSupplier = { 0 };
        netSupplier.supplierId = 0;
        netSupplier.slotId = slotId;
        netSupplier.simId = INVALID_SIM_ID;
        netSupplier.capability = capability;
        netSupplier.regState = SUPPLIER_INVALID_REG_STATE;
        netAgent.AddNetSupplier(netSupplier);
    }
}

int32_t CellularDataService::InitCellularDataController(int32_t slotId)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    if (slotId != CELLULAR_DATA_VSIM_SLOT_ID) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    std::vector<uint64_t> netCapabilities;
    netCapabilities.push_back(NetCap::NET_CAPABILITY_INTERNET);
    AddNetSupplier(slotId, netAgent, netCapabilities);
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataService::ReleaseNet(const NetRequest &request)
{
    std::string requestIdent = request.ident.substr(strlen(IDENT_PREFIX));
    if (!IsValidDecValue(requestIdent)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t simId = std::stoi(requestIdent);
    int32_t slotId = CoreManagerInner::GetInstance().GetSlotId(simId);
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->ReleaseNet(request);
    return static_cast<int32_t>(result ? RequestNetCode::REQUEST_SUCCESS : RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::RemoveUid(const NetRequest &request)
{
    std::string requestIdent = request.ident.substr(strlen(IDENT_PREFIX));
    if (!IsValidDecValue(requestIdent)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t simId = atoi(requestIdent.c_str());
    int32_t slotId = CoreManagerInner::GetInstance().GetSlotId(simId);
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->RemoveUid(request);
    return static_cast<int32_t>(result ? RequestNetCode::REQUEST_SUCCESS : RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::AddUid(const NetRequest &request)
{
    std::string requestIdent = request.ident.substr(strlen(IDENT_PREFIX));
    if (!IsValidDecValue(requestIdent)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t simId = atoi(requestIdent.c_str());
    int32_t slotId = CoreManagerInner::GetInstance().GetSlotId(simId);
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->AddUid(request);
    return static_cast<int32_t>(result ? RequestNetCode::REQUEST_SUCCESS : RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::RequestNet(const NetRequest &request)
{
    std::string requestIdent = request.ident.substr(strlen(IDENT_PREFIX));
    if (!IsValidDecValue(requestIdent)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t simId = std::stoi(requestIdent);
    if (TELEPHONY_EXT_WRAPPER.isCardAllowData_ &&
        !TELEPHONY_EXT_WRAPPER.isCardAllowData_(simId, request.capability)) {
        return static_cast<int32_t>(RequestNetCode::REQUEST_FAILED);
    }
    int32_t slotId = CoreManagerInner::GetInstance().GetSlotId(simId);
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->RequestNet(request);
    return static_cast<int32_t>(result ? RequestNetCode::REQUEST_SUCCESS : RequestNetCode::REQUEST_FAILED);
}

void CellularDataService::DispatchEvent(const int32_t slotId, const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGI("dispatch event slotId invalid");
        return;
    }
    cellularDataController->ProcessEvent(event);
}

void CellularDataService::UnRegisterAllNetSpecifier()
{
    CellularDataNetAgent::GetInstance().UnregisterAllNetSupplier();
    CellularDataNetAgent::GetInstance().UnregisterPolicyCallback();
}

int32_t CellularDataService::HandleApnChanged(const int32_t slotId)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->HandleApnChanged();
    return result ? static_cast<int32_t>(DataRespondCode::SET_SUCCESS)
                  : static_cast<int32_t>(DataRespondCode::SET_FAILED);
}

int32_t CellularDataService::GetDefaultCellularDataSlotId()
{
    return CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
}

int32_t CellularDataService::GetDefaultCellularDataSimId(int32_t &simId)
{
    return CoreManagerInner::GetInstance().GetDefaultCellularDataSimId(simId);
}

int32_t CellularDataService::SetDefaultCellularDataSlotId(const int32_t slotId)
{
    if (!TelephonyPermission::CheckCallerIsSystemApp()) {
        TELEPHONY_LOGE("Non-system applications use system APIs!");
        return TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API;
    }
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    bool hasSim = false;
    CoreManagerInner::GetInstance().HasSimCard(slotId, hasSim);
    if (!hasSim) {
        TELEPHONY_LOGE("has no sim!");
        return TELEPHONY_ERR_NO_SIM_CARD;
    }
    if (!CoreManagerInner::GetInstance().IsSimActive(slotId)) {
        TELEPHONY_LOGE("sim is not active!");
        return TELEPHONY_ERR_SLOTID_INVALID;
    }
    int32_t formerSlotId = GetDefaultCellularDataSlotId();
    if (formerSlotId < 0) {
        TELEPHONY_LOGI("No old card slot id.");
    }
    int32_t result = CoreManagerInner::GetInstance().SetDefaultCellularDataSlotId(slotId);
    if (result != TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("set slot id fail");
        return result;
    }
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataService::GetCellularDataFlowType()
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    DisConnectionReason reason = cellularDataController->GetDisConnectionReason();
    if (reason == DisConnectionReason::REASON_GSM_AND_CALLING_ONLY && cellularDataController->IsRestrictedMode()) {
        return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
    }
    int32_t result = cellularDataController->GetCellularDataFlowType();
    return result;
}

std::string CellularDataService::GetBeginTime()
{
    std::ostringstream oss;
    oss << beginTime_;
    TELEPHONY_LOGI("bindTime :=  %{public}s", oss.str().c_str());
    return oss.str();
}

std::string CellularDataService::GetEndTime()
{
    std::ostringstream oss;
    oss << endTime_;
    TELEPHONY_LOGI("endTime :=  %{public}s", oss.str().c_str());
    return oss.str();
}

std::string CellularDataService::GetCellularDataSlotIdDump()
{
    std::ostringstream oss;
    oss << "slotId:" << GetDefaultCellularDataSlotId();
    return oss.str();
}

std::string CellularDataService::GetStateMachineCurrentStatusDump()
{
    std::ostringstream oss;
    int32_t slotId = GetDefaultCellularDataSlotId();
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        oss << "default slotId: " << slotId;
        return oss.str();
    }
    ApnProfileState statusDefault = cellularDataController->GetCellularDataState(DATA_CONTEXT_ROLE_DEFAULT);
    ApnProfileState statusIms = cellularDataController->GetCellularDataState(DATA_CONTEXT_ROLE_IMS);
    oss << "Default connect state: " << static_cast<int32_t>(statusDefault);
    oss << "Ims connect state:  " << static_cast<int32_t>(statusIms);
    return oss.str();
}

std::string CellularDataService::GetFlowDataInfoDump()
{
    std::ostringstream oss;
    int32_t slotId = GetDefaultCellularDataSlotId();
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        oss << "default slotId: " << slotId;
        return oss.str();
    }
    int32_t dataFlowInfo = cellularDataController->GetCellularDataFlowType();
    oss << "data flow info: " << dataFlowInfo;
    return oss.str();
}

int32_t CellularDataService::StrategySwitch(int32_t slotId, bool enable)
{
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t result = cellularDataController->SetPolicyDataOn(enable);
    if (result == static_cast<int32_t>(DataRespondCode::SET_SUCCESS) && enable) {
        CellularDataHiSysEvent::WriteDataDeactiveBehaviorEvent(slotId, DataDisconnectCause::HIGN_PRIORITY_NETWORK);
    }
    return result;
}

int32_t CellularDataService::HasInternetCapability(const int32_t slotId, const int32_t cid)
{
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->HasInternetCapability(cid);
    return result ? static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)
                  : static_cast<int32_t>(RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::ClearCellularDataConnections(const int32_t slotId)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    return ClearAllConnections(slotId, DisConnectionReason::REASON_CLEAR_CONNECTION);
}

int32_t CellularDataService::ClearAllConnections(const int32_t slotId, DisConnectionReason reason)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->ClearAllConnections(reason);
    return result ? static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)
                  : static_cast<int32_t>(RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::ChangeConnectionForDsds(const int32_t slotId, bool enable)
{
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataController->ChangeConnectionForDsds(enable);
    return result ? static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS)
                  : static_cast<int32_t>(RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::GetServiceRunningState()
{
    return static_cast<int32_t>(state_);
}

int64_t CellularDataService::GetSpendTime()
{
    return endTime_ - beginTime_;
}

int32_t CellularDataService::RegisterSimAccountCallback(const sptr<SimAccountCallback> callback)
{
    return CoreManagerInner::GetInstance().RegisterSimAccountCallback(GetTokenID(), callback);
}

int32_t CellularDataService::UnregisterSimAccountCallback()
{
    return CoreManagerInner::GetInstance().UnregisterSimAccountCallback(GetTokenID());
}

int32_t CellularDataService::GetDataConnApnAttr(int32_t slotId, ApnItem::Attribute &apnAttr)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    cellularDataController->GetDataConnApnAttr(apnAttr);
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataService::GetDataConnIpType(int32_t slotId, std::string &ipType)
{
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    ipType = cellularDataController->GetDataConnIpType();
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataService::IsNeedDoRecovery(int32_t slotId, bool needDoRecovery)
{
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    cellularDataController->IsNeedDoRecovery(needDoRecovery);
    return TELEPHONY_ERR_SUCCESS;
}

std::shared_ptr<CellularDataController> CellularDataService::GetCellularDataController(int32_t slotId)
{
    if (slotId < 0 || !isInitSuccess_) {
        TELEPHONY_LOGE("Invalid slotId or Init is not success. slotId=%{public}d, isInitSuccess=%{public}d",
            slotId, (int32_t)isInitSuccess_);
        return nullptr;
    }
    std::lock_guard<std::mutex> guard(mapLock_);
    std::map<int32_t, std::shared_ptr<CellularDataController>>::const_iterator item =
        cellularDataControllers_.find(slotId);
    if (item == cellularDataControllers_.end() || item->second == nullptr) {
        return nullptr;
    }

    return item->second;
}

int32_t CellularDataService::EstablishAllApnsIfConnectable(const int32_t slotId)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }

    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("slot%{public}d cellularDataControllers is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }

    bool hasSim = false;
    CoreManagerInner::GetInstance().HasSimCard(slotId, hasSim);
    if (!hasSim) {
        TELEPHONY_LOGE("slot%{public}d has no sim", slotId);
        return TELEPHONY_ERR_NO_SIM_CARD;
    }
    if (!CoreManagerInner::GetInstance().IsSimActive(slotId)) {
        TELEPHONY_LOGE("slot%{public}d sim not active", slotId);
        return TELEPHONY_ERR_SLOTID_INVALID;
    }

    bool result = cellularDataController->EstablishAllApnsIfConnectable();
    return result ? TELEPHONY_ERR_SUCCESS : TELEPHONY_ERR_FAIL;
}

int32_t CellularDataService::ReleaseCellularDataConnection(int32_t slotId)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }

    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("slot%{public}d cellularDataControllers is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }

    bool hasSim = false;
    CoreManagerInner::GetInstance().HasSimCard(slotId, hasSim);
    if (!hasSim) {
        TELEPHONY_LOGE("slot%{public}d has no sim", slotId);
        return TELEPHONY_ERR_NO_SIM_CARD;
    }
    if (!CoreManagerInner::GetInstance().IsSimActive(slotId)) {
        TELEPHONY_LOGE("slot%{public}d sim not active", slotId);
        return TELEPHONY_ERR_SLOTID_INVALID;
    }

    return cellularDataController->ReleaseCellularDataConnection() ? TELEPHONY_ERR_SUCCESS : TELEPHONY_ERR_FAIL;
}

int32_t CellularDataService::GetCellularDataSupplierId(int32_t slotId, uint64_t capability, uint32_t &supplierId)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    if (capability < NetCap::NET_CAPABILITY_MMS || capability > NetCap::NET_CAPABILITY_INTERNAL_DEFAULT) {
        TELEPHONY_LOGE("Invalid capability = (%{public}" PRIu64 ")", capability);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    supplierId = CellularDataNetAgent::GetInstance().GetSupplierId(slotId, capability);
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataService::CorrectNetSupplierNoAvailable(int32_t slotId)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    std::shared_ptr<CellularDataController> cellularDataController = GetCellularDataController(slotId);
    if (cellularDataController == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers is null, slotId=%{public}d", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t apnState = static_cast<int32_t>(cellularDataController->GetCellularDataState(DATA_CONTEXT_ROLE_DEFAULT));
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        TELEPHONY_LOGE("Default apn state is connected, do not set available false");
        return TELEPHONY_ERR_FAIL;
    }
    TELEPHONY_LOGI("correct default supplier available is false, apn state = %{public}d", apnState);
    bool result = CellularDataNetAgent::GetInstance().UpdateNetSupplierAvailable(slotId, false);
    return result ? TELEPHONY_ERR_SUCCESS : TELEPHONY_ERR_FAIL;
}

int32_t CellularDataService::GetSupplierRegisterState(uint32_t supplierId, int32_t &regState)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        TELEPHONY_LOGE("Permission denied!");
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    bool result = CellularDataNetAgent::GetInstance().GetSupplierRegState(supplierId, regState);
    return result ? TELEPHONY_ERR_SUCCESS : TELEPHONY_ERR_FAIL;
}
} // namespace Telephony
} // namespace OHOS
