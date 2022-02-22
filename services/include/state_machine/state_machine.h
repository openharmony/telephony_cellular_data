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

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "event_handler.h"
#include "inner_event.h"

#include "telephony_log_wrapper.h"

#include "cellular_data_event_code.h"

namespace OHOS {
namespace Telephony {
class State : public RefBase {
#define PROCESSED true
#define NOT_PROCESSED false
public:
    explicit State(std::string &&name) : name_(std::move(name)) {}
    virtual ~State() = default;
    virtual void StateBegin() = 0;
    virtual void StateEnd() = 0;
    virtual bool StateProcess(const AppExecFwk::InnerEvent::Pointer &event) = 0;

    void SetParentState(sptr<State> &parent)
    {
        parent_ = parent;
    }

    std::string GetStateMachineName() const
    {
        return name_;
    }

protected:
    friend class StateMachineEventHandler;
    std::string name_;
    sptr<State> parent_;
    bool isActive_ = false;
};

class StateMachineEventHandler : public AppExecFwk::EventHandler {
public:
    explicit StateMachineEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
        : EventHandler(runner)
    {}
    ~StateMachineEventHandler() = default;

    virtual void SetOriginalState(sptr<State> &originalState)
    {
        originalState_ = originalState;
    }

    virtual void TransitionTo(sptr<State> &destState)
    {
        TELEPHONY_LOGI("State machine transition to %{public}s", destState->name_.c_str());
        destState_ = destState;
    }

    virtual void Quit()
    {
        sptr<State> tmpState = curState_;
        while (tmpState != nullptr && tmpState->isActive_) {
            tmpState->StateEnd();
            tmpState = tmpState->parent_;
            isQuit_ = true;
        }
    }

    // Only two-layer StateMachines are supported
    virtual void ProcessTransitions(const AppExecFwk::InnerEvent::Pointer &event)
    {
        if (curState_ != destState_) {
            TELEPHONY_LOGI("Begin process transitions");
            if (curState_ != nullptr) {
                sptr<State> tmpState = curState_->parent_;
                while (tmpState != nullptr) {
                    tmpState->StateEnd();
                    tmpState = tmpState->parent_;
                }
                curState_->StateEnd();
            }
            if (destState_ != nullptr) {
                sptr<State> tmpState = destState_->parent_;
                while (tmpState != nullptr) {
                    tmpState->StateBegin();
                    tmpState = tmpState->parent_;
                }
                destState_->StateBegin();
            }
            curState_ = destState_;
            SendDeferredEvent();
        }
    }

    void DeferEvent(AppExecFwk::InnerEvent::Pointer &&event)
    {
        std::lock_guard<std::mutex> guard(mtx_);
        deferEvents_.push_back(std::move(event));
    }

    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
    {
        if (event == nullptr || isQuit_) {
            TELEPHONY_LOGE("The event parameter is incorrect");
            return;
        }
        if (event->GetInnerEventId() == CellularDataEventCode::MSG_STATE_MACHINE_QUIT) {
            TELEPHONY_LOGI("State machine exit");
            Quit();
            return;
        }
        if (event->GetInnerEventId() == CellularDataEventCode::MSG_STATE_MACHINE_INIT) {
            destState_ = originalState_;
            InitCmdEnter(originalState_);
        }
        ProcessMsg(event);
        ProcessTransitions(event);
    }

    virtual void ProcessMsg(const AppExecFwk::InnerEvent::Pointer &event)
    {
        sptr<State> tmpState = curState_;
        TELEPHONY_LOGI("The event id: %{public}u", event->GetInnerEventId());
        while (tmpState != nullptr && !tmpState->StateProcess(event)) {
            tmpState = tmpState->parent_;
        }
    }

private:
    void InitCmdEnter(const sptr<State> &state)
    {
        if (state == nullptr) {
            TELEPHONY_LOGE("registerState_ is null");
            return;
        }
        if (state->parent_ != nullptr) {
            InitCmdEnter(state->parent_);
        }
        TELEPHONY_LOGI("Initialize entry %{public}s", state->name_.c_str());
        state->StateBegin();
        curState_ = state;
    }

    void SendDeferredEvent()
    {
        std::lock_guard<std::mutex> guard(mtx_);
        if (deferEvents_.empty()) {
            return;
        }
        for (size_t i = 0; i < deferEvents_.size(); ++i) {
            AppExecFwk::InnerEvent::Pointer event = std::move(deferEvents_[i]);
            SendImmediateEvent(event);
        }
        deferEvents_.clear();
    }

private:
    sptr<State> originalState_;
    sptr<State> destState_;
    sptr<State> curState_;
    std::vector<AppExecFwk::InnerEvent::Pointer> deferEvents_;
    std::mutex mtx_;
    bool isQuit_ = false;
};

class StateMachine {
public:
    explicit StateMachine(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    {
        stateMachineEventHandler_ = std::make_shared<StateMachineEventHandler>(runner);
        if (stateMachineEventHandler_ == nullptr) {
            TELEPHONY_LOGE("stateMachineEventHandler_ is null");
            return;
        }
    }

    virtual ~StateMachine() {}

    void Quit()
    {
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_STATE_MACHINE_QUIT);
        if (stateMachineEventHandler_ == nullptr) {
            TELEPHONY_LOGE("stateMachineEventHandler_ is null");
            return;
        }
        stateMachineEventHandler_->SendImmediateEvent(event);
    }

    void Start()
    {
        if (stateMachineEventHandler_ == nullptr) {
            TELEPHONY_LOGE("stateMachineEventHandler_ is null");
            return;
        }
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_STATE_MACHINE_INIT);
        stateMachineEventHandler_->SendImmediateEvent(event);
    }

    void SetOriginalState(sptr<State> &originalState)
    {
        if (originalState == nullptr) {
            TELEPHONY_LOGE("originalState is null");
            return;
        }
        if (stateMachineEventHandler_ == nullptr) {
            TELEPHONY_LOGE("stateMachineEventHandler_ is null");
            return;
        }
        stateMachineEventHandler_->SetOriginalState(originalState);
    }

    void TransitionTo(sptr<State> &destState)
    {
        if (destState == nullptr) {
            TELEPHONY_LOGE("destState is null");
            return;
        }
        if (stateMachineEventHandler_ == nullptr) {
            TELEPHONY_LOGE("stateMachineEventHandler_ is null");
            return;
        }
        stateMachineEventHandler_->TransitionTo(destState);
    }

    void DeferEvent(const AppExecFwk::InnerEvent::Pointer &&event)
    {
        if (stateMachineEventHandler_ == nullptr) {
            TELEPHONY_LOGE("stateMachineEventHandler_ is null");
            return;
        }
        stateMachineEventHandler_->DeferEvent(std::move(const_cast<AppExecFwk::InnerEvent::Pointer &>(event)));
    }

    void SendEvent(AppExecFwk::InnerEvent::Pointer &event)
    {
        if (stateMachineEventHandler_ == nullptr) {
            TELEPHONY_LOGE("stateMachineEventHandler_ is null");
            return;
        }
        TELEPHONY_LOGI("State machine send event id %{public}u ", event->GetInnerEventId());
        stateMachineEventHandler_->SendEvent(event);
    }

protected:
    std::shared_ptr<StateMachineEventHandler> stateMachineEventHandler_;
};
} // namespace Telephony
} // namespace OHOS
#endif // STATE_MACHINE_H
