/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <string>

#define private public
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <dlfcn.h>

#include "cellular_data_client.h"
#include "cellular_data_controller.h"
#include "cellular_data_error.h"
#include "cellular_data_net_agent.h"
#include "cellular_data_service.h"
#include "cellular_data_types.h"
#include "core_service_client.h"
#include "data_access_token.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/gtest_pred_impl.h"
#include "gtest/hwext/gtest-tag.h"
#include "hap_token_info.h"
#include "icellular_data_manager.h"
#include "iosfwd"
#include "iostream"
#include "net_conn_callback_stub.h"
#include "net_supplier_callback_base.h"
#include "net_conn_client.h"
#include "net_handle.h"
#include "net_specifier.h"
#include "ostream"
#include "permission_def.h"
#include "permission_state_full.h"
#include "refbase.h"
#include "telephony_ext_wrapper.h"
#include "telephony_types.h"
#include "token_setproc.h"
#include "unistd.h"
#include "apn_item.h"
#include "cellular_data_constant.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace Telephony {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::NetManagerStandard;
using ::testing::NiceMock;

static const int32_t SLEEP_TIME = 1;
static const int32_t SIM_SLOT_ID_1 = DEFAULT_SIM_SLOT_ID + 1;
static const int32_t DATA_SLOT_ID_INVALID = DEFAULT_SIM_SLOT_ID + 10;
static const int32_t PING_CHECK_SUCCESS = 0;
static const int32_t PING_CHECK_FAIL = 1;
static const int32_t MAX_TIMES = 60;
static const int32_t CMD_BUF_SIZE = 10240;
static const int32_t NET_REGISTER_TIMEOUT_MS = 20000;
static const int32_t SLEEP_TIME_SECONDS = 3;

class TestCallback : public NetManagerStandard::NetConnCallbackStub {
    int32_t NetAvailable(sptr<NetManagerStandard::NetHandle> &netHandle) override
    {
        isCallback_ = true;
        std::cout << "TestCallback::NetAvailable" << std::endl;
        return 0;
    }

    int32_t NetCapabilitiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
        const sptr<NetManagerStandard::NetAllCapabilities> &netAllCap) override
    {
        isCallback_ = true;
        std::cout << "TestCallback::NetCapabilitiesChange" << std::endl;
        return 0;
    }

    int32_t NetConnectionPropertiesChange(
        sptr<NetManagerStandard::NetHandle> &netHandle, const sptr<NetManagerStandard::NetLinkInfo> &info) override
    {
        isCallback_ = true;
        std::cout << "TestCallback::NetConnectionPropertiesChange" << std::endl;
        return 0;
    }

    int32_t NetLost(sptr<NetManagerStandard::NetHandle> &netHandle) override
    {
        isCallback_ = true;
        std::cout << "TestCallback::NetLost" << std::endl;
        return 0;
    }

    int32_t NetUnavailable() override
    {
        isCallback_ = true;
        std::cout << "TestCallback::NetUnavailable" << std::endl;
        return 0;
    }

    int32_t NetBlockStatusChange(sptr<NetManagerStandard::NetHandle> &netHandle, bool blocked) override
    {
        isCallback_ = true;
        std::cout << "TestCallback::NetBlockStatusChange" << std::endl;
        return 0;
    }

public:
    bool isCallback_ = false;
};

class MockDlsym {
public:
    MOCK_METHOD(void *, dlopen, (const char *fileName, int flag));
    MOCK_METHOD(void *, dlsym, (void *handle, const char *symbol));
};

NiceMock<MockDlsym> *mockDlsym;

extern "C" {
// mock dlopen
void *dlopen(const char *fileName, int flag)
{
    if (mockDlsym == nullptr) {
        mockDlsym = new NiceMock<MockDlsym>();
    }
    return mockDlsym->dlopen(fileName, flag);
}

// mock dlsym
void *dlsym(void *handle, const char *symbol)
{
    if (mockDlsym == nullptr) {
        mockDlsym = new NiceMock<MockDlsym>();
    }
    return mockDlsym->dlsym(handle, symbol);
}
}

class CellularDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();
    static bool HasSimCard(const int32_t slotId);
    static int32_t IsCellularDataEnabledTest(bool &dataEnabled);
    static int32_t EnableCellularDataTest(bool enable);
    static int32_t EnableIntelligenceSwitchTest(bool enable);
    static int32_t GetCellularDataStateTest();
    static int32_t IsCellularDataRoamingEnabledTest(int32_t slotId, bool &dataRoamingEnabled);
    static int32_t EnableCellularDataRoamingTest(int32_t slotId, bool enable);
    static int32_t GetDefaultCellularDataSlotIdTest();
    static int32_t GetDefaultCellularDataSimIdTest();
    static int32_t SetDefaultCellularDataSlotIdTest(int32_t slotId);
    static int32_t GetCellularDataFlowTypeTest();
    static void WaitTestTimeout(const int32_t status);
    static sptr<ICellularDataManager> GetProxy();
    static string GetCmdResult();
    static int32_t PingTest();
    static int32_t HasInternetCapability(int32_t slotId, int32_t cid);
    static int32_t ClearCellularDataConnections(int32_t slotId);
    static int32_t ClearAllConnections(int32_t slotId, DisConnectionReason reason);
    static int32_t GetApnState(int32_t slotId, const std::string &apnTyp);
    static int32_t GetDataRecoveryState();
    static int32_t GetDataConnApnAttr(int32_t slotId, ApnItem::Attribute &apnAttr);
    static int32_t GetDataConnIpType(int32_t slotId, std::string &ipType);
    static int32_t IsNeedDoRecovery(int32_t slotId, bool needDoRecovery);
    static int32_t InitCellularDataController(int32_t slotId);
    static int32_t GetIntelligenceSwitchStateTest(bool &state);
    static int32_t GetCellularDataSupplierId(int32_t slotId, uint64_t capability, uint32_t &supplierId);
    static int32_t CorrectNetSupplierNoAvailable(int32_t slotid);
    static int32_t GetSupplierRegisterState(uint32_t supplierId, int32_t &regState);
    CellularDataNetAgent &netAgent = CellularDataNetAgent::GetInstance();
};

bool CellularDataTest::HasSimCard(const int32_t slotId)
{
    bool hasSimCard = false;
    DelayedRefSingleton<CoreServiceClient>::GetInstance().HasSimCard(slotId, hasSimCard);
    return hasSimCard;
}

