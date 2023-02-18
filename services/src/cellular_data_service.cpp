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

#include "cellular_data_dump_helper.h"
#include "cellular_data_error.h"
#include "cellular_data_hisysevent.h"
#include "cellular_data_net_agent.h"
#include "core_manager_inner.h"
#include "data_connection_monitor.h"
#include "net_specifier.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "telephony_common_utils.h"
#include "telephony_log_wrapper.h"
#include "telephony_permission.h"

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
    TELEPHONY_LOGI("CellularDataService::OnStart start service success.");
}

void CellularDataService::OnStop()
{
    TELEPHONY_LOGI("CellularDataService::OnStop ready to stop service.");
    if (eventLoop_ != nullptr) {
        eventLoop_.reset();
    }
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

bool CellularDataService::Init()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("CellularDataService");
    if (eventLoop_ == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return false;
    }
    InitModule();
    eventLoop_->Run();
    if (!registerToService_) {
        bool ret = Publish(DelayedRefSingleton<CellularDataService>::GetInstance().AsObject());
        if (!ret) {
            TELEPHONY_LOGE("CellularDataService::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
    }
    for (const std::pair<const int32_t, std::shared_ptr<CellularDataController>> &it : cellularDataControllers_) {
        if (it.second != nullptr) {
            it.second->AsynchronousRegister();
        } else {
            TELEPHONY_LOGE("CellularDataController is null");
        }
    }
    return true;
}

int32_t CellularDataService::IsCellularDataEnabled(bool &dataEnabled)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    if (cellularDataControllers_[DEFAULT_SIM_SLOT_ID] == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers_[0] is null");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return cellularDataControllers_[DEFAULT_SIM_SLOT_ID]->IsCellularDataEnabled(dataEnabled);
}

int32_t CellularDataService::EnableCellularData(bool enable)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(
            slotId, enable, CellularDataErrorCode::DATA_ERROR_PERMISSION_ERROR, Permission::SET_TELEPHONY_STATE);
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    int32_t result = TELEPHONY_ERR_SUCCESS;
    for (const std::pair<const int32_t, std::shared_ptr<CellularDataController>> &it : cellularDataControllers_) {
        if (it.second != nullptr) {
            result = it.second->SetCellularDataEnable(enable);
            if (result == TELEPHONY_ERR_SUCCESS) {
                CellularDataHiSysEvent::WriteDataConnectStateBehaviorEvent(enable);
            }
        } else {
            TELEPHONY_LOGE("CellularDataController is null");
        }
    }

    return result;
}

int32_t CellularDataService::GetCellularDataState()
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    std::map<int32_t, std::shared_ptr<CellularDataController>>::const_iterator item =
        cellularDataControllers_.find(slotId);
    if (item == cellularDataControllers_.end() || item->second == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t dataState = CellularDataStateAdapter(item->second->GetCellularDataState());
    DisConnectionReason reason = item->second->GetDisConnectionReason();
    if (reason == DisConnectionReason::REASON_GSM_AND_CALLING_ONLY && item->second->IsRestrictedMode()) {
        dataState = static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED);
    }
    return dataState;
}

int32_t CellularDataService::IsCellularDataRoamingEnabled(const int32_t slotId, bool &dataRoamingEnabled)
{
    if (!TelephonyPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    return cellularDataControllers_[slotId]->IsCellularDataRoamingEnabled(dataRoamingEnabled);
}

int32_t CellularDataService::EnableCellularDataRoaming(const int32_t slotId, bool enable)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
    }
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return TELEPHONY_ERR_SLOTID_INVALID;
    }
    int32_t result = cellularDataControllers_[slotId]->SetCellularDataRoamingEnabled(enable);
    if (result == TELEPHONY_ERR_SUCCESS) {
        CellularDataHiSysEvent::WriteRoamingConnectStateBehaviorEvent(enable);
    }
    return result;
}

void CellularDataService::InitModule()
{
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    netAgent.ClearNetSupplier();
    cellularDataControllers_.clear();
    std::vector<uint64_t> netCapabilities;
    netCapabilities.push_back(NetCap::NET_CAPABILITY_INTERNET);
    netCapabilities.push_back(NetCap::NET_CAPABILITY_MMS);
    int32_t simNum = CoreManagerInner::GetInstance().GetMaxSimCount();
    for (int32_t i = 0; i < simNum; ++i) {
        std::shared_ptr<CellularDataController> cellularDataController =
            std::make_shared<CellularDataController>(eventLoop_, i);
        if (cellularDataController == nullptr) {
            TELEPHONY_LOGE("CellularDataService init module failed cellularDataController is null.");
            continue;
        }
        cellularDataControllers_.insert(
            std::pair<int32_t, std::shared_ptr<CellularDataController>>(i, cellularDataController));
        for (uint64_t capability : netCapabilities) {
            NetSupplier netSupplier = { 0 };
            netSupplier.supplierId = 0;
            netSupplier.slotId = i;
            netSupplier.capability = capability;
            netAgent.AddNetSupplier(netSupplier);
        }
    }
}

bool CellularDataService::CheckParamValid(const int32_t slotId)
{
    if (slotId < 0) {
        return false;
    }
    std::map<int32_t, std::shared_ptr<CellularDataController>>::const_iterator item =
        cellularDataControllers_.find(slotId);
    if (item == cellularDataControllers_.end()) {
        return false;
    }
    if (item->second == nullptr) {
        return false;
    }
    return true;
}

