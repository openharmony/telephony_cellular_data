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
#include "accesstoken_kit.h"
#include "cellular_data_client.h"
#include "cellular_data_error.h"
#include "cellular_data_service.h"
#include "cellular_data_types.h"
#include "core_service_client.h"
#include "cstdio"
#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "gtest/gtest.h"
#include "gtest/hwext/gtest-tag.h"
#include "i_cellular_data_manager.h"
#include "if_system_ability_manager.h"
#include "iosfwd"
#include "iostream"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "ostream"
#include "refbase.h"
#include "system_ability_definition.h"
#include "telephony_errors.h"
#include "telephony_types.h"
#include "token_setproc.h"
#include "unistd.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

static const int32_t SLEEP_TIME = 1;
static const int32_t SIM_SLOT_ID_1 = DEFAULT_SIM_SLOT_ID + 1;
static const int32_t DATA_SLOT_ID_INVALID = DEFAULT_SIM_SLOT_ID + 10;
static const int32_t PING_CHECK_SUCCESS = 0;
static const int32_t PING_CHECK_FAIL = 1;
static const int32_t MAX_TIMES = 35;
static const int32_t CMD_BUF_SIZE = 10240;

HapInfoParams testInfoParams = {
    .bundleName = "tel_cellular_data_test",
    .userID = 1,
    .instIndex = 0,
    .appIDDesc = "test",
};

PermissionDef testPermGetNetworkInfoDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "tel_cellular_data_test",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test cellular data",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testGetNetworkInfoState = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .resDeviceID = { "local" },
};

PermissionDef testPermSetTelephonyStateDef = {
    .permissionName = "ohos.permission.SET_TELEPHONY_STATE",
    .bundleName = "tel_cellular_data_test",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test cellular data",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testSetTelephonyState = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.SET_TELEPHONY_STATE",
    .resDeviceID = { "local" },
};

HapPolicyParams testPolicyParams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = { testPermGetNetworkInfoDef, testPermSetTelephonyStateDef },
    .permStateList = { testGetNetworkInfoState, testSetTelephonyState },
};

class AccessToken {
public:
    AccessToken()
    {
        currentID_ = GetSelfTokenID();
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParams, testPolicyParams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};

class CellularDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();
    static bool HasSimCard(const int32_t slotId);
    static int32_t IsCellularDataEnabledTest(bool &dataEnabled);
    static int32_t EnableCellularDataTest(bool enable);
    static int32_t GetCellularDataStateTest();
    static int32_t IsCellularDataRoamingEnabledTest(int32_t slotId, bool &dataRoamingEnabled);
    static int32_t EnableCellularDataRoamingTest(int32_t slotId, bool enable);
    static int32_t GetDefaultCellularDataSlotIdTest();
    static int32_t SetDefaultCellularDataSlotIdTest(int32_t slotId);
    static int32_t GetCellularDataFlowTypeTest();
    static void WaitTestTimeout(const int32_t status);
    static sptr<ICellularDataManager> GetProxy();
    static string GetCmdResult();
    static int32_t PingTest();
    static int32_t HasInternetCapability(int32_t slotId, int32_t cid);
    static int32_t ClearCellularDataConnections(int32_t slotId);
};

bool CellularDataTest::HasSimCard(const int32_t slotId)
{
    bool hasSimCard = false;
    DelayedRefSingleton<CoreServiceClient>::GetInstance().HasSimCard(slotId, hasSimCard);
    return hasSimCard;
}

void CellularDataTest::TearDownTestCase() {}

void CellularDataTest::SetUp() {}

void CellularDataTest::TearDown() {}

void CellularDataTest::SetUpTestCase()
{
    if (CoreServiceClient::GetInstance().GetProxy() == nullptr) {
        std::cout << "connect coreService server failed!" << std::endl;
        return;
    }

    AccessToken token;
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
    if (strRe.find("3 received") != string::npos) {
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

int32_t CellularDataTest::GetCellularDataStateTest()
{
    return CellularDataClient::GetInstance().GetCellularDataState();
}

int32_t CellularDataTest::EnableCellularDataRoamingTest(int32_t slotId, bool enable)
{
    return CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, enable);
}

int32_t CellularDataTest::GetDefaultCellularDataSlotIdTest()
{
    return CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
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
    return CellularDataClient::GetInstance().ClearCellularDataConnections(slotId);
}
#ifndef TEL_TEST_UNSUPPORT
/**
 * @tc.number   IsCellularDataEnabled_Test
 * @tc.name     Test cellular data switch status(enabled or disabled)
 * @tc.desc     Function test
 */
HWTEST_F(CellularDataTest, IsCellularDataEnabled_Test, TestSize.Level1)
{
    AccessToken token;
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
    AccessToken token;
    int32_t result = CellularDataTest::GetDefaultCellularDataSlotIdTest();
    if (result < DEFAULT_SIM_SLOT_ID_REMOVE) {
        return;
    }
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    // Multiple cards will need to be optimized again
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DEFAULT_SIM_SLOT_ID - 1);
    ASSERT_TRUE(result == TELEPHONY_ERR_SUCCESS);
    result = CellularDataTest::SetDefaultCellularDataSlotIdTest(DATA_SLOT_ID_INVALID);
    ASSERT_TRUE(result != TELEPHONY_ERR_SUCCESS);
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
    AccessToken token;
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
    AccessToken token;
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
    ASSERT_TRUE(pingResult == PING_CHECK_SUCCESS);
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
    AccessToken token;
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
    ASSERT_TRUE(pingResult == PING_CHECK_SUCCESS);
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
    AccessToken token;
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
    AccessToken token;
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
    AccessToken token;
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
    AccessToken token;
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
    AccessToken token;
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
    AccessToken token;
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
    AccessToken token;
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
    AccessToken token;
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
    AccessToken token;
    CellularDataTest::SetDefaultCellularDataSlotIdTest(SIM_SLOT_ID_1);
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
    AccessToken token;
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
    AccessToken token;
    int32_t result = CellularDataTest::ClearCellularDataConnections(DEFAULT_SIM_SLOT_ID);
    ASSERT_TRUE(result == static_cast<int32_t>(RequestNetCode::REQUEST_SUCCESS));
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
    EXPECT_EQ(DelayedSingleton<CellularDataService>::GetInstance()->Dump(-1, args), TELEPHONY_ERR_FAIL);
    EXPECT_EQ(DelayedSingleton<CellularDataService>::GetInstance()->Dump(0, emptyArgs), 0);
    EXPECT_EQ(DelayedSingleton<CellularDataService>::GetInstance()->Dump(0, args), 0);
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
} // namespace Telephony
} // namespace OHOS
