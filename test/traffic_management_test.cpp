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

#include "mock/mock_net_conn_service.h"
#include "mock/mock_sim_manager.h"
#include "traffic_management.h"
#include "core_manager_inner.h"
#include "net_manager_constants.h"
#include "net_conn_client.h"
#include "net_link_info.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::_;
using ::testing::DoAll;
using ::testing::Mock;
using ::testing::Return;
using ::testing::SetArgReferee;

class TrafficManagementTest : public testing::Test {
public:
    TrafficManagementTest()
    {
        trafficManagement = new TrafficManagement(0);

        mockSimManager = new MockSimManager();
        std::shared_ptr<MockSimManager> mockSimManagerPtr(mockSimManager);
        CoreManagerInner::GetInstance().simManager_ = mockSimManagerPtr;

        mockNetConnService = new NetManagerStandard::MockINetConnService();
        sptr<NetManagerStandard::INetConnService> mockNetConnServicePtr(mockNetConnService);
        NetManagerStandard::NetConnClient::GetInstance().NetConnService_ = mockNetConnServicePtr;
    }

    ~TrafficManagementTest()
    {
        delete trafficManagement;
    }

    static void TearDownTestCase()
    {
        if (CoreManagerInner::GetInstance().simManager_ != nullptr) {
            CoreManagerInner::GetInstance().simManager_ = nullptr;
            std::cout << "TrafficManagementTest set simManager_ nullptr" << std::endl;
        }

        if (NetManagerStandard::NetConnClient::GetInstance().NetConnService_ != nullptr) {
            NetManagerStandard::NetConnClient::GetInstance().NetConnService_ = nullptr;
            std::cout << "TrafficManagementTest set NetConnService_ nullptr" << std::endl;
        }
    }

    MockSimManager *mockSimManager;
    NetManagerStandard::MockINetConnService *mockNetConnService;
    TrafficManagement *trafficManagement;
};

HWTEST_F(TrafficManagementTest, TrafficManagementTest_001, Function | MediumTest | Level1)
{
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _)).WillOnce(Return(1));
    trafficManagement->UpdatePacketData();

    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(1));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _)).WillOnce(Return(1));
    std::string ifaceName = trafficManagement->GetIfaceName();
    std::cout << "TrafficManagementTest_001 interface name:" << ifaceName << std::endl;
    ASSERT_TRUE(ifaceName.empty());
}

HWTEST_F(TrafficManagementTest, TrafficManagementTest_002, Function | MediumTest | Level1)
{
    trafficManagement->sendPackets_ = 100;
    trafficManagement->recvPackets_ = 200;
    int64_t sendP = 0;
    int64_t recvP = 0;
    trafficManagement->GetPacketData(sendP, recvP);
    EXPECT_EQ(sendP, 100);
    EXPECT_EQ(recvP, 200);
}

HWTEST_F(TrafficManagementTest, TrafficManagementTest_003, Function | MediumTest | Level1)
{
    // get net id failed
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _)).WillOnce(Return(1));
    std::string ifaceName = trafficManagement->GetIfaceName();
    ASSERT_EQ(ifaceName, "");

    // get all nets failed
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetConnService, GetAllNets(_)).WillOnce(Return(1));
    ifaceName = trafficManagement->GetIfaceName();
    ASSERT_EQ(ifaceName, "");

    // net id not in netAllIds
    std::list<int32_t> netIdList = {1, 2};
    std::list<int32_t> netAllIds = {3};
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(netIdList), Return(0)));
    EXPECT_CALL(*mockNetConnService, GetAllNets(_)).WillOnce(DoAll(SetArgReferee<0>(netAllIds), Return(0)));
    ifaceName = trafficManagement->GetIfaceName();
    ASSERT_EQ(ifaceName, "");
}

HWTEST_F(TrafficManagementTest, TrafficManagementTest_004, Function | MediumTest | Level1)
{
    // net id in all netAllIds
    std::list<int32_t> netIdList = {1, 2};
    std::list<int32_t> netAllIds = {1, 2};
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(netIdList), Return(0)));
    EXPECT_CALL(*mockNetConnService, GetAllNets(_)).WillOnce(DoAll(SetArgReferee<0>(netAllIds), Return(0)));
    EXPECT_CALL(*mockNetConnService, GetConnectionProperties(_, _))
        .WillOnce([](int32_t netId, NetManagerStandard::NetLinkInfo &info) {
            info.ifaceName_ = "mock_ifaceName";
            return 0;
        });
    std::string ifaceName = trafficManagement->GetIfaceName();
    std::cout << "TrafficManagementTest_003 ifaceName: " << ifaceName << std::endl;
    ASSERT_EQ(ifaceName, "mock_ifaceName");

    // update data
    EXPECT_CALL(*mockSimManager, GetSimId(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockNetConnService, GetNetIdByIdentifier(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(netIdList), Return(0)));
    EXPECT_CALL(*mockNetConnService, GetAllNets(_)).WillOnce(DoAll(SetArgReferee<0>(netAllIds), Return(0)));
    EXPECT_CALL(*mockNetConnService, GetConnectionProperties(_, _))
        .WillOnce([](int32_t netId, NetManagerStandard::NetLinkInfo &info) {
            info.ifaceName_ = "mock_ifaceName";
            return 0;
        });
    trafficManagement->sendPackets_ = 100;
    trafficManagement->recvPackets_ = 200;
    trafficManagement->UpdatePacketData();
    EXPECT_EQ(trafficManagement->sendPackets_, 0);
    EXPECT_LE(trafficManagement->recvPackets_, 0);

    Mock::VerifyAndClearExpectations(mockSimManager);
}

}  // namespace Telephony
}  // namespace OHOS