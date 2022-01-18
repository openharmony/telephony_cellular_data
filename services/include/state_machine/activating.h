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

#ifndef ACTIVATING_H
#define ACTIVATING_H

#include <memory>
#include <string>

#include "event_handler.h"
#include "inner_event.h"

#include "cellular_data_state_machine.h"

namespace OHOS {
namespace Telephony {
class Activating : public State {
public:
    Activating(std::weak_ptr<CellularDataStateMachine> &&cellularService, std::string &&name)
        : State(std::move(name)), stateMachine_(std::move(cellularService))
    {}
    virtual ~Activating() = default;
    virtual void StateBegin();
    virtual void StateEnd();
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event);

private:
    DisConnectionReason DataCallPdpError(int32_t reason);
    bool RilActivatePdpContextDone(const AppExecFwk::InnerEvent::Pointer &event);
    bool RilErrorResponse(const AppExecFwk::InnerEvent::Pointer &event);
    void ProcessConnectTimeout(const AppExecFwk::InnerEvent::Pointer &event);
    std::weak_ptr<CellularDataStateMachine> stateMachine_;
};
} // namespace Telephony
} // namespace OHOS
#endif // ACTIVATING_H
