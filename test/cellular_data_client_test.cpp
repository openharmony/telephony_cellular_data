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

#include "cellular_data_client.h"
#include "cellular_data_constant.h"
#include "data_access_token.h"
#include "gtest/gtest.h"
#include "telephony_errors.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class CellularDataClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void CellularDataClientTest::SetUpTestCase() {}

void CellularDataClientTest::TearDownTestCase() {}

void CellularDataClientTest::SetUp() {}

void CellularDataClientTest::TearDown() {}

/**
 * @tc.number   EnableIntelligenceSwitch_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, CellularDataServiceValidTest_001, TestSize.Level0)
{
    auto proxy = DelayedRefSingleton<CellularDataService>::GetInstance()->GetProxy();
    EXPECT_TRUE(proxy != nullptr);
}

/**
 * @tc.number   EnableIntelligenceSwitch_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, EnableIntelligenceSwitch_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().EnableIntelligenceSwitch(false);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   GetApnState_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, GetApnState_001, TestSize.Level0)
{
    std::string str;
    int32_t result = CellularDataClient::GetInstance().GetApnState(0, str);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.number   GetDataRecoveryState_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, GetDataRecoveryState_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().GetDataRecoveryState();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.number   IsCellularDataRoamingEnabled_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, IsCellularDataRoamingEnabled_001, TestSize.Level0)
{
    bool dataRoamingEnabled = false;
    CellularDataClient::GetInstance().IsCellularDataRoamingEnabled(0, dataRoamingEnabled);
    EXPECT_EQ(dataRoamingEnabled, false);
}

/**
 * @tc.number   ClearCellularDataConnections_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, ClearCellularDataConnections_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().ClearCellularDataConnections(0);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   GetDataConnApnAttr_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, GetDataConnApnAttr_001, TestSize.Level0)
{
    ApnItem::Attribute apnAttr;
    int32_t result = CellularDataClient::GetInstance().GetDataConnApnAttr(0, apnAttr);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.number   GetDataConnIpType_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, GetDataConnIpType_001, TestSize.Level0)
{
    std::string ipType;
    int32_t result = CellularDataClient::GetInstance().GetDataConnIpType(0, ipType);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.number   ClearAllConnections_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, ClearAllConnections_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().ClearAllConnections(0, DisConnectionReason::REASON_NORMAL);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   IsNeedDoRecovery_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, IsNeedDoRecovery_001, TestSize.Level0)
{
    bool needDoRecovery = false;
    CellularDataClient::GetInstance().IsNeedDoRecovery(0, needDoRecovery);
    EXPECT_EQ(needDoRecovery, false);
}

/**
 * @tc.number   SetDefaultCellularDataSlotId_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, SetDefaultCellularDataSlotId_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(0);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   SetDefaultCellularDataSlotId_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, SetDefaultCellularDataSlotId_002, TestSize.Level0)
{
    DataAccessToken token;
    int32_t result = CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(0);
    EXPECT_NE(result, 0);
}

/**
 * @tc.number   EstablishAllApnsIfConnectable_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, EstablishAllApnsIfConnectable_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().EstablishAllApnsIfConnectable(0);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   ReleaseCellularDataConnection_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, ReleaseCellularDataConnection_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().ReleaseCellularDataConnection(0);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   GetCellularDataSupplierId_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, GetCellularDataSupplierId_001, TestSize.Level0)
{
    uint32_t supplierId = 0;
    int32_t result = CellularDataClient::GetInstance().GetCellularDataSupplierId(0, 0, supplierId);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   CorrectNetSupplierNoAvailable_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, CorrectNetSupplierNoAvailable_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().CorrectNetSupplierNoAvailable(0);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   GetSupplierRegisterState_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, GetSupplierRegisterState_001, TestSize.Level0)
{
    int32_t regState = 0;
    int32_t result = CellularDataClient::GetInstance().GetSupplierRegisterState(0, regState);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   OnInitCellularDataController_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataClientTest, OnInitCellularDataController_001, TestSize.Level0)
{
    int32_t result = CellularDataClient::GetInstance().InitCellularDataController(0);
    EXPECT_EQ(result, TELEPHONY_ERR_PERMISSION_ERR);
}
} // namespace Telephony
} // namespace OHOS