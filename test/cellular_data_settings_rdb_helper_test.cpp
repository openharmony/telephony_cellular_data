/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#include "cellular_data_constant.h"
#include "cellular_data_settings_rdb_helper.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "telephony_errors.h"
#include "uri.h"

namespace OHOS {
namespace Telephony {
using namespace testing;
using namespace testing::ext;

namespace {
const std::string BACKUP_SIM_LIST_VALUE_VALID = "0,1,0;1,2,1";
const std::string NON_EXISTENT_COLUMN = "non_existent_column_for_test";
}

class CellularDataSettingsRdbHelperTest : public testing::Test {
public:
    CellularDataSettingsRdbHelperTest() = default;
    ~CellularDataSettingsRdbHelperTest() = default;
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}

    bool WriteBackupSimList(const std::string &value)
    {
        auto settingHelper = CellularDataSettingsRdbHelper::GetInstance();
        if (settingHelper == nullptr) {
            return false;
        }
        std::shared_ptr<DataShare::DataShareHelper> helper = settingHelper->CreateDataShareHelper();
        if (helper == nullptr) {
            return false;
        }
        Uri backupSimListUri(CELLULAR_DATA_SETTING_BACKUP_SIM_LIST_URI);
        DataShare::DataSharePredicates predicates;
        predicates.EqualTo(CELLULAR_DATA_COLUMN_KEYWORD, BACKUP_SIM_LIST_COLUMN_ENABLE);
        DataShare::DataShareValuesBucket bucket;
        bucket.Put(CELLULAR_DATA_COLUMN_KEYWORD,
            DataShare::DataShareValueObject(std::string(BACKUP_SIM_LIST_COLUMN_ENABLE)));
        bucket.Put(CELLULAR_DATA_COLUMN_VALUE, DataShare::DataShareValueObject(value));
        int32_t result = helper->Update(backupSimListUri, predicates, bucket);
        if (result <= 0) {
            result = helper->Insert(backupSimListUri, bucket);
        }
        helper->NotifyChange(backupSimListUri);
        helper->Release();
        return result > 0;
    }
};

/**
 * @tc.number: CellularDataSettingsRdbHelper_GetValueString_001
 * @tc.name: Test GetValue string overload with existing value
 * @tc.desc: Verify GetValue string overload returns success and reads back the written backup sim list value
 */
HWTEST_F(CellularDataSettingsRdbHelperTest, CellularDataSettingsRdbHelper_GetValueString_001,
    Function | MediumTest | Level3)
{
    if (!WriteBackupSimList(BACKUP_SIM_LIST_VALUE_VALID)) {
        GTEST_SKIP() << "datashare unavailable, skip string get value case";
    }
    auto settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    ASSERT_NE(settingHelper, nullptr);
    Uri backupSimListUri(CELLULAR_DATA_SETTING_BACKUP_SIM_LIST_URI);
    std::string value;
    ASSERT_EQ(settingHelper->GetValue(backupSimListUri, BACKUP_SIM_LIST_COLUMN_ENABLE, value), TELEPHONY_ERR_SUCCESS);
    ASSERT_EQ(value, BACKUP_SIM_LIST_VALUE_VALID);
}

/**
 * @tc.number: CellularDataSettingsRdbHelper_GetValueString_002
 * @tc.name: Test GetValue string overload with non-existent column
 * @tc.desc: Verify GetValue string overload returns success when the queried column does not exist
 */
HWTEST_F(CellularDataSettingsRdbHelperTest, CellularDataSettingsRdbHelper_GetValueString_002,
    Function | MediumTest | Level3)
{
    auto settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    ASSERT_NE(settingHelper, nullptr);
    Uri backupSimListUri(CELLULAR_DATA_SETTING_BACKUP_SIM_LIST_URI);
    std::string value;
    ASSERT_EQ(settingHelper->GetValue(backupSimListUri, NON_EXISTENT_COLUMN, value), TELEPHONY_ERR_SUCCESS);
}

/**
 * @tc.number: CellularDataSettingsRdbHelper_PutValue_001
 * @tc.name: Test PutValue with int value
 * @tc.desc: Verify PutValue returns success and GetValue int overload reads back the written value
 */
HWTEST_F(CellularDataSettingsRdbHelperTest, CellularDataSettingsRdbHelper_PutValue_001, Function | MediumTest | Level3)
{
    auto settingHelper = CellularDataSettingsRdbHelper::GetInstance();
    ASSERT_NE(settingHelper, nullptr);
    Uri intelligenceNetworkUri(CELLULAR_DATA_SETTING_INTELLIGENCE_NETWORK_URI);
    ASSERT_EQ(settingHelper->PutValue(intelligenceNetworkUri, INTELLIGENCE_NETWORK_COLUMN_ENABLE, 1),
        TELEPHONY_ERR_SUCCESS);
    int32_t value = 0;
    ASSERT_EQ(settingHelper->GetValue(intelligenceNetworkUri, INTELLIGENCE_NETWORK_COLUMN_ENABLE, value),
        TELEPHONY_ERR_SUCCESS);
    ASSERT_EQ(value, 1);

    ASSERT_EQ(settingHelper->PutValue(intelligenceNetworkUri, INTELLIGENCE_NETWORK_COLUMN_ENABLE, 0),
        TELEPHONY_ERR_SUCCESS);
    ASSERT_EQ(settingHelper->GetValue(intelligenceNetworkUri, INTELLIGENCE_NETWORK_COLUMN_ENABLE, value),
        TELEPHONY_ERR_SUCCESS);
    ASSERT_EQ(value, 0);
}
} // namespace Telephony
} // namespace OHOS
