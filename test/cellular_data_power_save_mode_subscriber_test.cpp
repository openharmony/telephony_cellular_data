/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifdef BASE_POWER_IMPROVEMENT
#define private public
#define protected public

#include "cellular_data_power_save_mode_subscriber.h"
#include "gtest/gtest.h"
#include "common_event_support.h"
#include "telephony_types.h"
#include "cellular_data_handler.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class CellularDataPowerSaveModeSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_01,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    std::string action = ENTER_STR_TELEPHONY_NOTIFY;
    auto subscriber = cellularDataHandler->strEnterSubscriber_;
    cellularDataHandler = nullptr;
    subscriber->HandleEnterStrEvent(action);
    EXPECT_EQ(subscriber->GetPowerSaveFlag(), false);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_02,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    auto want = EventFwk::Want();
    want.SetAction(ENTER_STR_TELEPHONY_NOTIFY);
    auto event = EventFwk::CommonEventData(want);
    event.SetCode(1);
    cellularDataHandler->strEnterSubscriber_->OnReceiveEvent(event);
    EXPECT_EQ(cellularDataHandler->strEnterSubscriber_->lastMsg, ENTER_STR_TELEPHONY_NOTIFY);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_03,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    auto want = EventFwk::Want();
    want.SetAction(EXIT_STR_TELEPHONY_NOTIFY);
    auto event = EventFwk::CommonEventData(want);
    event.SetCode(1);
    cellularDataHandler->strExitSubscriber_->OnReceiveEvent(event);
    EXPECT_EQ(cellularDataHandler->strExitSubscriber_->lastMsg, EXIT_STR_TELEPHONY_NOTIFY);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_04,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    auto want = EventFwk::Want();
    want.SetAction(ENTER_STR_TELEPHONY_NOTIFY);
    auto event = EventFwk::CommonEventData(want);
    cellularDataHandler->strEnterSubscriber_->OnReceiveEvent(event);
    cellularDataHandler->strEnterSubscriber_->OnReceiveEvent(event);
    EXPECT_EQ(cellularDataHandler->strEnterSubscriber_->lastMsg, ENTER_STR_TELEPHONY_NOTIFY);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_05,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    auto want = EventFwk::Want();
    want.SetAction(EXIT_STR_TELEPHONY_NOTIFY);
    auto event = EventFwk::CommonEventData(want);
    event.SetCode(1);
    cellularDataHandler->strExitSubscriber_->OnReceiveEvent(event);
    cellularDataHandler->strExitSubscriber_->OnReceiveEvent(event);
    EXPECT_EQ(cellularDataHandler->strExitSubscriber_->lastMsg, EXIT_STR_TELEPHONY_NOTIFY);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_06,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    int32_t resultCode = 1;
    std::string resultData = "aa";
    bool ordered = false;
    bool sticky = true;
    sptr<IRemoteObject> token = nullptr;
    cellularDataHandler->strEnterSubscriber_->strAsyncCommonEvent_ =
        std::make_shared<EventFwk::AsyncCommonEventResult>(resultCode, resultData, ordered, sticky, token);
    auto want = EventFwk::Want();
    bool ret = cellularDataHandler->strEnterSubscriber_->FinishTelePowerCommonEvent();
    EXPECT_EQ(ret, false);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_07,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    cellularDataHandler->strEnterSubscriber_->strAsyncCommonEvent_ = nullptr;
    bool ret = cellularDataHandler->strEnterSubscriber_->FinishTelePowerCommonEvent();
    EXPECT_EQ(ret, false);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_08,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    std::string action = EXIT_STR_TELEPHONY_NOTIFY;
    auto subscriber = cellularDataHandler->strExitSubscriber_;
    cellularDataHandler = nullptr;
    subscriber->HandleExitStrEvent(action);
    EXPECT_EQ(cellularDataHandler->strExitSubscriber_->lastMsg, EXIT_STR_TELEPHONY_NOTIFY);
}

HWTEST_F(CellularDataPowerSaveModeSubscriberTest, CellularDataPowerSaveModeSubscriber_09,
         Function | MediumTest | Level1)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler =  std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->SubscribeTelePowerEvent();
    auto want = EventFwk::Want();
    want.SetAction(DYNAMIC_POWEROFF_MODEM_WITH_STR);
    auto event = EventFwk::CommonEventData(want);
    cellularDataHandler->strEnterSubscriber_->OnReceiveEvent(event);
    EXPECT_EQ(cellularDataHandler->strEnterSubscriber_->lastMsg, EXIT_STR_TELEPHONY_NOTIFY);
}
}
}
#endif
