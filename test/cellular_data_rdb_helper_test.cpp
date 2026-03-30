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
#include "cellular_data_rdb_helper.h"
#include "pdp_profile_data.h"
#include "mock/mock_data_share_result_set.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Telephony;

namespace OHOS {
namespace Telephony {

static PdpProfile g_testPdpProfile;
static int g_currentRow = 0;

class CellularDataRdbHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<DataShareResultSetMock> mockResultSet_;
    std::shared_ptr<DataShare::DataShareResultSet> result_;
    std::vector<PdpProfile> apnVec_;
};

void CellularDataRdbHelperTest::SetUpTestCase()
{
}

void CellularDataRdbHelperTest::TearDownTestCase()
{
}

void CellularDataRdbHelperTest::SetUp()
{
    mockResultSet_ = std::make_shared<DataShareResultSetMock>();
    result_ = mockResultSet_;
    apnVec_.clear();
    g_testPdpProfile = PdpProfile();
    g_currentRow = 0;
}

void CellularDataRdbHelperTest::TearDown()
{
    mockResultSet_.reset();
    result_.reset();
    apnVec_.clear();
}

/**
 * @tc.number: CellularDataRdbHelper_ReadApnResult_NullResult
 * @tc.name: Test handling of null result set
 * @tc.desc: Verify that ReadApnResult handles null result set correctly by returning without processing
 */
HWTEST_F(CellularDataRdbHelperTest, ReadApnResult_NullResult, TestSize.Level1)
{
    CellularDataRdbHelper helper;
    std::shared_ptr<DataShare::DataShareResultSet> nullResult = nullptr;

    helper.ReadApnResult(nullResult, apnVec_);

    EXPECT_EQ(apnVec_.size(), 0);
}

/**
 * @tc.number: CellularDataRdbHelper_ReadApnResult_EmptyResult
 * @tc.name: Test handling of empty result set
 * @tc.desc: Verify that ReadApnResult handles empty result set (rowCnt = 0) correctly
 */
HWTEST_F(CellularDataRdbHelperTest, ReadApnResult_EmptyResult, TestSize.Level1)
{
    int rowCount = 0;
    EXPECT_CALL(*mockResultSet_, GetRowCount(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));

    CellularDataRdbHelper helper;
    helper.ReadApnResult(result_, apnVec_);

    EXPECT_EQ(apnVec_.size(), 0);
}

/**
 * @tc.number: CellularDataRdbHelper_ReadApnResult_NonMvnoApn
 * @tc.name: Test adding non-MVNO type APN
 * @tc.desc: Verify that non-MVNO type APN (mvnoType is empty) is correctly added to the list
 */
HWTEST_F(CellularDataRdbHelperTest, ReadApnResult_NonMvnoApn, TestSize.Level1)
{
    int rowCount = 1;
    g_testPdpProfile.profileId = 1;
    g_testPdpProfile.mvnoType = "";
    g_testPdpProfile.edited = 0;

    EXPECT_CALL(*mockResultSet_, GetRowCount(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));

    for (int i = 0; i < rowCount; ++i) {
        EXPECT_CALL(*mockResultSet_, GoToRow(i))
            .Times(1)
            .WillOnce(Return(0));
        
        // Mock GetColumnIndex and GetInt to return the expected values
        EXPECT_CALL(*mockResultSet_, GetColumnIndex(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(0)));
        
        EXPECT_CALL(*mockResultSet_, GetInt(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(g_testPdpProfile.edited), Return(0)));
        
        EXPECT_CALL(*mockResultSet_, GetString(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(g_testPdpProfile.mvnoType), Return(0)));
    }

    CellularDataRdbHelper helper;
    helper.ReadApnResult(result_, apnVec_);

    EXPECT_EQ(apnVec_.size(), 1);
    EXPECT_TRUE(apnVec_[0].mvnoType.empty());
}

/**
 * @tc.number: CellularDataRdbHelper_ReadApnResult_EditedMvnoApn
 * @tc.name: Test adding edited MVNO type APN
 * @tc.desc: Verify that user-edited MVNO type APN is correctly added to the list
 */
HWTEST_F(CellularDataRdbHelperTest, ReadApnResult_EditedMvnoApn, TestSize.Level1)
{
    int rowCount = 1;
    g_testPdpProfile.profileId = 2;
    g_testPdpProfile.mvnoType = "imsi";
    g_testPdpProfile.edited = 1;

    EXPECT_CALL(*mockResultSet_, GetRowCount(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));

    for (int i = 0; i < rowCount; ++i) {
        EXPECT_CALL(*mockResultSet_, GoToRow(i))
            .Times(1)
            .WillOnce(Return(0));
        
        // Mock GetColumnIndex and GetInt to return the expected values
        EXPECT_CALL(*mockResultSet_, GetColumnIndex(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(0)));
        
        EXPECT_CALL(*mockResultSet_, GetInt(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(g_testPdpProfile.edited), Return(0)));
        
        EXPECT_CALL(*mockResultSet_, GetString(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(g_testPdpProfile.mvnoType), Return(0)));
    }

    CellularDataRdbHelper helper;
    helper.ReadApnResult(result_, apnVec_);

    EXPECT_EQ(apnVec_.size(), 1);
    EXPECT_EQ(apnVec_[0].mvnoType, "imsi");
    EXPECT_EQ(apnVec_[0].edited, 1);
}

/**
 * @tc.number: CellularDataRdbHelper_ReadApnResult_UneditedMvnoApn
 * @tc.name: Test filtering unedited MVNO type APN
 * @tc.desc: Verify that unedited MVNO type APN is correctly filtered out and not added to the list
 */
HWTEST_F(CellularDataRdbHelperTest, ReadApnResult_UneditedMvnoApn, TestSize.Level1)
{
    int rowCount = 1;
    g_testPdpProfile.profileId = 3;
    g_testPdpProfile.mvnoType = "imsi";
    g_testPdpProfile.edited = 0;

    EXPECT_CALL(*mockResultSet_, GetRowCount(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));

    for (int i = 0; i < rowCount; ++i) {
        EXPECT_CALL(*mockResultSet_, GoToRow(i))
            .Times(1)
            .WillOnce(Return(0));
        
        // Mock GetColumnIndex and GetInt to return the expected values
        EXPECT_CALL(*mockResultSet_, GetColumnIndex(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(0)));
        
        EXPECT_CALL(*mockResultSet_, GetInt(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(g_testPdpProfile.edited), Return(0)));
        
        EXPECT_CALL(*mockResultSet_, GetString(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(g_testPdpProfile.mvnoType), Return(0)));
    }

    CellularDataRdbHelper helper;
    helper.ReadApnResult(result_, apnVec_);

    EXPECT_EQ(apnVec_.size(), 0);
}

} // namespace Telephony
} // namespace OHOS
