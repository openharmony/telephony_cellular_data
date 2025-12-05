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
#include "data_switch_settings.h"
#include "core_manager_inner.h"
#include "telephony_errors.h"
#include "telephony_ext_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::_;
using ::testing::Return;

class DataSwitchSettingTest : public testing::Test {
public:
    DataSwitchSettingTest()
    {
        mockSimManager = new MockSimManager();
        std::shared_ptr<MockSimManager> mockSimManagerPtr(mockSimManager);
        CoreManagerInner::GetInstance().simManager_ = mockSimManagerPtr;
    }
    ~DataSwitchSettingTest() = default;

    static void TearDownTestCase()
    {
        if (CoreManagerInner::GetInstance().simManager_ != nullptr) {
            CoreManagerInner::GetInstance().simManager_ = nullptr;
            std::cout << "DataSwitchSettingTest set simManager_ nullptr" << std::endl;
        }
    }

    MockSimManager *mockSimManager;
};

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_01, Function | MediumTest | Level1)
{
    DataSwitchSettings sets(2);
    std::cout << "DataSwitchSetting_01 slotId: " << sets.slotId_ << std::endl;
    ASSERT_TRUE(sets.SetUserDataOn(true) == TELEPHONY_ERR_SUCCESS);

    DataSwitchSettings sets1(0);
    ASSERT_TRUE(sets.SetUserDataOn(true) == TELEPHONY_ERR_SUCCESS);
}

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_02, Function | MediumTest | Level1)
{
    DataSwitchSettings sets(2);
    std::cout << "DataSwitchSetting_02 slotId: " << sets.slotId_ << std::endl;
    bool dataEnabled;
    ASSERT_TRUE(sets.QueryUserDataStatus(dataEnabled) == TELEPHONY_ERR_SUCCESS);
    ASSERT_TRUE(dataEnabled);

    DataSwitchSettings sets1(0);
    ASSERT_TRUE(sets1.QueryUserDataStatus(dataEnabled) == TELEPHONY_ERR_SUCCESS);
    ASSERT_TRUE(dataEnabled);
}

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_03, Function | MediumTest | Level1)
{
    DataSwitchSettings sets(2);
    std::cout << "DataSwitchSetting_03 slotId: " << sets.slotId_ << std::endl;
    ASSERT_TRUE(sets.SetUserDataRoamingOn(true) == TELEPHONY_ERR_SUCCESS);

    DataSwitchSettings sets1(0);
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    ASSERT_TRUE(sets1.SetUserDataRoamingOn(true) == TELEPHONY_ERR_SLOTID_INVALID);

    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    ASSERT_TRUE(sets1.SetUserDataRoamingOn(true) == TELEPHONY_ERR_SUCCESS);
}

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_04, Function | MediumTest | Level1)
{
    bool dataRoamingEnabled;
    DataSwitchSettings sets(2);
    std::cout << "DataSwitchSetting_04 slotId: " << sets.slotId_ << std::endl;
    ASSERT_TRUE(sets.QueryUserDataRoamingStatus(dataRoamingEnabled) == TELEPHONY_ERR_SUCCESS);
    ASSERT_TRUE(dataRoamingEnabled);

    DataSwitchSettings sets1(0);
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    ASSERT_TRUE(sets1.QueryUserDataRoamingStatus(dataRoamingEnabled) == TELEPHONY_ERR_LOCAL_PTR_NULL);

    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    ASSERT_TRUE(sets1.QueryUserDataRoamingStatus(dataRoamingEnabled) == TELEPHONY_ERR_SUCCESS);
}

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_05, Function | MediumTest | Level1)
{
    DataSwitchSettings sets(0);
    sets.userDataOn_ = false;
    std::cout << "DataSwitchSetting_05 userDataOn_: " << sets.userDataOn_ << ", policyDataOn_:" << sets.policyDataOn_
              << ", internalDataOn_:" << sets.internalDataOn_ << std::endl;
    ASSERT_FALSE(sets.IsAllowActiveData());
    sets.userDataOn_ = true;
    sets.policyDataOn_ = true;
    sets.internalDataOn_ = true;
    ASSERT_TRUE(sets.IsAllowActiveData());
}

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_06, Function | MediumTest | Level1)
{
    DataSwitchSettings sets(2);
    std::cout << "DataSwitchSetting_06 slotId: " << sets.slotId_ << std::endl;
    int32_t simDetected = 1;
    ASSERT_TRUE(sets.SetAnySimDetected(simDetected) == TELEPHONY_ERR_SUCCESS);

    DataSwitchSettings sets1(0);
    ASSERT_TRUE(sets1.SetAnySimDetected(simDetected) == TELEPHONY_ERR_SUCCESS);
}

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_07, Function | MediumTest | Level1)
{
    DataSwitchSettings sets(2);
    std::cout << "DataSwitchSetting_07 slotId: " << sets.slotId_ << std::endl;
    int32_t simDetected = 0;
    ASSERT_TRUE(sets.QueryAnySimDetectedStatus(simDetected) == TELEPHONY_ERR_SUCCESS);

    DataSwitchSettings sets1(0);
    ASSERT_TRUE(sets1.QueryAnySimDetectedStatus(simDetected) == TELEPHONY_ERR_SUCCESS);
}

static bool IsVirtualModemConnected()
{
    return true;
}

static bool IsDcCellularDataAllowed()
{
    return true;
}

HWTEST_F(DataSwitchSettingTest, DataSwitchSetting_08, Function | MediumTest | Level1)
{
    DataSwitchSettings sets(0);
    std::cout << "DataSwitchSetting_08 slotId: " << sets.slotId_ << std::endl;
    TELEPHONY_EXT_WRAPPER.isVirtualModemConnected_ = IsVirtualModemConnected;
    EXPECT_EQ(sets.IsAllowActiveData(), false);

    TELEPHONY_EXT_WRAPPER.isDcCellularDataAllowed_ = IsDcCellularDataAllowed;
    EXPECT_EQ(sets.IsAllowActiveData(), true);

    TELEPHONY_EXT_WRAPPER.isVirtualModemConnected_ = nullptr;
    TELEPHONY_EXT_WRAPPER.isDcCellularDataAllowed_ = nullptr;
}
}  // namespace Telephony
}  // namespace OHOS
