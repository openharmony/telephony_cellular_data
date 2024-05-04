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

#include "cellular_data_constant.h"
#include "core_manager_inner.h"
#include "telephony_log_wrapper.h"

static constexpr const char *SIM_ID = "simId";
namespace OHOS {
namespace Telephony {
CellularDataRdbHelper::CellularDataRdbHelper() : cellularDataUri_(CELLULAR_DATA_RDB_SELECTION) {}

CellularDataRdbHelper::~CellularDataRdbHelper() = default;

std::shared_ptr<DataShare::DataShareHelper> CellularDataRdbHelper::CreateDataAbilityHelper()
{
    TELEPHONY_LOGI("Create data ability helper");
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        TELEPHONY_LOGE("GetSystemAbilityManager failed.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(TELEPHONY_CELLULAR_DATA_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        TELEPHONY_LOGE("GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, CELLULAR_DATA_RDB_URI);
}

int CellularDataRdbHelper::Update(
    const DataShare::DataShareValuesBucket &value, const DataShare::DataSharePredicates &predicates)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return NULL_POINTER_EXCEPTION;
    }
    TELEPHONY_LOGI("Cellular data RDB helper update");
    int32_t result = dataShareHelper->Update(cellularDataUri_, predicates, value);
    dataShareHelper->Release();
    return result;
}

int CellularDataRdbHelper::Insert(const DataShare::DataShareValuesBucket &values)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return NULL_POINTER_EXCEPTION;
    }
    TELEPHONY_LOGI("Cellular data RDB helper insert");
    int32_t result = dataShareHelper->Insert(cellularDataUri_, values);
    dataShareHelper->Release();
    return result;
}

bool CellularDataRdbHelper::ResetApns(int32_t slotId)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return false;
    }
    TELEPHONY_LOGI("Reset apns");
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId);
    Uri resetApnUri(CELLULAR_DATA_RDB_RESET);
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    values.Put(SIM_ID, simId);
    int32_t result = dataShareHelper->Update(resetApnUri, predicates, values);
    dataShareHelper->Release();
    return result >= 0;
}

bool CellularDataRdbHelper::QueryApns(
    const std::string &mcc, const std::string &mnc, std::vector<PdpProfile> &apnVec, int32_t slotId)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return false;
    }
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(PdpProfileData::MCCMNC, mcc + mnc);
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId);
    Uri cellularDataUri(std::string(CELLULAR_DATA_RDB_SELECTION) + "?Proxy=true&simId=" + std::to_string(simId));
    std::shared_ptr<DataShare::DataShareResultSet> result =
        dataShareHelper->Query(cellularDataUri, predicates, columns);
    if (result == nullptr) {
        TELEPHONY_LOGE("query apns error");
        dataShareHelper->Release();
        return false;
    }
    ReadApnResult(result, apnVec);
    result->Close();
    dataShareHelper->Release();
    return true;
}

bool CellularDataRdbHelper::QueryMvnoApnsByType(const std::string &mcc, const std::string &mnc,
    const std::string &mvnoType, const std::string &mvnoDataFromSim, std::vector<PdpProfile> &mvnoApnVec,
    int32_t slotId)
{
    if (mvnoDataFromSim.empty()) {
        TELEPHONY_LOGE("mvnoDataFromSim is empty!");
        return true;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return false;
    }
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(PdpProfileData::MVNO_TYPE, mvnoType)
        ->EqualTo(PdpProfileData::MCCMNC, mcc + mnc);
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId);
    Uri cellularDataUri(std::string(CELLULAR_DATA_RDB_SELECTION) + "?Proxy=true&simId=" + std::to_string(simId));
    std::shared_ptr<DataShare::DataShareResultSet> result =
        dataShareHelper->Query(cellularDataUri, predicates, columns);
    if (result == nullptr) {
        TELEPHONY_LOGE("Query apns error");
        dataShareHelper->Release();
        return false;
    }
    ReadMvnoApnResult(result, mvnoDataFromSim, mvnoApnVec);
    result->Close();
    dataShareHelper->Release();
    return true;
}

bool CellularDataRdbHelper::QueryPreferApn(int32_t slotId, std::vector<PdpProfile> &apnVec)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return false;
    }
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId);
    Uri preferApnUri(std::string(CELLULAR_DATA_RDB_PREFER) + "?Proxy=true&simId=" + std::to_string(simId));
    std::shared_ptr<DataShare::DataShareResultSet> result = dataShareHelper->Query(preferApnUri, predicates, columns);
    if (result == nullptr) {
        TELEPHONY_LOGE("query prefer apns error");
        dataShareHelper->Release();
        return false;
    }
    ReadApnResult(result, apnVec);
    result->Close();
    dataShareHelper->Release();
    if (apnVec.size() > 0) {
        return true;
    }
    return false;
}

