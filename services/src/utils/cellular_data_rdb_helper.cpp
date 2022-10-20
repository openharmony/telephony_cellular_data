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

#include "cellular_data_rdb_helper.h"

#include "telephony_log_wrapper.h"

#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
CellularDataRdbHelper::CellularDataRdbHelper() : cellularDataUri_(CELLULAR_DATA_RDB_URI)
{
    helper_ = CreateDataAbilityHelper();
}

CellularDataRdbHelper::~CellularDataRdbHelper() = default;

std::shared_ptr<AppExecFwk::DataAbilityHelper> CellularDataRdbHelper::CreateDataAbilityHelper()
{
    TELEPHONY_LOGI("Create data ability helper");
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        TELEPHONY_LOGE("CellularDataRdbHelper GetSystemAbilityManager failed.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        TELEPHONY_LOGE("CellularDataRdbHelper GetSystemAbility Service Failed.");
        return nullptr;
    }
    return AppExecFwk::DataAbilityHelper::Creator(remoteObj);
}

int CellularDataRdbHelper::Update(
    const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    if (helper_ == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return NULL_POINTER_EXCEPTION;
    }
    TELEPHONY_LOGI("Cellular data RDB helper update");
    int32_t result = helper_->Update(cellularDataUri_, value, predicates);
    helper_->NotifyChange(cellularDataUri_);
    return result;
}

int CellularDataRdbHelper::Insert(const NativeRdb::ValuesBucket &values)
{
    if (helper_ == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return NULL_POINTER_EXCEPTION;
    }
    TELEPHONY_LOGI("Cellular data RDB helper insert");
    int32_t result = helper_->Insert(cellularDataUri_, values);
    helper_->NotifyChange(cellularDataUri_);
    return result;
}

bool CellularDataRdbHelper::QueryApns(const std::string &mcc, const std::string &mnc, std::vector<PdpProfile> &apnVec)
{
    if (helper_ == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return false;
    }
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(PdpProfileData::MCC, mcc)->And()->EqualTo(PdpProfileData::MNC, mnc);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> result = helper_->Query(cellularDataUri_, columns, predicates);
    if (result == nullptr) {
        TELEPHONY_LOGE("CellularDataRdbHelper: query apns error");
        return false;
    }
    ReadApnResult(result, apnVec);
    return true;
}

void CellularDataRdbHelper::ReadApnResult(
    const std::shared_ptr<NativeRdb::AbsSharedResultSet> &result, std::vector<PdpProfile> &apnVec)
{
    if (result == nullptr) {
        TELEPHONY_LOGI("ReadApnResult result is nullptr");
        return;
    }

    int rowCnt = 0;
    int index = 0;
    result->GetRowCount(rowCnt);
    TELEPHONY_LOGI("CellularDataRdbHelper::query apns rowCnt = %{public}d", rowCnt);
    for (int i = 0; i < rowCnt; ++i) {
        PdpProfile apnBean;
        result->GoToRow(i);
        result->GetColumnIndex(PdpProfileData::PROFILE_ID, index);
        result->GetInt(index, apnBean.profileId);
        result->GetColumnIndex(PdpProfileData::PROFILE_NAME, index);
        result->GetString(index, apnBean.profileName);
        result->GetColumnIndex(PdpProfileData::MCC, index);
        result->GetString(index, apnBean.mcc);
        result->GetColumnIndex(PdpProfileData::MNC, index);
        result->GetString(index, apnBean.mnc);
        result->GetColumnIndex(PdpProfileData::APN, index);
        result->GetString(index, apnBean.apn);
        result->GetColumnIndex(PdpProfileData::AUTH_TYPE, index);
        result->GetInt(index, apnBean.authType);
        result->GetColumnIndex(PdpProfileData::AUTH_USER, index);
        result->GetString(index, apnBean.authUser);
        result->GetColumnIndex(PdpProfileData::AUTH_PWD, index);
        result->GetString(index, apnBean.authPwd);
        result->GetColumnIndex(PdpProfileData::APN_TYPES, index);
        result->GetString(index, apnBean.apnTypes);
        result->GetColumnIndex(PdpProfileData::APN_PROTOCOL, index);
        result->GetString(index, apnBean.pdpProtocol);
        result->GetColumnIndex(PdpProfileData::APN_ROAM_PROTOCOL, index);
        result->GetString(index, apnBean.roamPdpProtocol);
        if (apnBean.pdpProtocol.empty()) {
            apnBean.pdpProtocol = "IPV4V6";
        }
        if (apnBean.roamPdpProtocol.empty()) {
            apnBean.roamPdpProtocol = "IPV4V6";
        }
        apnVec.push_back(apnBean);
    }
}

void CellularDataRdbHelper::RegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (helper_ == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return;
    }
    helper_->RegisterObserver(cellularDataUri_, dataObserver);
    TELEPHONY_LOGI("CellularDataRdbHelper::RegisterObserver Success");
}

void CellularDataRdbHelper::UnRegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (helper_ == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return;
    }
    helper_->UnregisterObserver(cellularDataUri_, dataObserver);
    TELEPHONY_LOGI("CellularDataRdbHelper::UnRegisterObserver Success");
}
} // namespace Telephony
} // namespace OHOS
