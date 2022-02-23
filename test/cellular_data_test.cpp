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

#include <string>

#include "gtest/gtest.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "core_service_client.h"
#include "telephony_types.h"

#include "cellular_data_error.h"
#include "cellular_data_types.h"
#include "i_cellular_data_manager.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class CellularDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();
    static int32_t IsCellularDataEnabledTest();
    static int32_t EnableCellularDataTest(bool enable);
    static int32_t GetCellularDataStateTest();
    static int32_t IsCellularDataRoamingEnabledTest(int32_t slotId);
    static int32_t EnableCellularDataRoamingTest(int32_t slotId, bool enable);
    static int32_t GetDefaultCellularDataSlotIdTest();
    static int32_t SetDefaultCellularDataSlotIdTest(int32_t slotId);
    static int32_t GetCellularDataFlowTypeTest();
    static void WaitTestTimeout(const int32_t status);
    static sptr<ICellularDataManager> GetProxy();
    static string GetCmdResult(const string &strCmd);
    static int32_t PingTest();

public:
    static sptr<ICellularDataManager> proxy_;
    static const int32_t SLEEP_TIME = 1;
    static const int32_t DATA_SLOT_ID_INVALID = DEFAULT_SIM_SLOT_ID + 10;
    static const int32_t PING_CHECK_SUCCESS = 0;
    static const int32_t PING_CHECK_FAIL = 1;
};

sptr<ICellularDataManager> CellularDataTest::proxy_;

void CellularDataTest::TearDownTestCase()
{}

void CellularDataTest::SetUp()
{}

void CellularDataTest::TearDown()
{}

void CellularDataTest::SetUpTestCase()
{
    if (CoreServiceClient::GetInstance().GetProxy() == nullptr) {
        std::cout << "connect coreService server failed!" << std::endl;
        return;
    }

    proxy_ = GetProxy();
    ASSERT_TRUE(proxy_ != nullptr);
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    // Set the default slot
    int32_t result = proxy_->SetDefaultCellularDataSlotId(DEFAULT_SIM_SLOT_ID);
    if (result != static_cast<int32_t>(DataRespondCode::SET_SUCCESS)) {
        return;
    }
    int32_t enable = proxy_->EnableCellularData(true);
    ASSERT_TRUE(enable == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
}

void CellularDataTest::WaitTestTimeout(const int32_t status)
{
    if (proxy_ == nullptr) {
        return;
    }
    const int32_t maxTimes = 35;
    int32_t count = 0;
    while (count < maxTimes) {
        sleep(SLEEP_TIME);
        if (proxy_->GetCellularDataState() == status) {
            return;
        }
        count++;
    }
}

string CellularDataTest::GetCmdResult(const string &strCmd)
{
    char buf[10240] = {0};
    FILE *pf;
    char *fgetsRet;

    if ((pf = popen(strCmd.c_str(), "r")) == nullptr) {
        return "";
    }
    string strResult;
    while ((fgetsRet = fgets(buf, sizeof buf, pf)) != nullptr) {
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
    string strCmd = "ping -c3 www.openharmony.cn";
    string strRe = GetCmdResult(strCmd);
    std::cout << strRe << std::endl;

    // if ping succeed, the result should contains something like:
    // 3 packets transmitted, 3 received, 0% packet loss, time 5440ms
    if (strRe.find("3 received") != string::npos) {
        return PING_CHECK_SUCCESS;
    } else {
        return PING_CHECK_FAIL;
    }
}

int32_t CellularDataTest::IsCellularDataRoamingEnabledTest(int32_t slotId)
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = proxy_->IsCellularDataRoamingEnabled(slotId);
    return result;
}

int32_t CellularDataTest::IsCellularDataEnabledTest()
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = proxy_->IsCellularDataEnabled();
    return result;
}

int32_t CellularDataTest::EnableCellularDataTest(bool enable)
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = proxy_->EnableCellularData(enable);
    return result;
}

