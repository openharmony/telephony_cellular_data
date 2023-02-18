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

#include <iostream>
#include <map>
#include <string>
#include <unistd.h>

#include "ability_context.h"
#include "accesstoken_kit.h"
#include "cellular_data_client.h"
#include "telephony_types.h"
#include "token_setproc.h"

using namespace OHOS::Security::AccessToken;
using OHOS::Security::AccessToken::AccessTokenID;

HapInfoParams testInfoParams = {
    .bundleName = "tel_cellular_data_ui_test",
    .userID = 1,
    .instIndex = 0,
    .appIDDesc = "test",
};

PermissionDef testPermGetTelephonyStateDef = {
    .permissionName = "ohos.permission.GET_TELEPHONY_STATE",
    .bundleName = "tel_cellular_data_ui_test",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test cellular data",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testGetTelephonyState = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.GET_TELEPHONY_STATE",
    .resDeviceID = { "local" },
};

PermissionDef testPermSetTelephonyStateDef = {
    .permissionName = "ohos.permission.SET_TELEPHONY_STATE",
    .bundleName = "tel_cellular_data_ui_test",
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

PermissionDef testPermGetNetworkInfoDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "tel_core_service_gtest",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test cellular data",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testPermGetNetworkInfo = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .resDeviceID = { "local" },
};

HapPolicyParams testPolicyParams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = { testPermGetTelephonyStateDef, testPermSetTelephonyStateDef, testPermGetNetworkInfoDef },
    .permStateList = { testGetTelephonyState, testSetTelephonyState, testPermGetNetworkInfo },
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

namespace OHOS {
namespace Telephony {
class CellularDataCodeTest {
    using Fun = void (*)();
public:
    CellularDataCodeTest() = default;

    ~CellularDataCodeTest() = default;

    static void IsCellularDataEnabledTest()
    {
        AccessToken token;
        bool enabled = false;
        int32_t result = CellularDataClient::GetInstance().IsCellularDataEnabled(enabled);
        std::cout << "TelephonyTestService Remote IsCellularDataEnabled result [" << result << "]"
                  << " enabled [" << enabled << "]" << std::endl;
    }

    static void EnableCellularDataTest()
    {
        AccessToken token;
        const int32_t maxTestCount = 61;
        const uint32_t spaceTime = 1000 * 100 * 2.6;
        const int32_t useMaxType = 1;
        const int32_t evenNumber = 2;
        int32_t type = 0;
        std::cout << "please input whether enable: enable/1,disable/0 " << std::endl;
        std::cin >> type;
        int32_t result = 0;
        if (type > useMaxType) {
            for (int32_t i = 1; i < maxTestCount; i++) {
                bool dataEnable = i % evenNumber;
                if (dataEnable) {
                    std::cout << "enable====: " << dataEnable << std::endl;
                    CellularDataClient::GetInstance().EnableCellularData(dataEnable);
                    usleep(spaceTime);
                    result = CellularDataClient::GetInstance().GetCellularDataState();
                    std::cout << "Remote GetCellularDataState result [" << result << "]" << std::endl;
                } else {
                    std::cout << "enable false====: " << dataEnable << std::endl;
                    CellularDataClient::GetInstance().EnableCellularData(dataEnable);
                    usleep(spaceTime);
                    result = CellularDataClient::GetInstance().GetCellularDataState();
                    std::cout << "Remote GetCellularDataState result [" << result << "]" << std::endl;
                }
                std::cout << "i := " << i << std::endl;
            }
            return;
        }
        bool enable = (type > 0);
        result = CellularDataClient::GetInstance().EnableCellularData(enable);
        std::cout << "Remote EnableCellularData " << enable <<" result [" << result << "]" << std::endl;
    }

    static void GetCellularDataStateTest()
    {
        int32_t result = CellularDataClient::GetInstance().GetCellularDataState();
        std::cout << "Remote GetCellularDataState result [" << result << "]" << std::endl;
    }

    static void IsCellularDataRoamingEnabledTest()
    {
        AccessToken token;
        int32_t slotId = DEFAULT_SIM_SLOT_ID;
        std::cout << "please input parameter int slotId" << std::endl;
        std::cin >> slotId;
        bool isDataRoaming = false;
        int32_t result = CellularDataClient::GetInstance().IsCellularDataRoamingEnabled(slotId, isDataRoaming);
        std::cout << "Remote IsCellularDataRoamingEnabled result [" << result << "]"
                  << " isDataRoaming [" << isDataRoaming << "]" << std::endl;
    }

