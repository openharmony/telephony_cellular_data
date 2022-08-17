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

#include "cellular_data_settings_rdb_helper.h"

#include "telephony_log_wrapper.h"

#include "cellular_data_constant.h"
#include "cellular_data_hisysevent.h"

namespace OHOS {
namespace Telephony {
static constexpr const int32_t E_ERROR = -1;

CellularDataSettingsRdbHelper::CellularDataSettingsRdbHelper()
{
    settingHelper_ = CreateDataAbilityHelper();
}

CellularDataSettingsRdbHelper::~CellularDataSettingsRdbHelper() {}

std::shared_ptr<AppExecFwk::DataAbilityHelper> CellularDataSettingsRdbHelper::CreateDataAbilityHelper()
{
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

void CellularDataSettingsRdbHelper::UnRegisterSettingsObserver(
    const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (settingHelper_ == nullptr) {
        TELEPHONY_LOGE("UnRegister settings observer failed by nullptr");
        return;
    }
    settingHelper_->UnregisterObserver(uri, dataObserver);
    settingHelper_->Release();
}

void CellularDataSettingsRdbHelper::RegisterSettingsObserver(
    const Uri &uri, const sptr<AAFwk::IDataAbilityObserver>&dataObserver)
{
    if (settingHelper_ == nullptr) {
        TELEPHONY_LOGE("Register settings observer by nullptr");
        return;
    }
    settingHelper_->RegisterObserver(uri, dataObserver);
    settingHelper_->Release();
}

void CellularDataSettingsRdbHelper::NotifyChange(const Uri &uri)
{
    if (settingHelper_ == nullptr) {
        TELEPHONY_LOGE("notify settings changed fail by nullptr");
        return;
    }
    settingHelper_->NotifyChange(uri);
    settingHelper_->Release();
}

int CellularDataSettingsRdbHelper::GetValue(Uri &uri, const std::string &column)
{
    int resultValue = 0;
    if (settingHelper_ == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return NULL_POINTER_EXCEPTION;
    }
    NativeRdb::DataAbilityPredicates predicates;
    std::vector<std::string> columns;
    columns.emplace_back(column);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> result = settingHelper_->Query(uri, columns, predicates);
    if (result == nullptr) {
        TELEPHONY_LOGE("setting DB: query error");
        return NULL_POINTER_EXCEPTION;
    }
    settingHelper_->Release();
    result->GoToFirstRow();
    int columnIndex;
    result->GetColumnIndex(column, columnIndex);
    result->GetInt(columnIndex, resultValue);
    result->Close();
    TELEPHONY_LOGI("Query end resultValue is %{public}d", resultValue);
    return resultValue;
}

void CellularDataSettingsRdbHelper::PutValue(Uri &uri, const std::string &column, int value)
{
    if (settingHelper_ == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return;
    }
    int existValue = GetValue(uri, column);
    NativeRdb::ValuesBucket bucket;
    bucket.PutInt(column, value);
    int32_t result;
    if (existValue < 0) {
        result = settingHelper_->Insert(uri, bucket);
    } else {
        NativeRdb::DataAbilityPredicates predicates;
        result = settingHelper_->Update(uri, bucket, predicates);
    }
    TELEPHONY_LOGI("put value return %{public}d", result);
    if (result == E_ERROR) {
        Uri userDataEnableUri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
        Uri userDataRoamingUri(CELLULAR_DATA_SETTING_DATA_ROAMING_URI);
        if (uri == userDataEnableUri) {
            CellularDataHiSysEvent::WriteDataActivateFaultEvent(INVALID_PARAMETER, value,
                CellularDataErrorCode::DATA_ERROR_DATABASE_WRITE_ERROR,
                "SetCellularDataEnable " + std::to_string(value) + " fail");
        } else if (uri == userDataRoamingUri) {
            CellularDataHiSysEvent::WriteDataActivateFaultEvent(INVALID_PARAMETER, value,
                CellularDataErrorCode::DATA_ERROR_DATABASE_WRITE_ERROR,
                "SetUserDataRoamingOn " + std::to_string(value) + " fail");
        } else {
            TELEPHONY_LOGI("result is %{public}d, do not handle.", result);
        }
    }
    settingHelper_->NotifyChange(uri);
    settingHelper_->Release();
}
} // namespace Telephony
} // namespace OHOS