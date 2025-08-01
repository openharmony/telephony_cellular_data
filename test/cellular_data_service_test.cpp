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
#include "data_connection_monitor.h"
#include "gtest/gtest.h"
#include "tel_ril_network_parcel.h"
#include "traffic_management.h"
#include "apn_attribute.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

static const int32_t SLEEP_TIME = 3;

class CellularDataServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<CellularDataService> service = DelayedSingleton<CellularDataService>::GetInstance();
};
void CellularDataServiceTest::SetUpTestCase() {}

void CellularDataServiceTest::TearDownTestCase()
{
    sleep(SLEEP_TIME);
}

void CellularDataServiceTest::SetUp() {}

void CellularDataServiceTest::TearDown() {}

/**
 * @tc.number   CellularDataService_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, CellularDataService_001, TestSize.Level1)
{
    DataAccessToken token;
    service->OnStart();
    service->isInitSuccess_ = true;
    bool dataEnabled = false;
    bool dataRoamingEnabled = false;
    int32_t state;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->IsCellularDataEnabled(dataEnabled));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableCellularData(false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableCellularData(true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableIntelligenceSwitch(false));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->EnableIntelligenceSwitch(true));
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->GetCellularDataState(state));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->GetApnState(DEFAULT_SIM_SLOT_ID, std::string(), state));
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
HWTEST_F(CellularDataServiceTest, CellularDataService_002, TestSize.Level1)
{
    int32_t type;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->GetCellularDataFlowType(type));
    ASSERT_EQ("default slotId: -1", service->GetStateMachineCurrentStatusDump());
    service->GetFlowDataInfoDump();
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->StrategySwitch(DEFAULT_SIM_SLOT_ID, false));
    int32_t capability;
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->HasInternetCapability(DEFAULT_SIM_SLOT_ID, 0, capability));
    int32_t reason = static_cast<int32_t>(DisConnectionReason::REASON_NORMAL);
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->ClearAllConnections(DEFAULT_SIM_SLOT_ID, reason));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, false));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->ChangeConnectionForDsds(DEFAULT_SIM_SLOT_ID, true));
    ApnAttribute apnAttr;
    ASSERT_NE(TELEPHONY_ERR_SUCCESS, service->GetDataConnApnAttr(DEFAULT_SIM_SLOT_ID, apnAttr));
    std::string ipType;
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->GetDataConnIpType(DEFAULT_SIM_SLOT_ID, ipType));
    ASSERT_EQ(TELEPHONY_ERR_SUCCESS, service->IsNeedDoRecovery(DEFAULT_SIM_SLOT_ID, true));
    uint32_t supplierId = 0;
    ASSERT_EQ(TELEPHONY_ERR_PERMISSION_ERR,
        service->GetCellularDataSupplierId(DEFAULT_SIM_SLOT_ID, NetCap::NET_CAPABILITY_INTERNET, supplierId));
    ASSERT_EQ(TELEPHONY_ERR_PERMISSION_ERR,
        service->GetCellularDataSupplierId(DEFAULT_SIM_SLOT_ID, NetCap::NET_CAPABILITY_END, supplierId));
    service->CorrectNetSupplierNoAvailable(DEFAULT_SIM_SLOT_ID);
    ASSERT_EQ(TELEPHONY_ERR_PERMISSION_ERR, service->EstablishAllApnsIfConnectable(DEFAULT_SIM_SLOT_ID));
    int32_t slotId = 0;
    std::vector<uint8_t> buffer = {};
    ASSERT_EQ(TELEPHONY_ERR_LOCAL_PTR_NULL, service->SendUrspDecodeResult(slotId, buffer));
    ASSERT_EQ(TELEPHONY_ERR_LOCAL_PTR_NULL, service->SendUePolicySectionIdentifier(slotId, buffer));
    ASSERT_EQ(TELEPHONY_ERR_LOCAL_PTR_NULL, service->SendImsRsdList(slotId, buffer));
    ASSERT_EQ(TELEPHONY_ERR_LOCAL_PTR_NULL, service->GetNetworkSliceAllowedNssai(slotId, buffer));
    ASSERT_EQ(TELEPHONY_ERR_LOCAL_PTR_NULL, service->GetNetworkSliceEhplmn(slotId));
    int32_t regState = -1;
    service->GetSupplierRegisterState(supplierId, regState);
    bool isSupportDun = false;
    EXPECT_EQ(TELEPHONY_ERR_PERMISSION_ERR, service->GetIfSupportDunApn(isSupportDun));
    service->OnStop();
}

/**
 * @tc.number   DataConnectionMonitor_HandleScreenStateChanged_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, DataConnectionMonitor_HandleScreenStateChanged_001, TestSize.Level0)
{
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    dataConnectionMonitor->isScreenOn_ = false;
    dataConnectionMonitor->HandleScreenStateChanged(true);
    ASSERT_EQ(dataConnectionMonitor->isScreenOn_, true);
    dataConnectionMonitor->isScreenOn_ = false;
    dataConnectionMonitor->HandleScreenStateChanged(false);
    ASSERT_EQ(dataConnectionMonitor->isScreenOn_, false);
}

/**
 * @tc.number   DataConnectionMonitor_OnStallDetectionTimer_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, DataConnectionMonitor_OnStallDetectionTimer_001, TestSize.Level1)
{
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    dataConnectionMonitor->noRecvPackets_ = 20;
    dataConnectionMonitor->stallDetectionEnabled_ = true;
    dataConnectionMonitor->OnStallDetectionTimer();
    ASSERT_EQ(dataConnectionMonitor->noRecvPackets_, 20);
}

/**
 * @tc.number   DataConnectionMonitor_HandleRecovery_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, DataConnectionMonitor_HandleRecovery_001, TestSize.Level0)
{
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    dataConnectionMonitor->dataRecoveryState_ = RecoveryState::STATE_REQUEST_CONTEXT_LIST;
    dataConnectionMonitor->HandleRecovery();
    ASSERT_EQ(dataConnectionMonitor->dataRecoveryState_, RecoveryState::STATE_CLEANUP_CONNECTIONS);
    dataConnectionMonitor->dataRecoveryState_ = RecoveryState::STATE_REREGISTER_NETWORK;
    dataConnectionMonitor->HandleRecovery();
    ASSERT_EQ(dataConnectionMonitor->dataRecoveryState_, RecoveryState::STATE_RADIO_STATUS_RESTART);
    dataConnectionMonitor->dataRecoveryState_ = RecoveryState::STATE_RADIO_STATUS_RESTART;
    dataConnectionMonitor->HandleRecovery();
    ASSERT_EQ(dataConnectionMonitor->dataRecoveryState_, RecoveryState::STATE_REQUEST_CONTEXT_LIST);
}

/**
 * @tc.number   DataConnectionMonitor_EndNetStatistics_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, DataConnectionMonitor_EndNetStatistics_001, TestSize.Level0)
{
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    dataConnectionMonitor->dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_DOWN;
    dataConnectionMonitor->EndNetStatistics();
    ASSERT_EQ(dataConnectionMonitor->dataFlowType_, CellDataFlowType::DATA_FLOW_TYPE_NONE);
}

/**
 * @tc.number   DataConnectionMonitor_UpdateNetTrafficState_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, DataConnectionMonitor_UpdateNetTrafficState_001, TestSize.Level0)
{
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    dataConnectionMonitor->updateNetStat_ = true;
    dataConnectionMonitor->UpdateNetTrafficState();
    std::shared_ptr<PreferredNetworkTypeInfo> preferredTypeInfo = std::make_shared<PreferredNetworkTypeInfo>();
    auto event = AppExecFwk::InnerEvent::Get(0, preferredTypeInfo);
    dataConnectionMonitor->SetPreferredNetworkPara(event);
    ASSERT_EQ(dataConnectionMonitor->dataFlowType_, CellDataFlowType::DATA_FLOW_TYPE_NONE);
}

/**
 * @tc.number   DataConnectionMonitor_UpdateDataFlowType_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, DataConnectionMonitor_UpdateDataFlowType_001, TestSize.Level0)
{
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    dataConnectionMonitor->dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_DOWN;
    dataConnectionMonitor->trafficManager_->sendPackets_ = 200;
    dataConnectionMonitor->trafficManager_->recvPackets_ = 100;
    dataConnectionMonitor->UpdateDataFlowType();
    ASSERT_EQ(static_cast<int32_t>(dataConnectionMonitor->dataFlowType_),
        static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE));
}

/**
 * @tc.number   DataConnectionMonitor_SetDataFlowType_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, DataConnectionMonitor_SetDataFlowType_001, TestSize.Level0)
{
    std::shared_ptr<DataConnectionMonitor> dataConnectionMonitor = std::make_shared<DataConnectionMonitor>(0);
    auto event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_RUN_MONITOR_TASK);
    dataConnectionMonitor->ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(CellularDataEventCode::MSG_STALL_DETECTION_EVENT_ID);
    dataConnectionMonitor->ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_DATA_CALL_LIST_CHANGED);
    dataConnectionMonitor->ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_GET_PREFERRED_NETWORK_MODE);
    dataConnectionMonitor->ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_SET_PREFERRED_NETWORK_MODE);
    dataConnectionMonitor->ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_OFF);
    dataConnectionMonitor->ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(RadioEvent::RADIO_ON);
    dataConnectionMonitor->ProcessEvent(event);
    dataConnectionMonitor->dataFlowType_ = CellDataFlowType::DATA_FLOW_TYPE_NONE;
    dataConnectionMonitor->SetDataFlowType(CellDataFlowType::DATA_FLOW_TYPE_DOWN);
    ASSERT_EQ(static_cast<int32_t>(dataConnectionMonitor->dataFlowType_),
        static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN));
}

/**
 * @tc.number   CellularDataController_SetIntelligenceSwitchEnable_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, CellularDataController_SetIntelligenceSwitchEnable_001, TestSize.Level0)
{
    std::shared_ptr<CellularDataController> cellularDataController = std::make_shared<CellularDataController>(0);
    cellularDataController->cellularDataHandler_ = nullptr;
    int32_t result = cellularDataController->SetIntelligenceSwitchEnable(true);
    ASSERT_EQ(result, TELEPHONY_ERR_LOCAL_PTR_NULL);
}

/**
 * @tc.number   CellularDataController_SetIntelligenceSwitchEnable_002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, CellularDataController_SetIntelligenceSwitchEnable_002, TestSize.Level0)
{
    std::shared_ptr<CellularDataController> cellularDataController = std::make_shared<CellularDataController>(0);
    cellularDataController->Init();
    int32_t result = cellularDataController->SetIntelligenceSwitchEnable(false);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.number   CellularDataController_OnAddSystemAbility_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, CellularDataController_OnAddSystemAbility_001, TestSize.Level0)
{
    std::shared_ptr<CellularDataController> cellularDataController = std::make_shared<CellularDataController>(0);
    cellularDataController->systemAbilityListener_ =
        new (std::nothrow) CellularDataController::SystemAbilityStatusChangeListener(0, nullptr);
    if (cellularDataController->systemAbilityListener_ != nullptr) {
        cellularDataController->systemAbilityListener_->OnRemoveSystemAbility(
            COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, "");
        cellularDataController->systemAbilityListener_->OnRemoveSystemAbility(
            COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, "");
        cellularDataController->systemAbilityListener_->OnRemoveSystemAbility(
            DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        cellularDataController->systemAbilityListener_->OnRemoveSystemAbility(-1, "");
    }
    ASSERT_EQ(cellularDataController->cellularDataHandler_, nullptr);
}

/**
 * @tc.number   RemoveUid_ShouldReturnInvalidParam_WhenIdentLengthLessThanIdentPreLen
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, RemoveUid_WhenIdentLengthLessThanIdentPreLen, TestSize.Level1)
{
    NetRequest request;
    request.ident = "";
    int32_t result = service->RemoveUid(request);
    EXPECT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   RemoveUid_ShouldReturnInvalidParam_WhenIdentIsNotValidDecValue
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, RemoveUid_WhenIdentIsNotValidDecValue, TestSize.Level1)
{
    NetRequest request;
    request.ident = "simIdABC";
    int32_t result = service->RemoveUid(request);
    EXPECT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   RemoveUid_ShouldReturnInvalidParam_WhenCellularDataControllerIsNull
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, RemoveUid_WhenCellularDataControllerIsNull, TestSize.Level1)
{
    NetRequest request;
    request.ident = "simId1";
    int32_t result = service->RemoveUid(request);
    EXPECT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   AddUid_ShouldReturnInvalidParam_WhenIdentLengthLessThanIdentPreLen
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, AddUid_WhenIdentLengthLessThanIdentPreLen, TestSize.Level1)
{
    NetRequest request;
    request.ident = "";
    int32_t result = service->AddUid(request);
    EXPECT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   AddUid_ShouldReturnInvalidParam_WhenIdentIsNotValidDecValue
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, AddUid_WhenIdentIsNotValidDecValue, TestSize.Level1)
{
    NetRequest request;
    request.ident = "simIdABC";
    int32_t result = service->AddUid(request);
    EXPECT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   AddUid_ShouldReturnInvalidParam_WhenCellularDataControllerIsNull
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, AddUid_WhenCellularDataControllerIsNull, TestSize.Level1)
{
    NetRequest request;
    request.ident = "simId1";
    int32_t result = service->AddUid(request);
    EXPECT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   RemoveOrAddUidTest001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, RemoveOrAddUidTest001, TestSize.Level0)
{
    NetRequest request;
    request.ident = "simId123456789123";
    EXPECT_EQ(service->AddUid(request), CELLULAR_DATA_INVALID_PARAM);
    EXPECT_EQ(service->RemoveUid(request), CELLULAR_DATA_INVALID_PARAM);
    request.ident = "simId12";
    EXPECT_EQ(service->AddUid(request), CELLULAR_DATA_INVALID_PARAM);
    EXPECT_EQ(service->RemoveUid(request), CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   EstablishAllApnsIfConnectableTest001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, EstablishAllApnsIfConnectableTest001, TestSize.Level0)
{
    int32_t slotId = -1;
    EXPECT_EQ(service->EstablishAllApnsIfConnectable(slotId), TELEPHONY_ERR_PERMISSION_ERR);
    EXPECT_EQ(service->ReleaseCellularDataConnection(slotId), TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   EstablishAllApnsIfConnectableTest002
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, EstablishAllApnsIfConnectableTest002, TestSize.Level0)
{
    DataAccessToken token;
    int32_t slotId = -1;
    EXPECT_EQ(service->EstablishAllApnsIfConnectable(slotId), CELLULAR_DATA_INVALID_PARAM);
    EXPECT_EQ(service->ReleaseCellularDataConnection(slotId), CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   GetActReportInfoTest001
 * @tc.name     test GetDefaultActReportInfo and GetInternalActReportInfo
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, GetActReportInfoTest001, TestSize.Level0)
{
    ApnActivateReportInfoIpc infoIpc;
    int32_t slotId = -1;
    EXPECT_EQ(service->GetDefaultActReportInfo(slotId, infoIpc), TELEPHONY_ERR_PERMISSION_ERR);
    EXPECT_EQ(service->GetInternalActReportInfo(slotId, infoIpc), TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   GetActReportInfoTest002
 * @tc.name     test GetDefaultActReportInfo and GetInternalActReportInfo
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, GetActReportInfoTest002, TestSize.Level0)
{
    DataAccessToken token;
    ApnActivateReportInfoIpc infoIpc;
    int32_t slotId = -1;
    EXPECT_EQ(service->GetDefaultActReportInfo(slotId, infoIpc), CELLULAR_DATA_INVALID_PARAM);
    EXPECT_EQ(service->GetInternalActReportInfo(slotId, infoIpc), CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   ApnInfoTest001
 * @tc.name     test QueryApnIds, SetPreferApn and QueryAllApnInfo
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, ApnInfoTest001, TestSize.Level0)
{
    ApnInfo apnInfo;
    std::vector<uint32_t> apnIdList;
    std::vector<ApnInfo> allApnInfoList;
    int32_t apnId = 1;
    EXPECT_EQ(service->QueryApnIds(apnInfo, apnIdList), TELEPHONY_ERR_PERMISSION_ERR);
    EXPECT_EQ(service->SetPreferApn(apnId), TELEPHONY_ERR_PERMISSION_ERR);
    EXPECT_EQ(service->QueryAllApnInfo(allApnInfoList), TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   ApnInfoTest002
 * @tc.name     test QueryApnIds, SetPreferApn and QueryAllApnInfo
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, ApnInfoTest002, TestSize.Level0)
{
    DataAccessToken token;
    ApnInfo apnInfo;
    std::vector<uint32_t> apnIdList;
    std::vector<ApnInfo> allApnInfoList;
    int32_t apnId = 1;
    EXPECT_EQ(service->QueryApnIds(apnInfo, apnIdList), 0);
    EXPECT_EQ(service->SetPreferApn(apnId), -1);
    EXPECT_EQ(service->QueryAllApnInfo(allApnInfoList), 0);
}

/**
 * @tc.number   GetActiveApnName001
 * @tc.name     test GetActiveApnName
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, GetActiveApnName001, TestSize.Level0)
{
    std::string apnName;
    EXPECT_EQ(service->GetActiveApnName(apnName), TELEPHONY_ERR_PERMISSION_ERR);
}

/**
 * @tc.number   GetActiveApnName002
 * @tc.name     test GetActiveApnName
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, GetActiveApnName002, TestSize.Level0)
{
    DataAccessToken token;
    std::string apnName;
    EXPECT_EQ(service->GetActiveApnName(apnName), 0);
}

/**
 * @tc.number   ReleaseNet_001
 * @tc.name     test function branch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataServiceTest, ReleaseNet_001, TestSize.Level1)
{
    NetRequest request;
    request.ident = "1";
    ASSERT_EQ(CELLULAR_DATA_INVALID_PARAM, service->RequestNet(request));
    ASSERT_EQ(CELLULAR_DATA_INVALID_PARAM, service->ReleaseNet(request));
}
} // namespace Telephony
} // namespace OHOS