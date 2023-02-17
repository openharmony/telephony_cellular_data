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

#include "cellular_data_constant.h"
#include "cellular_data_error.h"
#include "cellular_data_hisysevent.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
static constexpr const int32_t E_ERROR = -1;

CellularDataSettingsRdbHelper::CellularDataSettingsRdbHelper() {}

CellularDataSettingsRdbHelper::~CellularDataSettingsRdbHelper() {}

std::shared_ptr<DataShare::DataShareHelper> CellularDataSettingsRdbHelper::CreateDataShareHelper()
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
    return DataShare::DataShareHelper::Creator(remoteObj, CELLULAR_DATA_SETTING_URI);
}

void CellularDataSettingsRdbHelper::UnRegisterSettingsObserver(
    const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    std::shared_ptr<DataShare::DataShareHelper> settingHelper = CreateDataShareHelper();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("UnRegister settings observer failed by nullptr");
        return;
    }
    settingHelper->UnregisterObserver(uri, dataObserver);
    settingHelper->Release();
}

void CellularDataSettingsRdbHelper::RegisterSettingsObserver(
    const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    std::shared_ptr<DataShare::DataShareHelper> settingHelper = CreateDataShareHelper();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("Register settings observer by nullptr");
        return;
    }
    settingHelper->RegisterObserver(uri, dataObserver);
    settingHelper->Release();
}

void CellularDataSettingsRdbHelper::NotifyChange(const Uri &uri)
{
    std::shared_ptr<DataShare::DataShareHelper> settingHelper = CreateDataShareHelper();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("notify settings changed fail by nullptr");
        return;
    }
    settingHelper->NotifyChange(uri);
    settingHelper->Release();
}

int32_t CellularDataSettingsRdbHelper::GetValue(Uri &uri, const std::string &column, int32_t &value)
{
    std::shared_ptr<DataShare::DataShareHelper> settingHelper = CreateDataShareHelper();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    predicates.EqualTo(CELLULAR_DATA_COLUMN_KEYWORD, column);
    auto result = settingHelper->Query(uri, predicates, columns);
    if (result == nullptr) {
        TELEPHONY_LOGE("setting DB: query error");
        return TELEPHONY_ERR_DATABASE_READ_FAIL;
    }
    settingHelper->Release();
    result->GoToFirstRow();
    int32_t columnIndex;
    std::string resultValue;
    result->GetColumnIndex(CELLULAR_DATA_COLUMN_VALUE, columnIndex);
    result->GetString(columnIndex, resultValue);
    result->Close();
    TELEPHONY_LOGI("Query end resultValue is %{public}s", resultValue.c_str());
    if (resultValue.empty()) {
        TELEPHONY_LOGE("resultValue is empty");
        return TELEPHONY_ERR_DATABASE_READ_FAIL;
    }
    value = atoi(resultValue.c_str());
    return TELEPHONY_ERR_SUCCESS;
}

int32_t CellularDataSettingsRdbHelper::PutValue(Uri &uri, const std::string &column, int value)
{
    std::shared_ptr<DataShare::DataShareHelper> settingHelper = CreateDataShareHelper();
    if (settingHelper == nullptr) {
        TELEPHONY_LOGE("helper_ is null");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    int32_t existValue = 0;
    int32_t getValueRet = GetValue(uri, column, existValue);
    DataShare::DataShareValueObject keyObj(column);
    DataShare::DataShareValueObject valueObj(std::to_string(value));
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(CELLULAR_DATA_COLUMN_VALUE, valueObj);
    bucket.Put(CELLULAR_DATA_COLUMN_KEYWORD, keyObj);
    int32_t result;
    if (getValueRet != TELEPHONY_ERR_SUCCESS) {
        result = settingHelper->Insert(uri, bucket);
    } else {
        DataShare::DataSharePredicates predicates;
        predicates.EqualTo(CELLULAR_DATA_COLUMN_KEYWORD, column);
        result = settingHelper->Update(uri, predicates, bucket);
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
        settingHelper->Release();
        return TELEPHONY_ERR_DATABASE_WRITE_FAIL;
    }
    settingHelper->NotifyChange(uri);
    settingHelper->Release();
    return TELEPHONY_ERR_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS
