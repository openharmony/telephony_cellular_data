/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#define private public
#define protected public

#include "common_event_manager.h"
#include "common_event_support.h"
#include "cellular_data_constant.h"
#include "cellular_data_handler.h"
#include "core_manager_inner.h"
#include "mock/mock_sim_manager.h"
#include "mock/mock_network_search.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;

class CellularDataHandlerBranchTest : public testing::Test {
public:
    CellularDataHandlerBranchTest() = default;
    ~CellularDataHandlerBranchTest() = default;
    static void TearDownTestCase()
    {
        UnmockManager();
    }

    void InitMockManager()
    {
        mockSimManager = new MockSimManager();
        std::shared_ptr<MockSimManager> mockSimManagerPtr(mockSimManager);
        CoreManagerInner::GetInstance().simManager_ = mockSimManagerPtr;

        mockNetworkSearchManager = new MockNetworkSearchManager();
        std::shared_ptr<MockNetworkSearchManager> mockNetworkSearchManagerPtr(mockNetworkSearchManager);
        CoreManagerInner::GetInstance().networkSearchManager_ = mockNetworkSearchManagerPtr;
    }

    static void UnmockManager()
    {
        if (CoreManagerInner::GetInstance().simManager_ != nullptr) {
            CoreManagerInner::GetInstance().simManager_ = nullptr;
            std::cout << "CellularDataHandlerBranchTest set simManager_ nullptr" << std::endl;
        }
        if (CoreManagerInner::GetInstance().networkSearchManager_ != nullptr) {
            CoreManagerInner::GetInstance().networkSearchManager_ = nullptr;
            std::cout << "CellularDataHandlerBranchTest set networkSearchManager_ nullptr" << std::endl;
        }
    }

    void InitCellularDataHandler()
    {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
        EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
        cellularDataHandler_ = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
        cellularDataHandler_->Init();
    }

    std::shared_ptr<CellularDataHandler> cellularDataHandler_;
    MockSimManager *mockSimManager;
    MockNetworkSearchManager *mockNetworkSearchManager;
};

HWTEST_F(CellularDataHandlerBranchTest, RoamingStateOn_001, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();
    // GetPsRoamingState return 1, dataRoamingEnabled is false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    auto event = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler_->RoamingStateOn(event);
    
    // GetPsRoamingState return 1, dataRoamingEnabled is false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockSimManager, GetSimId(_)).Times(AtLeast(0)).WillRepeatedly(Return(1));
    cellularDataHandler_->defaultDataRoamingEnable_ = true;
    auto event1 = AppExecFwk::InnerEvent::Get(0);
    cellularDataHandler_->RoamingStateOn(event1);
    EXPECT_EQ(cellularDataHandler_->apnManager_->GetOverallApnState(), PROFILE_STATE_IDLE);
    UnmockManager();
}

HWTEST_F(CellularDataHandlerBranchTest, CheckRoamingState_001, Function | MediumTest | Level3)
{
    InitCellularDataHandler();
    InitMockManager();
    // roamingState true, dataRoamingEnabled true, isMmsApn true, isEmergencyApn false, IsRestrictedMode false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _)).WillOnce(Return(0));
    sptr<ApnHolder> apnHolder = new ApnHolder("mms", 0);
    ASSERT_FALSE(cellularDataHandler_ == nullptr);
    ASSERT_FALSE(cellularDataHandler_->dataSwitchSettings_ == nullptr);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(true);
    ASSERT_TRUE(cellularDataHandler_->CheckRoamingState(apnHolder));

    // roamingState false, dataRoamingEnabled true, isMmsApn false, isEmergencyApn true, IsRestrictedMode false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _)).WillOnce(Return(0));
    apnHolder = new ApnHolder("emergency", 0);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(true);
    ASSERT_TRUE(cellularDataHandler_->CheckRoamingState(apnHolder));

    // roamingState true, dataRoamingEnabled false, isMmsApn true, isEmergencyApn false, IsRestrictedMode false
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(1));
    apnHolder = new ApnHolder("mms", 0);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(false);
    ASSERT_FALSE(cellularDataHandler_->CheckRoamingState(apnHolder));

    // roamingState false, dataRoamingEnabled false, isMmsApn false, isEmergencyApn true, IsRestrictedMode true
    int32_t tech = 1;
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRoamingState(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetworkSearchManager, GetPsRadioTech(_, _)).WillOnce(DoAll(SetArgReferee<1>(tech), Return(0)));
    cellularDataHandler_->lastCallState_ = 1;
    apnHolder = new ApnHolder("emergency", 0);
    cellularDataHandler_->dataSwitchSettings_->UpdateUserDataRoamingOn(false);
    ASSERT_FALSE(cellularDataHandler_->CheckRoamingState(apnHolder));
    UnmockManager();
}

} // namespace Telephony
} // namespace OHOS