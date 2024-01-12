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

#include "default.h"

#include "cellular_data_utils.h"
#include "core_manager_inner.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void Default::StateBegin()
{
    TELEPHONY_LOGD("Enter default state");
    isActive_ = true;
}

void Default::StateEnd()
{
    TELEPHONY_LOGD("Exit default state");
    isActive_ = false;
}

bool Default::StateProcess(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    uint32_t eventCode = event->GetInnerEventId();
    std::map<uint32_t, Fun>::iterator it = eventIdFunMap_.find(eventCode);
    if (it != eventIdFunMap_.end()) {
        return (this->*(it->second))(event);
    }
    return false;
}

bool Default::ProcessConnectDone(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Default::MSG_SM_CONNECT");
    return false;
}

bool Default::ProcessDisconnectDone(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("The state machine pointer is null");
        return false;
    }
    TELEPHONY_LOGI("The data connection is disconnected by default");
    stateMachine->DeferEvent(std::move(event));
    return true;
}

bool Default::ProcessDisconnectAllDone(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("The state machine pointer is null");
        return false;
    }
    TELEPHONY_LOGI("All data connections are disconnected by default");
    stateMachine->DeferEvent(std::move(event));
    return true;
}

bool Default::ProcessDataConnectionDrsOrRatChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event is null");
        return false;
    }
    std::shared_ptr<CellularDataStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        TELEPHONY_LOGE("stateMachine is null");
        return false;
    }
    TELEPHONY_LOGI("The RAT changes by default");
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
    int32_t supplierId = netAgent.GetSupplierId(stateMachine->GetSlotId(), stateMachine->GetCapability());
    netAgent.UpdateNetSupplierInfo(supplierId, stateMachine->netSupplierInfo_);
    netAgent.UpdateNetLinkInfo(supplierId, stateMachine->netLinkInfo_);
    int32_t radioTech = static_cast<int32_t>(RadioTech::RADIO_TECHNOLOGY_INVALID);
    CoreManagerInner::GetInstance().GetPsRadioTech(stateMachine->GetSlotId(), radioTech);
    netAgent.RegisterSlotType(supplierId, radioTech);
    TELEPHONY_LOGI("RegisterSlotType: supplierId[%{public}d] slotId[%{public}d] radioTech[%{public}d]",
        supplierId, stateMachine->GetSlotId(), radioTech);
    return false;
}

bool Default::ProcessDataConnectionRoamOn(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Default::EVENT_DATA_CONNECTION_ROAM_ON");
    return false;
}

bool Default::ProcessDataConnectionRoamOff(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGI("Default::EVENT_DATA_CONNECTION_ROAM_OFF");
    return false;
}
} // namespace Telephony
} // namespace OHOS