void CellularDataTest::TearDownTestCase()
{
    if (CoreServiceClient::GetInstance().GetProxy() == nullptr) {
        std::cout << "connect coreService server failed!" << std::endl;
        return;
    }
    DataAccessToken token;
    int32_t slotId = DATA_SLOT_ID_INVALID;
    if (HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        slotId = DEFAULT_SIM_SLOT_ID;
    } else if (HasSimCard(SIM_SLOT_ID_1)) {
        slotId = SIM_SLOT_ID_1;
    }
    if (slotId == DATA_SLOT_ID_INVALID) {
        return;
    }
    // Set the default slot
    int32_t result = CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(slotId);
    if (result != TELEPHONY_ERR_SUCCESS) {
        return;
    }
    int32_t enable = CellularDataClient::GetInstance().EnableCellularData(true);
    ASSERT_TRUE(enable == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    sleep(SLEEP_TIME_SECONDS);
}

void CellularDataTest::SetUp() {}

void CellularDataTest::TearDown() {}

void CellularDataTest::SetUpTestCase()
{
    if (CoreServiceClient::GetInstance().GetProxy() == nullptr) {
        std::cout << "connect coreService server failed!" << std::endl;
        return;
    }
    DataAccessToken token;
    int32_t slotId = DATA_SLOT_ID_INVALID;
    if (HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        slotId = DEFAULT_SIM_SLOT_ID;
    } else if (HasSimCard(SIM_SLOT_ID_1)) {
        slotId = SIM_SLOT_ID_1;
    }
    if (slotId == DATA_SLOT_ID_INVALID) {
        return;
    }
    // Set the default slot
    int32_t result = CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(slotId);
    if (result != TELEPHONY_ERR_SUCCESS) {
        return;
    }
    int32_t enable = CellularDataClient::GetInstance().EnableCellularData(true);
    ASSERT_TRUE(enable == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
}

void CellularDataTest::WaitTestTimeout(const int32_t status)
{
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (CellularDataClient::GetInstance().GetCellularDataState() == status) {
            return;
        }
        count++;
    }
}

string CellularDataTest::GetCmdResult()
{
    string strCmd = "ping -c3 www.openharmony.cn";
    char buf[CMD_BUF_SIZE] = { 0 };
    FILE *pf;

    if ((pf = popen(strCmd.c_str(), "r")) == nullptr) {
        return "";
    }
    string strResult;
    while (fgets(buf, sizeof(buf), pf) != nullptr) {
        strResult += buf;
    }
    pclose(pf);
    unsigned int iSize = strResult.size();
    if (iSize > 0 && strResult[iSize - 1] == '\n') {
        strResult = strResult.substr(0, iSize - 1);
    }
    return strResult;
}

int32_t CellularDataTest::PingTest()
{
    string strRe = GetCmdResult();
    std::cout << strRe << std::endl;

    // if ping succeed, the result should contains something like:
    // 3 packets transmitted, 3 received, 0% packet loss, time 5440ms
    if (strRe.find("3 packets") != string::npos) {
        return PING_CHECK_SUCCESS;
    } else {
        return PING_CHECK_FAIL;
    }
}

int32_t CellularDataTest::IsCellularDataRoamingEnabledTest(int32_t slotId, bool &dataRoamingEnabled)
{
    return CellularDataClient::GetInstance().IsCellularDataRoamingEnabled(slotId, dataRoamingEnabled);
}

int32_t CellularDataTest::IsCellularDataEnabledTest(bool &dataEnabled)
{
    return CellularDataClient::GetInstance().IsCellularDataEnabled(dataEnabled);
}

int32_t CellularDataTest::EnableCellularDataTest(bool enable)
{
    return CellularDataClient::GetInstance().EnableCellularData(enable);
}

int32_t CellularDataTest::EnableIntelligenceSwitchTest(bool enable)
{
    return CellularDataClient::GetInstance().EnableIntelligenceSwitch(enable);
}

int32_t CellularDataTest::GetCellularDataStateTest()
{
    return CellularDataClient::GetInstance().GetCellularDataState();
}

int32_t CellularDataTest::GetIntelligenceSwitchStateTest(bool &state)
{
    return CellularDataClient::GetInstance().GetIntelligenceSwitchState(state);
}

int32_t CellularDataTest::EnableCellularDataRoamingTest(int32_t slotId, bool enable)
{
    return CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, enable);
}

int32_t CellularDataTest::GetDefaultCellularDataSlotIdTest()
{
    return CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
}

int32_t CellularDataTest::GetDefaultCellularDataSimIdTest()
{
    int32_t simId = 0;
    return CellularDataClient::GetInstance().GetDefaultCellularDataSimId(simId);
}

int32_t CellularDataTest::SetDefaultCellularDataSlotIdTest(int32_t slotId)
{
    return CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(slotId);
}

int32_t CellularDataTest::GetCellularDataFlowTypeTest()
{
    return CellularDataClient::GetInstance().GetCellularDataFlowType();
}

int32_t CellularDataTest::HasInternetCapability(int32_t slotId, int32_t cid)
{
    CellularDataClient::GetInstance().IsConnect();
    return CellularDataClient::GetInstance().HasInternetCapability(slotId, cid);
}

int32_t CellularDataTest::ClearCellularDataConnections(int32_t slotId)
{
    CellularDataClient::GetInstance().IsConnect();
    return CellularDataClient::GetInstance().ClearCellularDataConnections(slotId);
}

int32_t CellularDataTest::ClearAllConnections(int32_t slotId, DisConnectionReason reason)
{
    return CellularDataClient::GetInstance().ClearAllConnections(slotId, reason);
}

int32_t CellularDataTest::GetApnState(int32_t slotId, const std::string &apnTyp)
{
    return CellularDataClient::GetInstance().GetApnState(slotId, apnTyp);
}

int32_t CellularDataTest::GetDataRecoveryState()
{
    return CellularDataClient::GetInstance().GetDataRecoveryState();
}

int32_t CellularDataTest::GetDataConnApnAttr(int32_t slotId, ApnItem::Attribute &apnAttr)
{
    return CellularDataClient::GetInstance().GetDataConnApnAttr(slotId, apnAttr);
}

