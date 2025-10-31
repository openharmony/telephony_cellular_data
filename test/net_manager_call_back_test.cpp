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
#include "gtest/gtest.h"
#include "cellular_data_error.h"
#include "net_all_capabilities.h"
#include "net_manager_call_back.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class NetManagerCallBackTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number   RequestNetwork_001
 * @tc.name     Test NetManagerCallBack RequestNetwork
 * @tc.desc     RequestNetwork ipc
 */
HWTEST_F(NetManagerCallBackTest, RequestNetwork_001, TestSize.Level3)
{
    auto netManagerCallBack = std::make_shared<NetManagerCallBack>();
    NetManagerStandard::NetRequest request;
    std::string ident = "testIdent";
    std::set<NetManagerStandard::NetCap> netCaps = {};
    int32_t result = netManagerCallBack->RequestNetwork(ident, netCaps, request);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
    netCaps.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
    result = netManagerCallBack->RequestNetwork(ident, netCaps, request);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   ReleaseNetwork_001
 * @tc.name     Test NetManagerCallBack ReleaseNetwork
 * @tc.desc     ReleaseNetwork ipc
 */
HWTEST_F(NetManagerCallBackTest, ReleaseNetwork_001, TestSize.Level3)
{
    auto netManagerCallBack = std::make_shared<NetManagerCallBack>();
    NetManagerStandard::NetRequest request;
    request.ident = "testIdent";
    int32_t result = netManagerCallBack->ReleaseNetwork(request);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
    request.netCaps.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
    result = netManagerCallBack->ReleaseNetwork(request);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   ReleaseNetwork_002
 * @tc.name     Test NetManagerCallBack ReleaseNetwork
 * @tc.desc     ReleaseNetwork ipc
 */
HWTEST_F(NetManagerCallBackTest, ReleaseNetwork_002, TestSize.Level3)
{
    auto netManagerCallBack = std::make_shared<NetManagerCallBack>();
    NetManagerStandard::NetRequest request;
    request.ident = "testIdent";
    reqeust.bearTypes.insert(NetManagerStandard::NetBearType::BEARER_WIFI);
    request.netCaps.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
    reqeust.isRemoveUid = 255;
    int32_t result = netManagerCallBack->ReleaseNetwork(request);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   AddRequest_001
 * @tc.name     Test NetManagerCallBack AddRequest
 * @tc.desc     Add Request and uid
 */
HWTEST_F(NetManagerCallBackTest, AddRequest_001, TestSize.Level3)
{
    auto netManagerCallBack = std::make_shared<NetManagerCallBack>();
    NetManagerStandard::NetRequest request;
    request.ident = "testIdent";
    int32_t result = netManagerCallBack->AddRequest(request);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
    request.netCaps.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
    result = netManagerCallBack->AddRequest(request);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}
}
}