    static void EnableCellularDataRoamingTest()
    {
        AccessToken token;
        int32_t slotId = DEFAULT_SIM_SLOT_ID;
        int32_t type = 0;
        std::cout << "please input parameter int slotId " << std::endl;
        std::cin >> slotId;
        std::cout << "please input parameter roaming enable: enable/1,disable/0" << std::endl;
        std::cin >> type;
        bool enable = (type > 0);
        int32_t result = CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, enable);
        std::cout << "Remote GetOperatorName " << enable << " result [" << result << "]" << std::endl;
    }

    static void HandleApnChangedTest()
    {
        AccessToken token;
        int32_t slotId = DEFAULT_SIM_SLOT_ID;
        sptr<ICellularDataManager> proxy = CellularDataClient::GetInstance().GetProxy();
        if (proxy != nullptr) {
            int32_t result = proxy->HandleApnChanged(slotId);
            std::cout << "TelephonyTestService Remote result [" << result << "]" << std::endl;
        } else {
            std::cout << "TelephonyTestService Remote is null" << std::endl;
        }
    }

    static void GetDefaultSlotId()
    {
        sptr<ICellularDataManager> proxy = CellularDataClient::GetInstance().GetProxy();
        if (proxy != nullptr) {
            int32_t result = proxy->GetDefaultCellularDataSlotId();
            std::cout << "get default slot id is := " << result << std::endl;
        } else {
            std::cout << "get default slot id fail" << std::endl;
        }
    }

    static void SetDefaultSlotId()
    {
        AccessToken token;
        int32_t slotId = DEFAULT_SIM_SLOT_ID;
        std::cout << "please input parameter int slot" << std::endl;
        std::cin >> slotId;
        sptr<ICellularDataManager> proxy = CellularDataClient::GetInstance().GetProxy();
        if (proxy != nullptr) {
            int32_t result = proxy->SetDefaultCellularDataSlotId(slotId);
            std::cout << "TelephonyTestService SetDefaultSlotId is " << result << std::endl;
        } else {
            std::cout << "TelephonyTestService SetDefaultSlotId is null" << std::endl;
        }
    }

    static void GetDataFlowType()
    {
        sptr<ICellularDataManager> proxy = CellularDataClient::GetInstance().GetProxy();
        if (proxy != nullptr) {
            int32_t result = proxy->GetCellularDataFlowType();
            std::cout << "get CellDataFlowType is := " << result << std::endl;
        } else {
            std::cout << "get CellDataFlowType is fail" << std::endl;
        }
    }

    static void TestDataPackageUp()
    {
        system("rm /data/iface_tx_p");
        system("touch /data/iface_rx_p");
        if (access("/data/iface_tx_p", F_OK) == 0) {
            std::cout << "Please manually delete iface_tx_p file " << std::endl;
            return;
        }
        if (access("/data/iface_tx_p", F_OK) != 0) {
            std::cout << "Please manually touch iface_tx_p file" << std::endl;
        }
    }

    static void TestDataPackageDown()
    {
        system("rm /data/iface_rx_p");
        system("touch /data/iface_tx_p");
        if (access("/data/iface_rx_p", F_OK) == 0) {
            std::cout << "Please manually delete iface_rx_p file " << std::endl;
            return;
        }
        if (access("/data/iface_tx_p", F_OK) != 0) {
            std::cout << "Please manually touch iface_tx_p file" << std::endl;
        }
    }

    static void TestDataPackageUpDown()
    {
        system("rm /data/iface_tx_p");
        system("rm /data/iface_rx_p");
        if (access("/data/iface_tx_p", F_OK) == 0 || access("/data/iface_rx_p", F_OK) == 0) {
            std::cout << "Please manually delete iface_rx_p and iface_tx_p file " << std::endl;
            return;
        }
    }

    static bool DataPackageTest(const int32_t testId)
    {
        std::map<int32_t, Fun> testFunMap {
            {TEST_DATA_PACKAGE_UP, &CellularDataCodeTest::TestDataPackageUp},
            {TEST_DATA_PACKAGE_DOWN, &CellularDataCodeTest::TestDataPackageDown},
            {TEST_DATA_PACKAGE_UP_DOWN, &CellularDataCodeTest::TestDataPackageUpDown},

        };
        std::map<int32_t, Fun>::iterator it = testFunMap.find(testId);
        if (it != testFunMap.end()) {
            (*(it->second))();
        } else if (TEST_DATA_PACKAGE_EXIT == testId) {
            std::cout << "exit..." << std::endl;
            return false;
        } else {
            std::cout << "please input correct number..." << std::endl;
        }
        return true;
    }