int32_t CellularDataTest::GetDataConnIpType(int32_t slotId, std::string &ipType)
{
    return CellularDataClient::GetInstance().GetDataConnIpType(slotId, ipType);
}

int32_t CellularDataTest::IsNeedDoRecovery(int32_t slotId, bool needDoRecovery)
{
    return CellularDataClient::GetInstance().IsNeedDoRecovery(slotId, needDoRecovery);
}

int32_t CellularDataTest::InitCellularDataController(int32_t slotId)
{
    return CellularDataClient::GetInstance().InitCellularDataController(slotId);
}

int32_t CellularDataTest::GetCellularDataSupplierId(int32_t slotId, uint64_t capability, uint32_t &supplierId)
{
    return CellularDataClient::GetInstance().GetCellularDataSupplierId(slotId, capability, supplierId);
}

int32_t CellularDataTest::CorrectNetSupplierNoAvailable(int32_t slotId)
{
    return CellularDataClient::GetInstance().CorrectNetSupplierNoAvailable(slotId);
}

int32_t CellularDataTest::GetSupplierRegisterState(uint32_t supplierId, int32_t &regState)
{
    return CellularDataClient::GetInstance().GetSupplierRegisterState(supplierId, regState);
}
#ifndef TEL_TEST_UNSUPPORT
/**
 * @tc.number   IsCellularDataEnabled_Test
 * @tc.name     Test cellular data switch status(enabled or disabled)
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, IsCellularDataEnabled_Test, TestSize.Level1)
{
    DataAccessToken token;
    bool dataEnabled = false;
    CellularDataTest::IsCellularDataEnabledTest(dataEnabled);
    ASSERT_TRUE(dataEnabled >= static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED));
}

/**
 * @tc.number   DefaultCellularDataSlotId_Test
 * @tc.name     Test set default data card slot
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DefaultCellularDataSlotId_Test, TestSize.Level2)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::GetDefaultCellularDataSlotIdTest();
    if (result < DEFAULT_SIM_SLOT_ID_REMOVE) {
        return;
    }
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    // Multiple cards will need to be optimized again
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID - 1);
    ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   GetDefaultCellularDataSimId
 * @tc.name     Test get default data sim id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DefaultCellularDataSimId_Test, TestSize.Level2)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    int32_t result = CellularDataTest::GetDefaultCellularDataSimIdTest();
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   DefaultCellularDataSlotId_Test_01
 * @tc.name     Test set default data card slot
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DefaultCellularDataSlotId_Test_01, TestSize.Level2)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::GetDefaultCellularDataSlotIdTest();
    if (result < DEFAULT_SIM_SLOT_ID_REMOVE) {
        return;
    }
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(SIM_SLOT_ID_1);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   EnableCellularData_Test_01
 * @tc.name     Test cellular data switch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, EnableCellularData_Test_01, TestSize.Level2)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);
    int32_t result = CellularDataTest::EnableCellularDataTest(true);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Connected Ping..." << std::endl;
    int32_t pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult >= PING_CHECK_SUCCESS);
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Disconnected Ping..." << std::endl;
    pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult == PING_CHECK_FAIL);
}

/**
 * @tc.number   EnableCellularData_Test_02
 * @tc.name     Test cellular data switch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, EnableCellularData_Test_02, TestSize.Level2)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(SIM_SLOT_ID_1);
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);
    int32_t result = CellularDataTest::EnableCellularDataTest(true);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Connected Ping..." << std::endl;
    int32_t pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult >= PING_CHECK_SUCCESS);
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Disconnected Ping..." << std::endl;
    pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult == PING_CHECK_FAIL);
}

/**
 * @tc.number   DataRoamingState_ValidSlot_Test_01
 * @tc.name     Test the cellular data roaming switch with a slot id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataRoamingState_ValidSlot_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    int32_t disabled = CellularDataTest::EnableCellularDataTest(false);
    ASSERT_TRUE(disabled == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));

    // slot0 enable data roaming
    int32_t enabled = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, true);
    ASSERT_TRUE(enabled == TELEPHONY_ERR_SUCCESS);
    bool dataRoamingEnabled = false;
    CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID, dataRoamingEnabled);
    ASSERT_TRUE(dataRoamingEnabled);
    // slot0 close
    int32_t enable = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, false);
    ASSERT_TRUE(enable == TELEPHONY_ERR_SUCCESS);
    CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID, dataRoamingEnabled);
    ASSERT_TRUE(!dataRoamingEnabled);

    // At present, multiple card problems, the subsequent need to continue to deal with
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    int32_t result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
    // At present, multiple card problems, the subsequent need to continue to deal with
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   DataRoamingState_ValidSlot_Test_02
 * @tc.name     Test the cellular data roaming switch with a slot id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataRoamingState_ValidSlot_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(SIM_SLOT_ID_1);
    int32_t disabled = CellularDataTest::EnableCellularDataTest(false);
    ASSERT_TRUE(disabled == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));

    // slot1 enable data roaming
    int32_t enabled = CellularDataTest::EnableCellularDataRoamingTest(SIM_SLOT_ID_1, true);
    ASSERT_TRUE(enabled == TELEPHONY_ERR_SUCCESS);
    bool dataRoamingEnabled = false;
    CellularDataTest::IsCellularDataRoamingEnabledTest(SIM_SLOT_ID_1, dataRoamingEnabled);
    ASSERT_TRUE(dataRoamingEnabled);
    // slot1 close
    int32_t enable = CellularDataTest::EnableCellularDataRoamingTest(SIM_SLOT_ID_1, false);
    ASSERT_TRUE(enable == TELEPHONY_ERR_SUCCESS);
    CellularDataTest::IsCellularDataRoamingEnabledTest(SIM_SLOT_ID_1, dataRoamingEnabled);
    ASSERT_TRUE(!dataRoamingEnabled);

    // At present, multiple card problems, the subsequent need to continue to deal with
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    int32_t result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
    // At present, multiple card problems, the subsequent need to continue to deal with
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   EnableCellularDataRoaming_ValidSlot_Test_01
 * @tc.name     Test the cellular data roaming switch with a slot id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, EnableCellularDataRoaming_ValidSlot_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    int32_t disabled = CellularDataTest::EnableCellularDataTest(false);
    ASSERT_TRUE(disabled == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));

    bool dataRoamingEnabled = false;
    CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID, dataRoamingEnabled);
    if (dataRoamingEnabled) {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, false);
        ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    } else {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, true);
        ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    }
    // At present, multiple card problems, the subsequent need to continue to deal with
    CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID, dataRoamingEnabled);
    if (dataRoamingEnabled) {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
        ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
    } else {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
        ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
    }
}

/**
 * @tc.number   EnableCellularDataRoaming_ValidSlot_Test_02
 * @tc.name     Test the cellular data roaming switch with a slot id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, EnableCellularDataRoaming_ValidSlot_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(SIM_SLOT_ID_1);
    int32_t disabled = CellularDataTest::EnableCellularDataTest(false);
    ASSERT_TRUE(disabled == TELEPHONY_ERR_SUCCESS);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));

    bool dataRoamingEnabled = false;
    CellularDataTest::IsCellularDataRoamingEnabledTest(SIM_SLOT_ID_1, dataRoamingEnabled);
    if (dataRoamingEnabled) {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(SIM_SLOT_ID_1, false);
        ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    } else {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(SIM_SLOT_ID_1, true);
        ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    }
    // At present, multiple card problems, the subsequent need to continue to deal with
    CellularDataTest::IsCellularDataRoamingEnabledTest(SIM_SLOT_ID_1, dataRoamingEnabled);
    if (dataRoamingEnabled) {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
        ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
    } else {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
        ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
    }
}

/**
 * @tc.number   GetCellularDataState_ValidityTest_01
 * @tc.name     Test the GetCellularDataState function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetCellularDataState_ValidityTest_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    bool dataEnabled = false;
    CellularDataTest::IsCellularDataEnabledTest(dataEnabled);
    if (dataEnabled) {
        CellularDataTest::EnableCellularDataTest(false);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
        sleep(SLEEP_TIME);
        CellularDataTest::EnableCellularDataTest(true);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
        int32_t result = CellularDataTest::GetCellularDataStateTest();
        ASSERT_TRUE(result == static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    } else {
        CellularDataTest::EnableCellularDataTest(true);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
        sleep(SLEEP_TIME);
        CellularDataTest::EnableCellularDataTest(false);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
        int32_t result = CellularDataTest::GetCellularDataStateTest();
        ASSERT_TRUE(result == static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    }
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
}

/**
 * @tc.number   GetCellularDataState_ValidityTest_02
 * @tc.name     Test the GetCellularDataState function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetCellularDataState_ValidityTest_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(SIM_SLOT_ID_1);
    bool dataEnabled = false;
    CellularDataTest::IsCellularDataEnabledTest(dataEnabled);
    if (dataEnabled) {
        CellularDataTest::EnableCellularDataTest(false);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
        sleep(SLEEP_TIME);
        CellularDataTest::EnableCellularDataTest(true);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
        int32_t result = CellularDataTest::GetCellularDataStateTest();
        ASSERT_TRUE(result == static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    } else {
        CellularDataTest::EnableCellularDataTest(true);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
        sleep(SLEEP_TIME);
        CellularDataTest::EnableCellularDataTest(false);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
        int32_t result = CellularDataTest::GetCellularDataStateTest();
        ASSERT_TRUE(result == static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    }
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
}

/**
 * @tc.number   DataRoamingState_InValidSlot_Test_01
 * @tc.name     Test the EnableCellularDataRoaming function with a invalid slot id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataRoamingState_InValidSlot_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    // invalid slot turn on data roaming
    int32_t enable = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID - 1, true);
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    bool dataRoamingEnabled = false;
    int32_t result = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID - 1, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    // invalid slot disable roaming
    enable = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID - 1, false);
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID - 1, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
    ASSERT_TRUE(enable != TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID, dataRoamingEnabled);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   DataFlowType_Test_01
 * @tc.name     Test the GetCellularDataFlowType function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataFlowType_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);

    CellularDataTest::EnableCellularDataTest(true);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Connected Ping..." << std::endl;
    int32_t pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult >= PING_CHECK_SUCCESS);
    int32_t dataFlowType = CellularDataTest::GetCellularDataFlowTypeTest();
    ASSERT_TRUE(dataFlowType >= 0);

    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Disconnected Ping..." << std::endl;
    pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult == PING_CHECK_FAIL);
    dataFlowType = CellularDataTest::GetCellularDataFlowTypeTest();
    ASSERT_TRUE(dataFlowType == 0);
}

/**
 * @tc.number   DataFlowType_Test_02
 * @tc.name     Test the GetCellularDataFlowType function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataFlowType_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(SIM_SLOT_ID_1);
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);

    CellularDataTest::EnableCellularDataTest(true);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Connected Ping..." << std::endl;
    int32_t pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult >= PING_CHECK_SUCCESS);
    int32_t dataFlowType = CellularDataTest::GetCellularDataFlowTypeTest();
    ASSERT_TRUE(dataFlowType >= 0);

    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Disconnected Ping..." << std::endl;
    pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult == PING_CHECK_FAIL);
    dataFlowType = CellularDataTest::GetCellularDataFlowTypeTest();
    ASSERT_TRUE(dataFlowType == 0);
}

/**
 * @tc.number   MmsApn_Test_01
 * @tc.name     Test the Mms apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, MmsApn_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_MMS);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(DEFAULT_SIM_SLOT_ID);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto mmsCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (mmsCallback == nullptr) {
        std::cout << "mmsCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (mmsCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(mmsCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   MmsApn_Test_02
 * @tc.name     Test the Mms apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, MmsApn_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_MMS);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(SIM_SLOT_ID_1);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto mmsCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (mmsCallback == nullptr) {
        std::cout << "mmsCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (mmsCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(mmsCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   HasInternetCapability_Test_01
 * @tc.name     Test the HasInternetCapability function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, HasInternetCapability_Test_01, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }

    int32_t cid = 1;
    int32_t result = CellularDataTest::HasInternetCapability(SIM_SLOT_ID_1, cid);
    ASSERT_TRUE(result == static_cast<int32_t>(RequestNetCode::REQUEST_FAILED));
}

/**
 * @tc.number   HasInternetCapability_Test_02
 * @tc.name     Test the HasInternetCapability function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, HasInternetCapability_Test_02, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }

    int32_t cid = 1;
    int32_t result = CellularDataTest::HasInternetCapability(DEFAULT_SIM_SLOT_ID, cid);
    ASSERT_TRUE(result == static_cast<int32_t>(RequestNetCode::REQUEST_FAILED));
}

/**
 * @tc.number   ClearCellularDataConnections_Test_01
 * @tc.name     Test the ClearCellularDataConnections function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ClearCellularDataConnections_Test_01, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::ClearCellularDataConnections(SIM_SLOT_ID_1);
    ASSERT_TRUE(result == static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS));
}

/**
 * @tc.number   ClearCellularDataConnections_Test_02
 * @tc.name     Test the ClearCellularDataConnections function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ClearCellularDataConnections_Test_02, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::ClearCellularDataConnections(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS));
}

/**
 * @tc.number   ClearAllConnections
 * @tc.name     Test the ClearAllConnections function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ClearAllConnections_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::ClearAllConnections(
        DEFAULT_SIM_SLOT_ID, DisConnectionReason::REASON_RETRY_CONNECTION);
    ASSERT_TRUE(result == static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS));
}

/**
 * @tc.number   GetApnState
 * @tc.name     Test the GetApnState function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetApnState_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::GetApnState(DEFAULT_SIM_SLOT_ID, "default");
    ASSERT_TRUE(result >= 0 && result <= 5);
}

/**
 * @tc.number   GetDataRecoveryState
 * @tc.name     Test the GetDataRecoveryState function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetDataRecoveryState_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::GetDataRecoveryState();
    ASSERT_TRUE(result >= 0 && result <= 3);
}

/**
 * @tc.number   CellularDataDump_Test_01
 * @tc.name    TestDump
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, CellularDataDump_Test_01, Function | MediumTest | Level3)
{
    std::vector<std::u16string> emptyArgs = {};
    std::vector<std::u16string> args = { u"test", u"test1" };
    EXPECT_GE(DelayedSingleton<CellularDataService>::GetInstance()->Dump(-1, args), 0);
    EXPECT_GE(DelayedSingleton<CellularDataService>::GetInstance()->Dump(0, emptyArgs), 0);
    EXPECT_GE(DelayedSingleton<CellularDataService>::GetInstance()->Dump(0, args), 0);
}

/**
 * @tc.number   Telephony_Cellulardata_InitTelephonyExtService_0100
 * @tc.name     Init Telephony Ext Service.
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, Telephony_Cellulardata_InitTelephonyExtService_0100, Function | MediumTest | Level1)
{
    DataAccessToken token;
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapper();
    if (TELEPHONY_EXT_WRAPPER.telephonyExtWrapperHandle_ == nullptr) {
        TELEPHONY_LOGI("telephonyExtWrapperHandle_ null");
    } else {
        TELEPHONY_LOGI("telephonyExtWrapperHandle_ not null");
        EXPECT_EQ(TELEPHONY_EXT_WRAPPER.dataEndSelfCure_ != nullptr, true);
    }
}

/**
 * @tc.number   Telephony_Cellulardata_InitTelephonyExtService_0101
 * @tc.name     Init Telephony Ext Service.
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, Telephony_Cellulardata_InitTelephonyExtService_0101, Function | MediumTest | Level1)
{
    mockDlsym = new NiceMock<MockDlsym>();
    EXPECT_CALL(*mockDlsym, dlopen(_, _))
        .WillRepeatedly(Return(nullptr));
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapperForDynamicLoad();
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.telephonyDynamicLoadWrapperHandle_, nullptr);
    delete mockDlsym;
    mockDlsym = nullptr;
}

/**
 * @tc.number   Telephony_Cellulardata_InitTelephonyExtService_0102
 * @tc.name     Init Telephony Ext Service.
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, Telephony_Cellulardata_InitTelephonyExtService_0102, Function | MediumTest | Level1)
{
    mockDlsym = new NiceMock<MockDlsym>();
    EXPECT_CALL(*mockDlsym, dlopen(_, _))
        .WillRepeatedly(Return(reinterpret_cast<void *>(0x1234)));
    EXPECT_CALL(*mockDlsym, dlsym(_, _))
        .WillRepeatedly(Return(nullptr));
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapperForDynamicLoad();
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.telephonyDynamicLoadWrapperHandle_ != nullptr, true);
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.dynamicLoadInit_, nullptr);
    delete mockDlsym;
    mockDlsym = nullptr;
}

/**
 * @tc.number   Telephony_Cellulardata_InitTelephonyExtService_0103
 * @tc.name     Init Telephony Ext Service.
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, Telephony_Cellulardata_InitTelephonyExtService_0103, Function | MediumTest | Level1)
{
    mockDlsym = new NiceMock<MockDlsym>();
    EXPECT_CALL(*mockDlsym, dlopen(_, _))
        .WillRepeatedly(Return(reinterpret_cast<void *>(0x1234)));
    EXPECT_CALL(*mockDlsym, dlsym(_, _))
        .WillOnce(Return(reinterpret_cast<void *>(0x12345)))
        .WillOnce(Return(nullptr));
    auto controller = std::make_shared<CellularDataController>(DEFAULT_SIM_SLOT_ID);
    controller->Init();
    ASSERT_TRUE(controller->cellularDataHandler_ != nullptr);
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapperForDynamicLoad();
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.telephonyDynamicLoadWrapperHandle_ != nullptr, true);
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.dynamicLoadInit_ != nullptr, true);
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    controller->cellularDataHandler_->NotifyReqCellularData(false);
#endif
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.dynamicLoadNotifyReqCellularDataStatus_, nullptr);
    delete mockDlsym;
    mockDlsym = nullptr;
}

/**
 * @tc.number   Telephony_Cellulardata_InitTelephonyExtService_0104
 * @tc.name     Init Telephony Ext Service.
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, Telephony_Cellulardata_InitTelephonyExtService_0104, Function | MediumTest | Level1)
{
    mockDlsym = new NiceMock<MockDlsym>();
    EXPECT_CALL(*mockDlsym, dlopen(_, _))
        .WillRepeatedly(Return(reinterpret_cast<void *>(0x1234)));
    EXPECT_CALL(*mockDlsym, dlsym(_, _))
        .WillRepeatedly(Return(reinterpret_cast<void *>(0x12345)));
    auto controller = std::make_shared<CellularDataController>(DEFAULT_SIM_SLOT_ID);
    controller->Init();
    ASSERT_TRUE(controller->cellularDataHandler_ != nullptr);
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapperForDynamicLoad();
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.telephonyDynamicLoadWrapperHandle_ != nullptr, true);
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.dynamicLoadInit_ != nullptr, true);
#ifdef OHOS_BUILD_ENABLE_TELEPHONY_EXT
    controller->cellularDataHandler_->NotifyReqCellularData(true);
#endif
    EXPECT_EQ(TELEPHONY_EXT_WRAPPER.dynamicLoadNotifyReqCellularDataStatus_ != nullptr, true);
}

/**
 * @tc.number   GetDataConnApnAttr_Test_01
 * @tc.name     Test the GetDataConnApnAttr function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetDataConnApnAttr_Test_01, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    ApnItem::Attribute apnAttr;
    int32_t result = CellularDataTest::GetDataConnApnAttr(SIM_SLOT_ID_1, apnAttr);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   GetDataConnApnAttr_Test_02
 * @tc.name     Test the GetDataConnApnAttr function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetDataConnApnAttr_Test_02, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    ApnItem::Attribute apnAttr;
    int32_t result = CellularDataTest::GetDataConnApnAttr(DEFAULT_SIM_SLOT_ID, apnAttr);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   GetDataConnIpType_Test_01
 * @tc.name     Test the GetDataConnIpType function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetDataConnIpType_Test_01, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    std::string ipType;
    int32_t result = CellularDataTest::GetDataConnIpType(SIM_SLOT_ID_1, ipType);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   GetDataConnIpType_Test_02
 * @tc.name     Test the GetDataConnIpType function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetDataConnIpType_Test_02, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    std::string ipType;
    int32_t result = CellularDataTest::GetDataConnIpType(DEFAULT_SIM_SLOT_ID, ipType);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   IsNeedDoRecovery_Test_01
 * @tc.name     Test the IsNeedDoRecovery function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, IsNeedDoRecovery_Test_01, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    bool needDoRecovery = true;
    int32_t result = CellularDataTest::IsNeedDoRecovery(SIM_SLOT_ID_1, needDoRecovery);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   IsNeedDoRecovery_Test_02
 * @tc.name     Test the IsNeedDoRecovery function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, IsNeedDoRecovery_Test_02, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    bool needDoRecovery = true;
    int32_t result = CellularDataTest::IsNeedDoRecovery(DEFAULT_SIM_SLOT_ID, needDoRecovery);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   GetCellularDataSupplierId_Test_01
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetCellularDataSupplierId_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    uint32_t supplierId = 0;
    uint64_t capabilityInvalid = NetCap::NET_CAPABILITY_END;
    int32_t result = CellularDataTest::GetCellularDataSupplierId(DEFAULT_SIM_SLOT_ID, capabilityInvalid, supplierId);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);

    result =
        CellularDataTest::GetCellularDataSupplierId(DEFAULT_SIM_SLOT_ID, NetCap::NET_CAPABILITY_INTERNET, supplierId);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   CorrectNetSupplierNoAvailable_Test_01
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, CorrectNetSupplierNoAvailable_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::CorrectNetSupplierNoAvailable(DEFAULT_SIM_SLOT_ID);
    int32_t apnState = CellularDataTest::GetApnState(DEFAULT_SIM_SLOT_ID, "default");
    if (apnState == ApnProfileState::PROFILE_STATE_CONNECTED) {
        ASSERT_TRUE(result == TELEPHONY_ERR_FAIL);
    } else {
        ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    }
}

/**
 * @tc.number   GetSupplierRegisterState_Test_01
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetSupplierRegisterState_Test_01, TestSize.Level3)
{
    DataAccessToken token;
    int32_t regState = -1;
    uint32_t supplierId = 1;
    int32_t result = CellularDataTest::GetSupplierRegisterState(supplierId, regState);
    ASSERT_TRUE(result == TELEPHONY_ERR_FAIL);

    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    int32_t getSupplierIdRet =
        CellularDataTest::GetCellularDataSupplierId(DEFAULT_SIM_SLOT_ID, NetCap::NET_CAPABILITY_INTERNET, supplierId);
    if (getSupplierIdRet == TELEPHONY_ERR_SUCCESS) {
        result = CellularDataTest::GetSupplierRegisterState(supplierId, regState);
        ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    }
}

/**
 * @tc.number   EnableIntelligenceSwitch_Test_01
 * @tc.name     Test Intelligence switch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, EnableIntelligenceSwitch_Test_01, TestSize.Level2)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    int32_t result1 = CellularDataTest::EnableIntelligenceSwitchTest(true);
    ASSERT_TRUE(result1 == TELEPHONY_ERR_SUCCESS);
    sleep(SLEEP_TIME);
    std::cout << "EnableIntelligenceSwitch ..." << std::endl;
    int32_t result2 = CellularDataTest::EnableIntelligenceSwitchTest(false);
    ASSERT_TRUE(result2 == TELEPHONY_ERR_SUCCESS);
    sleep(SLEEP_TIME);
    std::cout << "DisableIntelligenceSwitch ..." << std::endl;
}

/**
 * @tc.number   GetIntelligenceSwitchState_Test_01
 * @tc.name     Test Intelligence switch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetIntelligenceSwitchState_Test_01, TestSize.Level2)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    int32_t result1 = CellularDataTest::EnableIntelligenceSwitchTest(true);
    ASSERT_TRUE(result1 == TELEPHONY_ERR_SUCCESS);
    bool res1 = false;
    CellularDataTest::GetIntelligenceSwitchStateTest(res1);
    ASSERT_TRUE(res1 == true);
    std::cout << "Test GetIntelligenceSwitchState Of True..." << std::endl;
    int32_t result2 = CellularDataTest::EnableIntelligenceSwitchTest(false);
    ASSERT_TRUE(result2 == TELEPHONY_ERR_SUCCESS);
    bool res2 = true;
    CellularDataTest::GetIntelligenceSwitchStateTest(res2);
    ASSERT_TRUE(res2 == false);
    std::cout << "Test GetIntelligenceSwitchState Of False..." << std::endl;
}

/**
 * @tc.number   InitCellularDataController_Test_01
 * @tc.name     Test the InitCellularDataController function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, InitCellularDataController_Test_01, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::InitCellularDataController(SIM_SLOT_ID_1);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   InitCellularDataController_Test_02
 * @tc.name     Test the InitCellularDataController function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, InitCellularDataController_Test_02, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    int32_t result = CellularDataTest::InitCellularDataController(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   InitCellularDataController_Test_03
 * @tc.name     Test the InitCellularDataController function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, InitCellularDataController_Test_03, TestSize.Level3)
{
    DataAccessToken token;
    int32_t result = CellularDataTest::InitCellularDataController(CELLULAR_DATA_VSIM_SLOT_ID);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number   SUPL_Apn_Test_01
 * @tc.name     Test the SUPL apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, SUPL_Apn_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_SUPL);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(DEFAULT_SIM_SLOT_ID);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto suplCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (suplCallback == nullptr) {
        std::cout << "suplCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (suplCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(suplCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   SUPL_Apn_Test_02
 * @tc.name     Test the SUPL apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, SUPL_Apn_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_SUPL);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(SIM_SLOT_ID_1);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto suplCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (suplCallback == nullptr) {
        std::cout << "suplCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (suplCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(suplCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   DUN_Apn_Test_01
 * @tc.name     Test the DUN apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DUN_Apn_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_DUN);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(DEFAULT_SIM_SLOT_ID);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto dunCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (dunCallback == nullptr) {
        std::cout << "dunCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (dunCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(dunCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   DUN_Apn_Test_02
 * @tc.name     Test the DUN apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DUN_Apn_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_DUN);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(SIM_SLOT_ID_1);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto dunCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (dunCallback == nullptr) {
        std::cout << "dunCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (dunCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(dunCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   IA_Apn_Test_01
 * @tc.name     Test the IA apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, IA_Apn_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_IA);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(DEFAULT_SIM_SLOT_ID);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto iaCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (iaCallback == nullptr) {
        std::cout << "iaCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (iaCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(iaCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   IA_Apn_Test_02
 * @tc.name     Test the IA apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, IA_Apn_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_IA);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(SIM_SLOT_ID_1);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto iaCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (iaCallback == nullptr) {
        std::cout << "iaCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (iaCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(iaCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   XCAP_Apn_Test_01
 * @tc.name     Test the XCAP apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, XCAP_Apn_Test_01, TestSize.Level3)
{
    if (!HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_XCAP);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(DEFAULT_SIM_SLOT_ID);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto xcapCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (xcapCallback == nullptr) {
        std::cout << "xcapCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (xcapCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(xcapCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   XCAP_Apn_Test_02
 * @tc.name     Test the XCAP apn function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, XCAP_Apn_Test_02, TestSize.Level3)
{
    if (!HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    DataAccessToken token;
    sptr<INetConnCallback> callback = new (std::nothrow) TestCallback();
    if (callback == nullptr) {
        std::cout << "callback is null" << std::endl;
        return;
    }
    NetSpecifier netSpecifier;
    NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetCap::NET_CAPABILITY_XCAP);
    netAllCapabilities.bearerTypes_.insert(NetBearType::BEARER_CELLULAR);
    int32_t simId = CoreServiceClient::GetInstance().GetSimId(SIM_SLOT_ID_1);
    netSpecifier.ident_ = "simId" + std::to_string(simId);
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier(netSpecifier);
    if (specifier == nullptr) {
        std::cout << "specifier is null" << std::endl;
        return;
    }
    int32_t result = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, callback, NET_REGISTER_TIMEOUT_MS);
    std::cout << "RegisterNetConnCallback result [" << result << "]" << std::endl;
    auto xcapCallback = static_cast<TestCallback *>(callback.GetRefPtr());
    if (xcapCallback == nullptr) {
        std::cout << "xcapCallback is null" << std::endl;
        return;
    }
    int32_t count = 0;
    while (count < MAX_TIMES) {
        sleep(SLEEP_TIME);
        if (xcapCallback->isCallback_ == true) {
            break;
        }
        count++;
    }
    ASSERT_TRUE(xcapCallback->isCallback_);
    result = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    std::cout << "UnregisterNetConnCallback result [" << result << "]" << std::endl;
}

/**
 * @tc.number   RequestNetwork_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, RequestNetwork_001, TestSize.Level3)
{
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    NetManagerStandard::NetRequest netrequest;
    int32_t result = netAgent.callBack_->RequestNetwork(ident, netCaps, netrequest);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   RequestNetwork_002
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, RequestNetwork_002, TestSize.Level3)
{
    std::string ident = "testIdent";
    std::set<NetCap> netCaps = { NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET };
    NetManagerStandard::NetRequest netrequest;
    int32_t result = netAgent.callBack_->RequestNetwork(ident, netCaps, netrequest);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   ReleaseNetwork_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ReleaseNetwork_001, TestSize.Level3)
{
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    int32_t result = netAgent.callBack_->ReleaseNetwork(ident, netCaps);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   ReleaseNetwork_002
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ReleaseNetwork_002, TestSize.Level3)
{
    std::string ident = "testIdent";
    std::set<NetCap> netCaps = { NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET };
    int32_t result = netAgent.callBack_->ReleaseNetwork(ident, netCaps);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   NetStrategySwitch_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, NetStrategySwitch_001, TestSize.Level3)
{
    int32_t result = netAgent.tacticsCallBack_->NetStrategySwitch("", true);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   NetStrategySwitch_002
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, NetStrategySwitch_002, TestSize.Level3)
{
    int32_t result = netAgent.tacticsCallBack_->NetStrategySwitch("abc", true);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   NetStrategySwitch_003
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, NetStrategySwitch_003, TestSize.Level3)
{
    int32_t result = netAgent.tacticsCallBack_->NetStrategySwitch("123", true);
    ASSERT_EQ(result, CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   GetSupplierRegState_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetSupplierRegState_001, TestSize.Level3)
{
    int32_t regState = 0;
    int32_t result = netAgent.GetSupplierRegState(0, regState);
    EXPECT_FALSE(result);

    NetSupplier netSupplier = { 0 };
    netSupplier.supplierId = 1000;
    netSupplier.slotId = 0;
    netAgent.AddNetSupplier(netSupplier);
    result = netAgent.GetSupplierRegState(1000, regState);
    EXPECT_TRUE(result);

    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo();
    result = netAgent.UpdateNetSupplierInfo(1000, netSupplierInfo);
    EXPECT_NE(result, 0);
}

/**
 * @tc.number   RdbUpdate_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, RdbUpdate_001, TestSize.Level3)
{
    DataShare::DataShareValuesBucket values;
    DataShare::DataSharePredicates predicates;
    CellularDataRdbHelper cellularDataRdbHelper;
    int result = cellularDataRdbHelper.Update(values, predicates);
    ASSERT_EQ(result, NULL_POINTER_EXCEPTION);
}

/**
 * @tc.number   RdbInsert_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, RdbInsert_001, TestSize.Level3)
{
    DataShare::DataShareValuesBucket values;
    CellularDataRdbHelper cellularDataRdbHelper;
    int result = cellularDataRdbHelper.Insert(values);
    ASSERT_EQ(result, NULL_POINTER_EXCEPTION);
}

/**
 * @tc.number   QueryApns_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, QueryApns_001, TestSize.Level3)
{
    std::string mcc = "123";
    std::string mnc = "456";
    std::vector<PdpProfile> apnVec;
    int32_t slotId = 0;
    CellularDataRdbHelper cellularDataRdbHelper;
    bool result = cellularDataRdbHelper.QueryApns(mcc, mnc, apnVec, slotId);
    ASSERT_FALSE(result);
}


/**
 * @tc.number   QueryMvnoApnsByType_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, QueryMvnoApnsByType_001, TestSize.Level3)
{
    std::string mcc = "123";
    std::string mnc = "456";
    std::string mvnoType = "789";
    std::string mvnoDataFromSim = "";
    std::vector<PdpProfile> mvnoApnVec;
    int32_t slotId = 0;
    CellularDataRdbHelper cellularDataRdbHelper;
    bool result = cellularDataRdbHelper.QueryMvnoApnsByType(mcc, mnc, mvnoType, mvnoDataFromSim, mvnoApnVec, slotId);
    ASSERT_TRUE(result);
}

/**
 * @tc.number   QueryMvnoApnsByType_002
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, QueryMvnoApnsByType_002, TestSize.Level3)
{
    std::string mcc = "123";
    std::string mnc = "456";
    std::string mvnoType = "789";
    std::string mvnoDataFromSim = "012";
    std::vector<PdpProfile> mvnoApnVec;
    int32_t slotId = 0;
    CellularDataRdbHelper cellularDataRdbHelper;
    bool result = cellularDataRdbHelper.QueryMvnoApnsByType(mcc, mnc, mvnoType, mvnoDataFromSim, mvnoApnVec, slotId);
    ASSERT_FALSE(result);
}

/**
 * @tc.number   ReadApnResult_001
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ReadApnResult_001, TestSize.Level3)
{
    std::shared_ptr<DataShare::DataShareResultSet> result = nullptr;
    std::vector<PdpProfile> apnVec;
    CellularDataRdbHelper cellularDataRdbHelper;
    cellularDataRdbHelper.ReadApnResult(nullptr, apnVec);
    ASSERT_TRUE(apnVec.empty());
}

/**
 * @tc.number   ReadApnResult_002
 * @tc.name     Test the function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ReadApnResult_002, TestSize.Level3)
{
    std::shared_ptr<DataShare::DataShareResultSet> result = std::make_shared<DataShare::DataShareResultSet>();
    std::vector<PdpProfile> apnVec;
    CellularDataRdbHelper cellularDataRdbHelper;
    cellularDataRdbHelper.ReadApnResult(nullptr, apnVec);
    ASSERT_TRUE(apnVec.empty());
}

#else  // TEL_TEST_UNSUPPORT
/**
 * @tc.number   DataMock_Test_01
 * @tc.name     Test for unsupport platform
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataMock_Test_01, TestSize.Level3)
{
    EXPECT_TRUE(true);
}
#endif // TEL_TEST_UNSUPPORT
/**
 * @tc.number   CellularDataControllerAddUid_Test_01
 * @tc.name     Test the CellularDataControllerAddUid function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, CellularDataControllerAddUid_Test_01, TestSize.Level3)
{
    auto controller = std::make_shared<CellularDataController>(DEFAULT_SIM_SLOT_ID);
    controller->cellularDataHandler_ = nullptr;
    NetRequest request;
    request.uid = 0;
    ASSERT_FALSE(controller->AddUid(request));
}

/**
 * @tc.number   CellularDataControllerRemoveUid_Test_01
 * @tc.name     Test the CellularDataControllerRemoveUid function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, CellularDataControllerRemoveUid_Test_1, TestSize.Level3)
{
    auto controller = std::make_shared<CellularDataController>(DEFAULT_SIM_SLOT_ID);
    controller->cellularDataHandler_ = nullptr;
    NetRequest request;
    request.uid = 0;
    ASSERT_FALSE(controller->RemoveUid(request));
}

/**
 * @tc.number   ControllerReleaseCellularDataConnection_Test_01
 * @tc.name     Test the Controlle ReleaseCellularDataConnection function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ControllerReleaseCellularDataConnection_Test_1, TestSize.Level3)
{
    auto controller = std::make_shared<CellularDataController>(DEFAULT_SIM_SLOT_ID);
    controller->cellularDataHandler_ = nullptr;
    ASSERT_FALSE(controller->ReleaseCellularDataConnection());
}

/**
 * @tc.number  ControllerUpdateNetworkInfo_Test_01
 * @tc.name     Test the Controller UpdateNetworkInfo function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ControllerUpdateNetworkInfo_Test_01, TestSize.Level3)
{
    auto controller = std::make_shared<CellularDataController>(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(controller->cellularDataHandler_ == nullptr);
    ASSERT_FALSE(controller->UpdateNetworkInfo());
}

/**
 * @tc.number  ControllerUpdateNetworkInfo_Test_02
 * @tc.name     Test the Controller UpdateNetworkInfo function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, ControllerUpdateNetworkInfo_Test_02, TestSize.Level3)
{
    auto controller = std::make_shared<CellularDataController>(DEFAULT_SIM_SLOT_ID);
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto cellularDataHandler = std::make_shared<CellularDataHandler>(subscriberInfo, 0);
    cellularDataHandler->Init();
    controller->cellularDataHandler_ = cellularDataHandler;
    ASSERT_TRUE(controller->cellularDataHandler_ != nullptr);
    ASSERT_TRUE(controller->UpdateNetworkInfo());
}

} // namespace Telephony
} // namespace OHOS
