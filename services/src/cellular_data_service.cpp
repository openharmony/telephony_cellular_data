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

#include "string_ex.h"
#include "system_ability_definition.h"

#include "net_conn_constants.h"
#include "net_specifier.h"

#include "cellular_data_error.h"
#include "core_manager.h"
#include "sim_utils.h"
#include "telephony_log_wrapper.h"

#include "cellular_data_net_agent.h"
#include "cellular_data_dump_helper.h"
#include "data_connection_monitor.h"

namespace OHOS {
namespace Telephony {
using namespace NetManagerStandard;

bool g_registerResult =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<CellularDataService>::GetInstance().get());
CellularDataService::CellularDataService()
    : SystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID, true), registerToService_(false),
    state_(ServiceRunningState::STATE_NOT_START)
{}

CellularDataService::~CellularDataService() = default;

void CellularDataService::OnStart()
{
    bindTime_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TELEPHONY_LOGE("CellularDataService has already started.");
        return;
    }
    WaitCoreServiceToInit();
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
    for (const std::u16string& arg :args) {
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

void CellularDataService::WaitCoreServiceToInit()
{
    uint32_t i = 0;
    int slotId = CoreManager::DEFAULT_SLOT_ID;
    for (i = 0; i < CONNECT_MAX_TRY_COUNT; i++) {
        std::shared_ptr<Core> core = CoreManager::GetInstance().getCore(slotId);
        if (core != nullptr && core->IsInitCore()) {
            TELEPHONY_LOGI("CellularDataService connection successful");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CONNECT_SERVICE_WAIT_TIME));
    }
    if (CONNECT_MAX_TRY_COUNT == i) {
        TELEPHONY_LOGI("CellularDataService connection failed");
    }
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
        bool ret = Publish(DelayedSingleton<CellularDataService>::GetInstance().get());
        if (!ret) {
            TELEPHONY_LOGE("CellularDataService::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
    }
    for (const std::pair<const int32_t, std::shared_ptr<CellularDataController>> &it : cellularDataControllers_) {
        it.second->AsynchronousRegister();
    }
    RegisterAllNetSpecifier();
    return true;
}

int32_t CellularDataService::IsCellularDataEnabled()
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->IsCellularDataEnabled();
    return result ? static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED) :
        static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED);
}

int32_t CellularDataService::EnableCellularData(bool enable)
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->SetCellularDataEnable(enable);
    return result ? static_cast<int32_t>(DataRespondCode::SET_SUCCESS) :
        static_cast<int32_t>(DataRespondCode::SET_FAILED);
}

int32_t CellularDataService::GetCellularDataState()
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t dataState = CellularDataStateAdapter(
        cellularDataControllers_[slotId]->GetCellularDataState(DATA_CONTEXT_ROLE_DEFAULT));
    int32_t reason = cellularDataControllers_[slotId]->GetDisConnectionReason();
    if (reason == REASON_GSM_AND_CALLING_ONLY) {
        if (cellularDataControllers_[slotId]->IsRestrictedMode()) {
            dataState = static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED);
        }
    }
    TELEPHONY_LOGI("slotId=%{public}d, dataState=%{public}d", slotId, dataState);
    return dataState;
}

int32_t CellularDataService::IsCellularDataRoamingEnabled(const int32_t slotId)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->IsCellularDataRoamingEnabled();
    return result ? static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED) :
        static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED);
}

int32_t CellularDataService::EnableCellularDataRoaming(const int32_t slotId, bool enable)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->SetCellularDataRoamingEnabled(enable);
    return result ? static_cast<int32_t>(DataRespondCode::SET_SUCCESS) :
        static_cast<int32_t>(DataRespondCode::SET_FAILED);
}