int32_t CellularDataService::ReleaseNet(const NetRequest &request)
{
    std::string requestIdent = request.ident.substr(strlen(IDENT_PREFIX));
    if (!IsValidDecValue(requestIdent)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t simId = std::stoi(requestIdent);
    int32_t slotId = CoreManagerInner::GetInstance().GetSlotId(simId);
    if (!CheckParamValid(slotId)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->ReleaseNet(request);
    return static_cast<int32_t>(result ? RequestNetCode::REQUEST_SUCCESS : RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::RequestNet(const NetRequest &request)
{
    std::string requestIdent = request.ident.substr(strlen(IDENT_PREFIX));
    if (!IsValidDecValue(requestIdent)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t simId = std::stoi(requestIdent);
    int32_t slotId = CoreManagerInner::GetInstance().GetSlotId(simId);
    if (!CheckParamValid(slotId)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->RequestNet(request);
    return static_cast<int32_t>(result ? RequestNetCode::REQUEST_SUCCESS : RequestNetCode::REQUEST_FAILED);
}

void CellularDataService::DispatchEvent(const int32_t slotId, const AppExecFwk::InnerEvent::Pointer &event)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGI("dispatch event slotId invalid");
        return;
    }
    cellularDataControllers_[slotId]->ProcessEvent(event);
}

void CellularDataService::UnRegisterAllNetSpecifier()
{
    CellularDataNetAgent::GetInstance().UnregisterNetSupplier();
    CellularDataNetAgent::GetInstance().UnregisterPolicyCallback();
}

int32_t CellularDataService::HandleApnChanged(const int32_t slotId)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->HandleApnChanged();
    return result ? static_cast<int32_t>(DataRespondCode::SET_SUCCESS)
                  : static_cast<int32_t>(DataRespondCode::SET_FAILED);
}

int32_t CellularDataService::GetDefaultCellularDataSlotId()
{
    return CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
}

int32_t CellularDataService::SetDefaultCellularDataSlotId(const int32_t slotId)
{
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        return TELEPHONY_ERR_PERMISSION_ERR;
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
    if (formerSlotId >= 0 && formerSlotId != slotId) {
        std::map<int32_t, std::shared_ptr<CellularDataController>>::iterator itController =
            cellularDataControllers_.find(formerSlotId);
        if (itController != cellularDataControllers_.end() && (itController->second != nullptr)) {
            itController->second->ClearAllConnections(DisConnectionReason::REASON_CLEAR_CONNECTION);
        } else {
            TELEPHONY_LOGI("Not find old slot[%{public}d] object", formerSlotId);
        }
    }
    int32_t newSlotId = GetDefaultCellularDataSlotId();
    if (formerSlotId != newSlotId) {
        if (CheckParamValid(formerSlotId)) {
            cellularDataControllers_[formerSlotId]->SetDataPermitted(false);
        }
        if (CheckParamValid(newSlotId)) {
            cellularDataControllers_[newSlotId]->SetDataPermitted(true);
        }
    }
    bool dataEnabled = false;
    IsCellularDataEnabled(dataEnabled);
    if (dataEnabled && CheckParamValid(slotId)) {
        cellularDataControllers_[slotId]->EstablishDataConnection();
    }
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataService::GetCellularDataFlowType()
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    std::map<int32_t, std::shared_ptr<CellularDataController>>::const_iterator item =
        cellularDataControllers_.find(slotId);
    if (item == cellularDataControllers_.end() || item->second == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    DisConnectionReason reason = item->second->GetDisConnectionReason();
    if (reason == DisConnectionReason::REASON_GSM_AND_CALLING_ONLY && item->second->IsRestrictedMode()) {
        return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
    }
    int32_t result = item->second->GetCellularDataFlowType();
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
    if (!CheckParamValid(slotId)) {
        oss << "default slotId: " << slotId;
        return oss.str();
    }
    ApnProfileState statusDefault = cellularDataControllers_[slotId]->GetCellularDataState(DATA_CONTEXT_ROLE_DEFAULT);
    ApnProfileState statusIms = cellularDataControllers_[slotId]->GetCellularDataState(DATA_CONTEXT_ROLE_IMS);
    oss << "Default connect state: " << static_cast<int32_t>(statusDefault);
    oss << "Ims connect state:  " << static_cast<int32_t>(statusIms);
    return oss.str();
}

std::string CellularDataService::GetFlowDataInfoDump()
{
    std::ostringstream oss;
    int32_t slotId = GetDefaultCellularDataSlotId();
    if (!CheckParamValid(slotId)) {
        oss << "default slotId: " << slotId;
        return oss.str();
    }
    int32_t dataFlowInfo = cellularDataControllers_[slotId]->GetCellularDataFlowType();
    oss << "data flow info: " << dataFlowInfo;
    return oss.str();
}

int32_t CellularDataService::StrategySwitch(int32_t slotId, bool enable)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t result = cellularDataControllers_[slotId]->SetPolicyDataOn(enable);
    if (result == static_cast<int32_t>(DataRespondCode::SET_SUCCESS) && enable) {
        CellularDataHiSysEvent::WriteDataDeactiveBehaviorEvent(slotId, DataDisconnectCause::HIGN_PRIORITY_NETWORK);
    }
    return result;
}

int32_t CellularDataService::HasInternetCapability(const int32_t slotId, const int32_t cid)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->HasInternetCapability(cid);
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
    std::map<int32_t, std::shared_ptr<CellularDataController>>::const_iterator item =
        cellularDataControllers_.find(slotId);
    if (item == cellularDataControllers_.end() || item->second == nullptr) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = item->second->ClearAllConnections(reason);
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

int32_t CellularDataService::RegisterSimAccountCallback(const sptr<SimAccountCallback> &callback)
{
    return CoreManagerInner::GetInstance().RegisterSimAccountCallback(GetBundleName(), callback);
}

int32_t CellularDataService::UnregisterSimAccountCallback()
{
    return CoreManagerInner::GetInstance().UnregisterSimAccountCallback(GetBundleName());
}
} // namespace Telephony
} // namespace OHOS
