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

#include <gtest/gtest.h>

#include "mock/mock_sim_manager.h"
#include "core_manager_inner.h"
#include "cellular_data_handler.h"
#include "cellular_data_incall_observer.h"
#include "cellular_data_setting_observer.h"
#include "cellular_data_rdb_observer.h"
#include "cellular_data_roaming_observer.h"
#include "cellular_data_settings_rdb_helper.h"
#include "cellular_data_airplane_observer.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::_;
using ::testing::Mock;
using ::testing::Return;

class CellularDataObserverTest : public testing::Test {
public:
    CellularDataObserverTest() = default;
    ~CellularDataObserverTest() = default;

    static void TearDownTestCase()
    {
        if (CoreManagerInner::GetInstance().simManager_ != nullptr) {
            CoreManagerInner::GetInstance().simManager_ = nullptr;
            std::cout << "CellularDataObserverTest set simManager_ nullptr" << std::endl;
        }
    }

    static std::shared_ptr<CellularDataHandler> createCellularDataHandler()
    {
        EventFwk::CommonEventSubscribeInfo subscriberInfo;
        subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
        std::shared_ptr<CellularDataHandler> cellularDataHandler =
            std::make_shared<CellularDataHandler>(subscriberInfo, 0);
        return cellularDataHandler;
    }
};

HWTEST_F(CellularDataObserverTest, CellularDataIncallObserver_01, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularDataHandler> cellularDataHandler = nullptr;
    auto cellularDataIncallObserver = std::make_shared<CellularDataIncallObserver>(cellularDataHandler);
    cellularDataIncallObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // GetValue success, cellularDataHandler is nullptr
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    Uri uri(CELLULAR_DATA_SETTING_DATA_INCALL_URI);
    ASSERT_TRUE(settingHelper->PutValue(uri, CELLULAR_DATA_COLUMN_INCALL, 1) == TELEPHONY_ERR_SUCCESS);
    cellularDataIncallObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // GetValue success, cellularDataHandler is not nullptr
    cellularDataHandler = CellularDataObserverTest::createCellularDataHandler();
    cellularDataIncallObserver = std::make_shared<CellularDataIncallObserver>(cellularDataHandler);
    cellularDataIncallObserver->OnChange();
    ASSERT_FALSE(cellularDataHandler == nullptr);
}

HWTEST_F(CellularDataObserverTest, CellularDataRdbObserver_02, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularDataHandler> cellularDataHandler = nullptr;
    auto cellularDataRdbObserver = std::make_shared<CellularDataRdbObserver>(cellularDataHandler);
    cellularDataRdbObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // cellularDataHandler is not nullptr
    cellularDataHandler = CellularDataObserverTest::createCellularDataHandler();
    cellularDataRdbObserver = std::make_shared<CellularDataRdbObserver>(cellularDataHandler);
    cellularDataRdbObserver->OnChange();
    ASSERT_FALSE(cellularDataHandler == nullptr);
}

HWTEST_F(CellularDataObserverTest, CellularDataRoamingObserver_03, Function | MediumTest | Level1)
{
    // mock sim manager
    ASSERT_TRUE(CoreManagerInner::GetInstance().simManager_ == nullptr);
    MockSimManager *mockSimManager = new MockSimManager();
    std::shared_ptr<MockSimManager> mockSimManagerPtr(mockSimManager);
    CoreManagerInner::GetInstance().OnInit(nullptr, mockSimManagerPtr, nullptr);
    ASSERT_FALSE(CoreManagerInner::GetInstance().simManager_ == nullptr);
    ASSERT_EQ(CoreManagerInner::GetInstance().simManager_, mockSimManagerPtr);

    // get sim id failed
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(-1));
    std::shared_ptr<CellularDataHandler> cellularDataHandler = nullptr;
    auto cellularDataRoamingObserver = std::make_shared<CellularDataRoamingObserver>(cellularDataHandler, 0);
    cellularDataRoamingObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // get sim id success, GetValue failed
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    cellularDataRoamingObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // get sim id success, GetValue success, cellularDataHandler is nullptr
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    Uri uri(std::string(CELLULAR_DATA_SETTING_DATA_ROAMING_URI) + std::to_string(1));
    std::string col = std::string(CELLULAR_DATA_COLUMN_ROAMING) + std::to_string(1);
    ASSERT_TRUE(settingHelper->PutValue(uri, col, 1) == TELEPHONY_ERR_SUCCESS);
    cellularDataRoamingObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // get sim id success, GetValue success, cellularDataHandler is nullptr
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    cellularDataHandler = CellularDataObserverTest::createCellularDataHandler();
    cellularDataRoamingObserver = std::make_shared<CellularDataRoamingObserver>(cellularDataHandler, 0);
    cellularDataRoamingObserver->OnChange();
    ASSERT_FALSE(cellularDataHandler == nullptr);

    Mock::VerifyAndClearExpectations(mockSimManager);
}

HWTEST_F(CellularDataObserverTest, CellularDataSettingObserver_04, Function | MediumTest | Level1)
{
    std::shared_ptr<CellularDataHandler> cellularDataHandler = nullptr;
    auto cellularDataSettingObserver = std::make_shared<CellularDataSettingObserver>(cellularDataHandler);
    cellularDataSettingObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // GetValue success, cellularDataHandler is nullptr
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    Uri uri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    ASSERT_TRUE(settingHelper->PutValue(uri, CELLULAR_DATA_COLUMN_ENABLE, 1) == TELEPHONY_ERR_SUCCESS);
    cellularDataSettingObserver->OnChange();
    ASSERT_TRUE(cellularDataHandler == nullptr);

    // GetValue success, cellularDataHandler is not nullptr
    cellularDataHandler = CellularDataObserverTest::createCellularDataHandler();
    cellularDataSettingObserver = std::make_shared<CellularDataSettingObserver>(cellularDataHandler);
    cellularDataSettingObserver->OnChange();
    ASSERT_FALSE(cellularDataHandler == nullptr);
}

HWTEST_F(CellularDataObserverTest, CellularDataAirplaneObserver_05, Function | MediumTest | Level1)
{
    auto cellularDataAirplaneObserver = std::make_shared<CellularDataAirplaneObserver>();
    
    std::shared_ptr<CellularDataSettingsRdbHelper> settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    Uri uri(CELLULAR_DATA_AIRPLANE_MODE_URI);
    EXPECT_TRUE(settingHelper->PutValue(uri, CELLULAR_DATA_COLUMN_AIRPLANE, 1) == TELEPHONY_ERR_SUCCESS);
    cellularDataAirplaneObserver->OnChange();
    EXPECT_TRUE(cellularDataAirplaneObserver->isAirplaneModeOn_);

    EXPECT_TRUE(settingHelper->PutValue(uri, CELLULAR_DATA_COLUMN_AIRPLANE, 0) == TELEPHONY_ERR_SUCCESS);
    cellularDataAirplaneObserver->OnChange();
    EXPECT_FALSE(cellularDataAirplaneObserver->isAirplaneModeOn_);
    EXPECT_FALSE(cellularDataAirplaneObserver->IsAirplaneModeOn());
}

}  // namespace Telephony
}  // namespace OHOS