int32_t CellularDataTest::GetCellularDataStateTest()
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t result = proxy_->GetCellularDataState();
    return result;
}

int32_t CellularDataTest::EnableCellularDataRoamingTest(int32_t slotId, bool enable)
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy_->EnableCellularDataRoaming(slotId, enable);
}

int32_t CellularDataTest::GetDefaultCellularDataSlotIdTest()
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy_->GetDefaultCellularDataSlotId();
}

int32_t CellularDataTest::SetDefaultCellularDataSlotIdTest(int32_t slotId)
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy_->SetDefaultCellularDataSlotId(slotId);
}

int32_t CellularDataTest::GetCellularDataFlowTypeTest()
{
    if (proxy_ == nullptr) {
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return proxy_->GetCellularDataFlowType();
}

sptr<ICellularDataManager> CellularDataTest::GetProxy()
{
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        return nullptr;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID);
    if (remote) {
        sptr<ICellularDataManager> dataManager = iface_cast<ICellularDataManager>(remote);
        return dataManager;
    }
    return nullptr;
}

/**
 * @tc.number   GetProxy_Test
 * @tc.name     Check whether the cellular data service(SystemAbility) is started
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetProxy_Test, TestSize.Level1)
{
    CellularDataTest::proxy_ = CellularDataTest::GetProxy();
    ASSERT_FALSE(CellularDataTest::proxy_ == nullptr);
}

/**
 * @tc.number   IsCellularDataEnabled_Test
 * @tc.name     Test cellular data switch status(enabled or disabled)
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, IsCellularDataEnabled_Test, TestSize.Level1)
{
    int32_t result = CellularDataTest::IsCellularDataEnabledTest();
    ASSERT_TRUE(result >= static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_DISABLED));
}

/**
 * @tc.number   DefaultCellularDataSlotId_Test
 * @tc.name     Test set default data card slot
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DefaultCellularDataSlotId_Test, TestSize.Level2)
{
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    int32_t result = CellularDataTest::GetDefaultCellularDataSlotIdTest();
    if (result < DEFAULT_SIM_SLOT_ID) {
        return;
    }
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    // Multiple cards will need to be optimized again
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID - 1);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   EnableCellularData_Test
 * @tc.name     Test cellular data switch
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, EnableCellularData_Test, TestSize.Level2)
{
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    int32_t enabled = CellularDataTest::IsCellularDataEnabledTest();
    if (enabled == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED)) {
        // It takes seconds after being enabled for the connection status
        // changed to DATA_STATE_CONNECTED, so we must wait it out before excecuting ping check
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
        sleep(SLEEP_TIME);
        std::cout << "Cellular Data Connected Ping..." << std::endl;
        int32_t pingResult = CellularDataTest::PingTest();
        ASSERT_TRUE(pingResult == PING_CHECK_SUCCESS);
        int32_t disabled = CellularDataTest::EnableCellularDataTest(false);
        ASSERT_TRUE(disabled == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
        sleep(SLEEP_TIME);
        std::cout << "Cellular Data Disconnected Ping..." << std::endl;
        pingResult = CellularDataTest::PingTest();
        ASSERT_TRUE(pingResult == PING_CHECK_FAIL);
    } else {
        int32_t result = CellularDataTest::EnableCellularDataTest(true);
        ASSERT_TRUE(result == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
        sleep(SLEEP_TIME);
        std::cout << "Cellular Data Connected Ping..." << std::endl;
        int32_t pingResult = CellularDataTest::PingTest();
        ASSERT_TRUE(pingResult == PING_CHECK_SUCCESS);
        CellularDataTest::EnableCellularDataTest(false);
        WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
        sleep(SLEEP_TIME);
        std::cout << "Cellular Data Disconnected Ping..." << std::endl;
        pingResult = CellularDataTest::PingTest();
        ASSERT_TRUE(pingResult == PING_CHECK_FAIL);
    }
}

/**
 * @tc.number   DataRoamingState_ValidSlot_Test_01
 * @tc.name     Test the cellular data roaming switch with a slot id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataRoamingState_ValidSlot_Test_01, TestSize.Level3)
{
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    int32_t disabled = CellularDataTest::EnableCellularDataTest(false);
    ASSERT_TRUE(disabled == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));

    // slot1 enable data roaming
    int32_t enabled = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, true);
    ASSERT_TRUE(enabled == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    int32_t result = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED));
    // slot1 close
    int32_t enable = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, false);
    ASSERT_TRUE(enable == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_DISABLED));

    // At present, multiple card problems, the subsequent need to continue to deal with
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
    ASSERT_TRUE(enable == CELLULAR_DATA_INVALID_PARAM);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
    // At present, multiple card problems, the subsequent need to continue to deal with
    ASSERT_TRUE(enable == CELLULAR_DATA_INVALID_PARAM);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   EnableCellularDataRoaming_ValidSlot_Test_01
 * @tc.name     Test the cellular data roaming switch with a slot id
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, EnableCellularDataRoaming_ValidSlot_Test_01, TestSize.Level3)
{
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    int32_t disabled = CellularDataTest::EnableCellularDataTest(false);
    ASSERT_TRUE(disabled == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));

    int32_t isDataRoaming = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID);
    if (isDataRoaming == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED)) {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, false);
        ASSERT_TRUE(result == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    } else {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID, true);
        ASSERT_TRUE(result == static_cast<int32_t>(DataRespondCode::SET_SUCCESS));
    }
    // At present, multiple card problems, the subsequent need to continue to deal with
    isDataRoaming = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID);
    if (isDataRoaming == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED)) {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
        ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    } else {
        int32_t result = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
        ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    }
}

/**
 * @tc.number   GetCellularDataState_ValidityTest_01
 * @tc.name     Test the GetCellularDataState function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, GetCellularDataState_ValidityTest_01, TestSize.Level3)
{
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    int32_t enabled = CellularDataTest::IsCellularDataEnabledTest();
    if (enabled == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED)) {
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
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    // invalid slot turn on data roaming
    int32_t enable = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID -1, true);
    ASSERT_TRUE(enable == CELLULAR_DATA_INVALID_PARAM);
    int32_t result = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID -1);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, true);
    ASSERT_TRUE(enable == CELLULAR_DATA_INVALID_PARAM);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    // invalid slot disable roaming
    enable = CellularDataTest::EnableCellularDataRoamingTest(DEFAULT_SIM_SLOT_ID -1, false);
    ASSERT_TRUE(enable == CELLULAR_DATA_INVALID_PARAM);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DEFAULT_SIM_SLOT_ID -1);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
    enable = CellularDataTest::EnableCellularDataRoamingTest(DATA_SLOT_ID_INVALID, false);
    ASSERT_TRUE(enable == CELLULAR_DATA_INVALID_PARAM);
    result = CellularDataTest::IsCellularDataRoamingEnabledTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result == CELLULAR_DATA_INVALID_PARAM);
}

/**
 * @tc.number   DataFlowType_Test_01
 * @tc.name     Test the GetCellularDataFlowType function
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, DataFlowType_Test_01, TestSize.Level3)
{
    if (!CoreServiceClient::GetInstance().HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    CellularDataTest::EnableCellularDataTest(false);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED));
    sleep(SLEEP_TIME);

    CellularDataTest::EnableCellularDataTest(true);
    WaitTestTimeout(static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED));
    sleep(SLEEP_TIME);
    std::cout << "Cellular Data Connected Ping..." << std::endl;
    int32_t pingResult = CellularDataTest::PingTest();
    ASSERT_TRUE(pingResult == PING_CHECK_SUCCESS);
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
} // namespace Telephony
} // namespace OHOS