    static void TouchOrRmDataPackage()
    {
        int32_t input = 0;
        while (true) {
            std::cout << "\n-----------start--------------\nusage:please input a cmd num:\n"
                "0:testDataPackageUp\n"
                "1:testDataPackageDown\n"
                "2:testDataPackageUpDown\n"
                "100:exit\n"
            << std::endl;
            std::cin >> input;
            std::cout << "inputCMD is [" << input << "]" << std::endl;
            if (!DataPackageTest(input)) {
                break;
            }
        }
    }

    bool UnitTest(const int32_t testId) const
    {
        std::map<int32_t, Fun> testFunMap {
            {IS_CELLULAR_DATA_ENABLED_TEST, &IsCellularDataEnabledTest},
            {ENABLE_CELLULAR_DATA_TEST, &EnableCellularDataTest},
            {GET_CELLULAR_DATA_STATE_TEST, &GetCellularDataStateTest},
            {IS_DATA_ROAMING_ENABLED_TEST, &IsCellularDataRoamingEnabledTest},
            {ENABLE_DATA_ROAMING_TEST, &EnableCellularDataRoamingTest},
            {APN_CHANGED_TEST, &HandleApnChangedTest},
            {GET_DEFAULT_SLOT_ID, &GetDefaultSlotId},
            {SET_DEFAULT_SLOT_ID, &SetDefaultSlotId},
            {GET_DATA_FLOW_TYPE, &GetDataFlowType},
            {TOUCH_OR_RM_DATA_PACKAGE, &TouchOrRmDataPackage},
        };
        std::map<int32_t, Fun>::iterator it = testFunMap.find(testId);
        if (it != testFunMap.end()) {
            (*(it->second))();
        } else if (EXIT_CELLULAR_DATA_TEST == testId) {
            std::cout << "exit..." << std::endl;
            return false;
        } else {
            std::cout << "please input correct number..." << std::endl;
        }
        return true;
    }

private:
    const int32_t IS_CELLULAR_DATA_ENABLED_TEST = 0;
    const int32_t ENABLE_CELLULAR_DATA_TEST = 1;
    const int32_t GET_CELLULAR_DATA_STATE_TEST = 2;
    const int32_t IS_DATA_ROAMING_ENABLED_TEST = 3;
    const int32_t ENABLE_DATA_ROAMING_TEST = 4;
    const int32_t APN_CHANGED_TEST = 5;
    const int32_t GET_DEFAULT_SLOT_ID = 6;
    const int32_t SET_DEFAULT_SLOT_ID = 7;
    const int32_t GET_DATA_FLOW_TYPE = 8;
    const int32_t TOUCH_OR_RM_DATA_PACKAGE = 9;
    const int32_t EXIT_CELLULAR_DATA_TEST = 1000;
    static const int32_t TEST_DATA_PACKAGE_UP = 0;
    static const int32_t TEST_DATA_PACKAGE_DOWN = 1;
    static const int32_t TEST_DATA_PACKAGE_UP_DOWN = 2;
    static const int32_t TEST_DATA_PACKAGE_EXIT = 100;
};
} // namespace Telephony
} // namespace OHOS

int main()
{
    OHOS::Telephony::CellularDataClient &cellularDataClient = OHOS::Telephony::CellularDataClient::GetInstance();
    if (cellularDataClient.GetProxy() == nullptr) {
        std::cout << "\n--- telephonyService == nullptr\n" << std::endl;
        return 0;
    }
    int32_t inputCMD = 0;
    OHOS::Telephony::CellularDataCodeTest cellularDataCodeTest;
    while (true) {
        std::cout << "\n-----------start--------------\nusage:please input a cmd num:\n"
            "0:IsCellularDataEnabledTest\n"
            "1:EnableCellularDataTest\n"
            "2:GetCellularDataStateTest\n"
            "3:IsCellularDataRoamingEnabledTest\n"
            "4:EnableCellularDataRoamingTest\n"
            "5:ApnChangedTest\n"
            "6:GetDefaultSlotId\n"
            "7:SetDefaultSlotId\n"
            "8:GetCellularDataFlowType\n"
            "9:TouchOrRmDataPackage\n"
            "1000:exit\n"
            << std::endl;
        std::cin >> inputCMD;
        std::cout << "inputCMD is [" << inputCMD << "]" << std::endl;
        if (!cellularDataCodeTest.UnitTest(inputCMD)) {
            break;
        }
    }
    return 0;
}
