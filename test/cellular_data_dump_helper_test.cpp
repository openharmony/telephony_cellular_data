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

#include "cellular_data_dump_helper.h"
#include "core_service_client.h"
#include "mock/mock_core_service.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::SetArgReferee;

class CellularDataDumpHelperTest : public testing::Test {
public:
    CellularDataDumpHelperTest()
    {
        mockCoreService = new MockCoreService();
        sptr<ICoreService> mockCoreServicePtr(mockCoreService);
        DelayedRefSingleton<CoreServiceClient>::GetInstance().proxy_ = mockCoreServicePtr;
    }
    ~CellularDataDumpHelperTest() = default;
    static void TearDownTestCase()
    {
        if (DelayedRefSingleton<CoreServiceClient>::GetInstance().proxy_ != nullptr) {
            DelayedRefSingleton<CoreServiceClient>::GetInstance().proxy_ = nullptr;
            std::cout << "CellularDataDumpHelperTest set proxy_ nullptr" << std::endl;
        }
    }
    MockCoreService *mockCoreService;
};

HWTEST_F(CellularDataDumpHelperTest, CellularDataDumpHelper_01, Function | MediumTest | Level1)
{
    CellularDataDumpHelper help;
    std::vector<std::string> args = {"cellular_data", "--help"};
    std::string result = "";
    help.Dump(args, result);
    std::cout << "CellularDataDumpHelper_01 result: " << result << std::endl;
    ASSERT_FALSE(result.find("CellularData") == std::string::npos);
    ASSERT_FALSE(result.find("Usage:dump <command> [options]") == std::string::npos);
}

HWTEST_F(CellularDataDumpHelperTest, CellularDataDumpHelper_02, Function | MediumTest | Level1)
{
    CellularDataDumpHelper help;
    std::vector<std::string> args = {"cellular_data", "help"};
    std::string result = "";
    EXPECT_CALL(*mockCoreService, HasSimCard(_, _))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<1>(false), Return(0)));
    help.Dump(args, result);
    std::cout << "CellularDataDumpHelper_02 result: " << result << std::endl;
    ASSERT_FALSE(result.find("Ohos cellular data service") == std::string::npos);
    ASSERT_TRUE(result.find("CellularDataRoamingEnabled") == std::string::npos);
}

HWTEST_F(CellularDataDumpHelperTest, CellularDataDumpHelper_03, Function | MediumTest | Level1)
{
    CellularDataDumpHelper help;
    std::vector<std::string> args = {"cellular_data_1", "--help"};
    std::string result = "";
    EXPECT_CALL(*mockCoreService, HasSimCard(_, _))
        .Times(AtLeast(0))
        .WillRepeatedly(DoAll(SetArgReferee<1>(false), Return(0)));
    help.Dump(args, result);
    std::cout << "CellularDataDumpHelper_03 result: " << result << std::endl;
    ASSERT_FALSE(result.find("Ohos cellular data service") == std::string::npos);
    ASSERT_TRUE(result.find("CellularDataRoamingEnabled") == std::string::npos);
}

HWTEST_F(CellularDataDumpHelperTest, CellularDataDumpHelper_04, Function | MediumTest | Level1)
{
    maxSlotCount_ = 2;
    EXPECT_CALL(*mockCoreService, HasSimCard(0, _)).WillOnce(DoAll(SetArgReferee<1>(true), Return(0)));
    EXPECT_CALL(*mockCoreService, HasSimCard(1, _)).WillOnce(DoAll(SetArgReferee<1>(false), Return(0)));
    CellularDataDumpHelper help;
    std::vector<std::string> args = {"cellular_data_1", "help"};
    std::string result = "";
    help.Dump(args, result);
    std::cout << "CellularDataDumpHelper_04 result: " << result << std::endl;
    ASSERT_FALSE(result.find("Ohos cellular data service") == std::string::npos);
    ASSERT_FALSE(result.find("CellularDataRoamingEnabled") == std::string::npos);
    maxSlotCount_ = 0;
}

}  // namespace Telephony
}  // namespace OHOS