void CellularDataRdbHelper::ReadApnResult(
    const std::shared_ptr<DataShare::DataShareResultSet> &result, std::vector<PdpProfile> &apnVec)
{
    if (result == nullptr) {
        TELEPHONY_LOGI("result is nullptr");
        return;
    }
    int rowCnt = 0;
    result->GetRowCount(rowCnt);
    TELEPHONY_LOGI("query apns rowCnt = %{public}d", rowCnt);
    for (int i = 0; i < rowCnt; ++i) {
        PdpProfile apnBean;
        MakePdpProfile(result, i, apnBean);
        if (apnBean.mvnoType.empty()) {
            apnVec.push_back(apnBean);
        }
    }
}

void CellularDataRdbHelper::ReadMvnoApnResult(const std::shared_ptr<DataShare::DataShareResultSet> &result,
    const std::string &mvnoDataFromSim, std::vector<PdpProfile> &apnVec)
{
    if (result == nullptr) {
        TELEPHONY_LOGI("result is nullptr");
        return;
    }
    int rowCnt = 0;
    result->GetRowCount(rowCnt);
    TELEPHONY_LOGI("query mvno apns rowCnt = %{public}d", rowCnt);
    for (int i = 0; i < rowCnt; ++i) {
        PdpProfile apnBean;
        MakePdpProfile(result, i, apnBean);
        if (IsMvnoDataMatched(mvnoDataFromSim, apnBean)) {
            apnVec.push_back(apnBean);
        }
    }
}

void CellularDataRdbHelper::MakePdpProfile(
    const std::shared_ptr<DataShare::DataShareResultSet> &result, int i, PdpProfile &apnBean)
{
    int index = 0;
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
    result->GetColumnIndex(PdpProfileData::MVNO_TYPE, index);
    result->GetString(index, apnBean.mvnoType);
    result->GetColumnIndex(PdpProfileData::MVNO_MATCH_DATA, index);
    result->GetString(index, apnBean.mvnoMatchData);
    result->GetColumnIndex(PdpProfileData::EDITED_STATUS, index);
    result->GetInt(index, apnBean.edited);
    result->GetColumnIndex(PdpProfileData::PROXY_IP_ADDRESS, index);
    result->GetString(index, apnBean.proxyIpAddress);
    if (apnBean.pdpProtocol.empty()) {
        apnBean.pdpProtocol = "IP";
    }
    if (apnBean.roamPdpProtocol.empty()) {
        apnBean.roamPdpProtocol = "IP";
    }
}

bool CellularDataRdbHelper::IsMvnoDataMatched(const std::string &mvnoDataFromSim, const PdpProfile &apnBean)
{
    if (mvnoDataFromSim.empty()) {
        return false;
    }
    if (apnBean.mvnoType.compare(MvnoType::ICCID) == 0) {
        return std::regex_match(mvnoDataFromSim, std::regex(apnBean.mvnoMatchData));
    }
    if (apnBean.mvnoType.compare(MvnoType::SPN) == 0) {
        return std::regex_match(mvnoDataFromSim, std::regex(apnBean.mvnoMatchData));
    }
    if (apnBean.mvnoType.compare(MvnoType::IMSI) == 0) {
        return std::regex_match(mvnoDataFromSim, std::regex(apnBean.mvnoMatchData));
    }
    if (apnBean.mvnoType.compare(MvnoType::GID1) == 0) {
        return mvnoDataFromSim.compare(0, apnBean.mvnoMatchData.size(), apnBean.mvnoMatchData) == 0;
    }
    return false;
}

void CellularDataRdbHelper::RegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return;
    }
    Uri preferApnUri(CELLULAR_DATA_RDB_PREFER);
    Uri resetApnUri(CELLULAR_DATA_RDB_RESET);
    Uri initApnUri(CELLULAR_DATA_RDB_INIT);
    dataShareHelper->RegisterObserver(resetApnUri, dataObserver);
    dataShareHelper->RegisterObserver(preferApnUri, dataObserver);
    dataShareHelper->RegisterObserver(initApnUri, dataObserver);
    dataShareHelper->RegisterObserver(cellularDataUri_, dataObserver);
    dataShareHelper->Release();
    TELEPHONY_LOGI("RegisterObserver Success");
}

void CellularDataRdbHelper::UnRegisterObserver(const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("dataShareHelper is null");
        return;
    }
    Uri preferApnUri(CELLULAR_DATA_RDB_PREFER);
    Uri resetApnUri(CELLULAR_DATA_RDB_RESET);
    Uri initApnUri(CELLULAR_DATA_RDB_INIT);
    dataShareHelper->UnregisterObserver(resetApnUri, dataObserver);
    dataShareHelper->UnregisterObserver(preferApnUri, dataObserver);
    dataShareHelper->UnregisterObserver(initApnUri, dataObserver);
    dataShareHelper->UnregisterObserver(cellularDataUri_, dataObserver);
    dataShareHelper->Release();
    TELEPHONY_LOGI("UnRegisterObserver Success");
}
} // namespace Telephony
} // namespace OHOS
