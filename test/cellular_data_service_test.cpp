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

#include "cellular_data_error.h"
#include "cellular_data_service.h"
#include "data_access_token.h"
#include "gtest/gtest.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
const int32_t DEFAULT_SIM_SLOT_ID = 0;

class CellularDataServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<CellularDataService> service = DelayedSingleton<CellularDataService>::GetInstance();
};
void CellularDataServiceTest::SetUpTestCase() {}

void CellularDataServiceTest::TearDownTestCase() {}

void CellularDataServiceTest::SetUp() {}

void CellularDataServiceTest::TearDown() {}

/**
 * @tc.number   CellularDataService_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, CellularDataService_001, TestSize.Level0)
{
    DataAccessToken token;
    service->OnStart();
    service->isInitSuccess_ = true;
    bool dataEnabled = false;
    bool dataRoamingEnabled = false;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->IsCellularDataEnabled(dataEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableCellularData(false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableCellularData(true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableIntelligenceSwitch(false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableIntelligenceSwitch(true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->GetCellularDataState());
    ASSERT_EQ(static_cast<int32_t>(DisConnectionReason::REASON_CHANGE_CONNECTION),
        service->GetApnState(DEFAULT_SIM_SLOT_ID, std::string()));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->IsCellularDataRoamingEnabled(DEFAULT_SIM_SLOT_ID, dataRoamingEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableCellularDataRoaming(DEFAULT_SIM_SLOT_ID, true));
    NetRequest request;
    request.ident = "simId12";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->RequestNet(request));
    request.ident = "simId2";
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->ReleaseNet(request));
    auto event = AppExecFwk::InnerEvent::Get(0);
    service->DispatchEvent(DEFAULT_SIM_SLOT_ID, event);
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->HandleApnChanged(DEFAULT_SIM_SLOT_ID));
}

/**
 * @tc.number   CellularDataService_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, CellularDataService_002, TestSize.Level0)
{
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->GetCellularDataFlowType());
    ASSERT_EQ("default slotId: -1", service->GetStateMachineCurrentStatusDump());
    service->GetFlowDataInfoDump();
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->StrategySwitch(DEFAULT_SIM_SLOT_ID, false));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->HasInternetCapability(DEFAULT_SIM_SLOT_ID, 0));
    DisConnectionReason reason = DisConnectionReason::REASON_NORMAL;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->ClearAllConnections(DEFAULT_SIM_SLOT_ID, reason));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, false));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, true));
    ApnItem::Attribute apnAttr;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->GetDataConnApnAttr(DEFAULT_SIM_SLOT_ID, apnAttr));
    std::string ipType;
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->GetDataConnIpType(DEFAULT_SIM_SLOT_ID, ipType));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->IsNeedDoRecovery(DEFAULT_SIM_SLOT_ID, true));
    service->OnStop();
}
} // namespace Telephony
} // namespace OHOS