void CellularDataService::InitModule()
{
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    netAgent.ClearNetSupplier();
    cellularDataControllers_.clear();
    std::vector<uint64_t> netCapabilities;
    netCapabilities.push_back(NetCapabilities::NET_CAPABILITIES_INTERNET);
    netCapabilities.push_back(NetCapabilities::NET_CAPABILITIES_MMS);
    int32_t simNum = SimUtils::GetSimNum();
    for (int32_t i = 0; i < simNum; ++i) {
        std::shared_ptr<CellularDataController> cellularDataController =
        std::make_shared<CellularDataController>(eventLoop_, i);
        if (cellularDataController == nullptr) {
            TELEPHONY_LOGE("CellularDataService init module failed cellularDataController is null.");
            continue;
        }
        cellularDataControllers_.insert(
            std::pair<int32_t, std::shared_ptr<CellularDataController>>(i, cellularDataController));
        for (uint64_t capability: netCapabilities) {
            NetSupplier netSupplier = { 0 };
            netSupplier.supplierId = NET_CONN_ERR_INVALID_SUPPLIER_ID;
            netSupplier.slotId = i;
            netSupplier.netType = NetworkType::NET_TYPE_CELLULAR;
            netSupplier.capabilities = capability;
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
    int32_t slotId = std::stoi(request.ident.substr(IDENT_PREFIX.length()));
    if (!CheckParamValid(slotId)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->ReleaseNet(request);
    return static_cast<int32_t>(result ? RequestNetCode::REQUEST_SUCCESS : RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::RequestNet(const NetRequest &request)
{
    int32_t slotId = std::stoi(request.ident.substr(IDENT_PREFIX.length()));
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

void CellularDataService::RegisterAllNetSpecifier()
{
    TELEPHONY_LOGI("CellularDataService %{public}p", this);
    std::map<int32_t, std::shared_ptr<CellularDataController>>::iterator itController
        = cellularDataControllers_.find(CoreManager::DEFAULT_SLOT_ID);
    if (!CellularDataNetAgent::GetInstance().RegisterNetSupplier()) {
        if (itController != cellularDataControllers_.end() && (itController->second != nullptr)) {
            itController->second->SendRegisterNetManagerEvent();
        } else {
            TELEPHONY_LOGE("not register NetSupplier size:%{public}zu", cellularDataControllers_.size());
        }
    }
    if (CellularDataNetAgent::GetInstance().RegisterPolicyCallback() != 0) {
        if (itController != cellularDataControllers_.end() && (itController->second != nullptr)) {
            itController->second->SendRegisterPolicyEvent();
        } else {
            TELEPHONY_LOGE("not register NetPolicy size:%{public}zu", cellularDataControllers_.size());
        }
    }
}

void CellularDataService::UnRegisterAllNetSpecifier()
{
    CellularDataNetAgent::GetInstance().UnregisterNetSupplier();
    CellularDataNetAgent::GetInstance().UnregisterPolicyCallback();
}

int32_t CellularDataService::HandleApnChanged(const int32_t slotId, std::string apns)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t result = cellularDataControllers_[slotId]->HandleApnChanged(apns);
    return result;
}

int32_t CellularDataService::GetDefaultCellularDataSlotId()
{
    return SimUtils::GetDefaultCellularDataSlotId();
}

int32_t CellularDataService::SetDefaultCellularDataSlotId(const int32_t slotId)
{
    if (!CheckParamValid(slotId)) {
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t formerSlotId = GetDefaultCellularDataSlotId();
    TELEPHONY_LOGI("former slot id is :=  %{public}d", formerSlotId);
    if (formerSlotId == TELEPHONY_ERROR) {
        TELEPHONY_LOGI("No old card slot id.");
    }
    int32_t result = SimUtils::SetDefaultCellularDataSlotId(slotId);
    if (result == static_cast<int32_t>(DataRespondCode::SET_SUCCESS)) {
        if (formerSlotId >= 0 && formerSlotId != slotId) {
            cellularDataControllers_[formerSlotId]->ClearAllConnectionsFormerSlot();
        }
        if (IsCellularDataEnabled()) {
            cellularDataControllers_[slotId]->ConnectDataNetWork();
        }
    } else {
        TELEPHONY_LOGE("set slot id fail");
    }
    return result;
}

int32_t CellularDataService::GetCellularDataFlowType()
{
    int32_t slotId = CellularDataService::GetDefaultCellularDataSlotId();
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("slotId[%{public}d] invalid", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool isSuspend = (GetCellularDataState() == static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED));
    if (isSuspend) {
        return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
    }
    int32_t result = cellularDataControllers_[slotId]->GetCellularDataFlowType();
    TELEPHONY_LOGI("CellDataFlowType:%{public}d", result);
    return result;
}

std::string CellularDataService::GetBindTime()
{
    std::ostringstream oss;
    oss << bindTime_;
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
    int32_t slotId = 0;
    int32_t statusDefault = 0;
    int32_t statusIms = 0;
    slotId = GetDefaultCellularDataSlotId();
    if (!CheckParamValid(slotId)) {
        oss << "default slotId: " << slotId;
        return oss.str();
    }
    statusDefault = cellularDataControllers_[slotId]->GetCellularDataState(DATA_CONTEXT_ROLE_DEFAULT);
    statusIms = cellularDataControllers_[slotId]->GetCellularDataState(DATA_CONTEXT_ROLE_IMS);
    oss << "Default connect state: " << statusDefault;
    oss << "Ims connect state:  " << statusIms;
    return oss.str();
}

std::string CellularDataService::GetFlowDataInfoDump()
{
    std::ostringstream oss;
    int32_t slotId = 0;
    slotId = GetDefaultCellularDataSlotId();
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
    TELEPHONY_LOGI("slotId is :%{public}d enable is :%{public}d", slotId, enable);
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    int32_t result = cellularDataControllers_[slotId]->SetPolicyDataOn(enable);
    return result;
}

int32_t CellularDataService::HasInternetCapability(const int32_t slotId, const int32_t cid)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->HasInternetCapability(cid);
    return result ? static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS) :
        static_cast<int32_t>(RequestNetCode::REQUEST_FAILED);
}

int32_t CellularDataService::ClearCellularDataConnections(const int32_t slotId)
{
    if (!CheckParamValid(slotId)) {
        TELEPHONY_LOGE("cellularDataControllers_[%{public}d] is null", slotId);
        return CELLULAR_DATA_INVALID_PARAM;
    }
    bool result = cellularDataControllers_[slotId]->ClearAllConnections();
    return result ? static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS) :
        static_cast<int32_t>(RequestNetCode::REQUEST_FAILED);
}
} // namespace Telephony
} // namespace OHOS
