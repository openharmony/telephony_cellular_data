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
#include "cellular_data_hisysevent.h"
#include "cellular_data_constant.h"
#include "core_manager_inner.h"
#include "core_service_client.h"
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
    Uri cellularDataUri(std::string(CELLULAR_DATA_RDB_SELECTION) + "?simId=" + std::to_string(simId));
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
    Uri cellularDataUri(std::string(CELLULAR_DATA_RDB_SELECTION) + "?simId=" + std::to_string(simId));
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
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_APN_CREATE_HELPER_FAIL, "Create dataHelper fail");
        return false;
    }
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId);
    Uri preferApnUri(std::string(CELLULAR_DATA_RDB_PREFER) + "?simId=" + std::to_string(simId));
    std::shared_ptr<DataShare::DataShareResultSet> result = dataShareHelper->Query(preferApnUri, predicates, columns);
    if (result == nullptr) {
        TELEPHONY_LOGE("query prefer apns error");
        dataShareHelper->Release();
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_APN_QUERY_FAIL, "Query apn fail");
        return false;
    }
    ReadApnResult(result, apnVec);
    result->Close();
    dataShareHelper->Release();
    if (apnVec.size() <= 0) {
        TELEPHONY_LOGI("simid no set prefer apn");
        CellularDataHiSysEvent::WriteDataActivateFaultEvent(slotId, SWITCH_ON,
            CellularDataErrorCode::DATA_ERROR_APN_FOUND_EMPTY, "Apn list is empty");
    }
    return true;
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
    result->GetColumnIndex(PdpProfileData::AUTH_USER, index);
    result->GetString(index, apnBean.authUser);
    int32_t authType;
    result->GetColumnIndex(PdpProfileData::AUTH_TYPE, index);
    result->GetInt(index, authType);
    if (authType == -1) {
        apnBean.authType = (apnBean.authUser.empty()) ? SETUP_DATA_AUTH_NONE : SETUP_DATA_AUTH_PAP_CHAP;
    } else {
        apnBean.authType = authType;
    }
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
    if (apnBean.mvnoType.compare(MvnoType::ICCID) == 0 ||
        apnBean.mvnoType.compare(MvnoType::SPN) == 0 ||
        apnBean.mvnoType.compare(MvnoType::IMSI) == 0) {
        try {
            return std::regex_match(mvnoDataFromSim, std::regex(apnBean.mvnoMatchData));
        } catch (std::regex_error &e) {
            TELEPHONY_LOGE("regex error");
            return false;
        }
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

int32_t CellularDataRdbHelper::GetSimId()
{
    int32_t slotId = CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId();
    if ((slotId < DEFAULT_SIM_SLOT_ID) || (slotId > SIM_SLOT_COUNT)) {
        TELEPHONY_LOGE("slotId invalid slotId = %{public}d", slotId);
        return -1;
    }
    int32_t simId = CoreManagerInner::GetInstance().GetSimId(slotId);
    if (simId <= 0) {
        TELEPHONY_LOGE("simId invalid simId = %{public}d", simId);
        return -1;
    }
    TELEPHONY_LOGI("GetSimId simId = %{public}d", simId);
    return simId;
}

std::string CellularDataRdbHelper::GetOpKey(int slotId)
{
    std::string opkey;
    std::u16string opkeyU16;
    DelayedRefSingleton<CoreServiceClient>::GetInstance().GetOpKey(slotId, opkeyU16);
    opkey = Str16ToStr8(opkeyU16);
    TELEPHONY_LOGI("GetOpKey##slotId = %{public}d, opkey = %{public}s", slotId, opkey.c_str());
    return opkey;
}

void CellularDataRdbHelper::QueryApnIds(const ApnInfo &apnInfo, std::vector<uint32_t> &apnIdList)
{
    if (GetSimId() == -1) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        return;
    }
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(Telephony::PdpProfileData::PROFILE_NAME, Str16ToStr8(apnInfo.apnName));
    predicates.EqualTo(Telephony::PdpProfileData::APN, Str16ToStr8(apnInfo.apn));
    predicates.EqualTo(Telephony::PdpProfileData::MCC, Str16ToStr8(apnInfo.mcc));
    predicates.EqualTo(Telephony::PdpProfileData::MNC, Str16ToStr8(apnInfo.mnc));
    if (strcmp(NOT_FILLED_IN, (Str16ToStr8(apnInfo.user)).c_str()) != 0) {
        predicates.EqualTo(Telephony::PdpProfileData::AUTH_USER, Str16ToStr8(apnInfo.user));
    }
    if (strcmp(NOT_FILLED_IN, (Str16ToStr8(apnInfo.type)).c_str()) != 0) {
        predicates.EqualTo(Telephony::PdpProfileData::APN_TYPES, Str16ToStr8(apnInfo.type));
    }
    if (strcmp(NOT_FILLED_IN, (Str16ToStr8(apnInfo.proxy)).c_str()) != 0) {
        predicates.EqualTo(Telephony::PdpProfileData::PROXY_IP_ADDRESS, Str16ToStr8(apnInfo.proxy));
    }
    if (strcmp(NOT_FILLED_IN, (Str16ToStr8(apnInfo.mmsproxy)).c_str()) != 0) {
        predicates.EqualTo(Telephony::PdpProfileData::MMS_IP_ADDRESS, Str16ToStr8(apnInfo.mmsproxy));
    }
    std::string opkey = GetOpKey(CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId());
    predicates.EqualTo(Telephony::PdpProfileData::OPKEY, opkey);
    Uri cellularDataUri(std::string(CELLULAR_DATA_RDB_SELECTION) + "?simId=" + std::to_string(GetSimId()));
    std::shared_ptr<DataShare::DataShareResultSet> rst = dataShareHelper->Query(cellularDataUri, predicates, columns);
    if (rst == nullptr) {
        TELEPHONY_LOGE("QueryApnIds: query apns error");
        dataShareHelper->Release();
        return;
    }
    int rowCnt = 0;
    rst->GetRowCount(rowCnt);
    TELEPHONY_LOGI("QueryApnIds::query apns rowCnt = %{public}d, opkey= %{public}s", rowCnt, opkey.c_str());
    for (int i = 0; i < rowCnt; ++i) {
        int index = 0;
        int profileId;
        rst->GoToRow(i);
        rst->GetColumnIndex(Telephony::PdpProfileData::PROFILE_ID, index);
        rst->GetInt(index, profileId);
        TELEPHONY_LOGI("profileId: %{public}d", profileId);
        apnIdList.push_back(profileId);
    }
    rst->Close();
    dataShareHelper->Release();
}

int32_t CellularDataRdbHelper::SetPreferApn(int32_t apnId)
{
    if (GetSimId() == -1) {
        return -1;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("SetPreferApn dataShareHelper is null");
        return -1;
    }
    TELEPHONY_LOGI("SetPreferApn apnId:%{public}d", apnId);
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    double profileIdAsDouble = static_cast<double>(apnId);
    double simIdAsDouble = static_cast<double>(GetSimId());
    values.Put(PdpProfileData::PROFILE_ID, profileIdAsDouble);
    values.Put(PdpProfileData::SIM_ID, simIdAsDouble);
    Uri preferApnUri(CELLULAR_DATA_RDB_PREFER);
    int32_t result = dataShareHelper->Update(preferApnUri, predicates, values);
    if (result < TELEPHONY_ERR_SUCCESS) {
        TELEPHONY_LOGE("SetPreferApn fail! result:%{public}d", result);
        dataShareHelper->Release();
        return -1;
    }
    TELEPHONY_LOGI("SetPreferApn result:%{public}d", result);
    dataShareHelper->Release();
    return 0;
}

void CellularDataRdbHelper::GetApnInfo(ApnInfo &apnInfo,
    int rowIndex, std::shared_ptr<DataShare::DataShareResultSet> result)
{
    int index = 0;
    std::string apnName;
    std::string apn;
    std::string mcc;
    std::string mnc;
    std::string user;
    std::string type;
    std::string proxy;
    std::string mmsproxy;
    result->GoToRow(rowIndex);
    result->GetColumnIndex(Telephony::PdpProfileData::PROFILE_NAME, index);
    result->GetString(index, apnName);
    result->GetColumnIndex(Telephony::PdpProfileData::APN, index);
    result->GetString(index, apn);
    result->GetColumnIndex(Telephony::PdpProfileData::MCC, index);
    result->GetString(index, mcc);
    result->GetColumnIndex(Telephony::PdpProfileData::MNC, index);
    result->GetString(index, mnc);
    result->GetColumnIndex(Telephony::PdpProfileData::AUTH_USER, index);
    result->GetString(index, user);
    result->GetColumnIndex(Telephony::PdpProfileData::APN_TYPES, index);
    result->GetString(index, type);
    result->GetColumnIndex(Telephony::PdpProfileData::PROXY_IP_ADDRESS, index);
    result->GetString(index, proxy);
    result->GetColumnIndex(Telephony::PdpProfileData::MMS_IP_ADDRESS, index);
    result->GetString(index, mmsproxy);
    apnInfo.apnName = Str8ToStr16(apnName);
    apnInfo.apn = Str8ToStr16(apn);
    apnInfo.mcc = Str8ToStr16(mcc);
    apnInfo.mnc = Str8ToStr16(mnc);
    apnInfo.user = Str8ToStr16(user);
    apnInfo.type = Str8ToStr16(type);
    apnInfo.proxy = Str8ToStr16(proxy);
    apnInfo.mmsproxy = Str8ToStr16(mmsproxy);
}

void CellularDataRdbHelper::QueryAllApnInfo(std::vector<ApnInfo> &apnInfoList)
{
    TELEPHONY_LOGI("QueryAllApnInfo");
    if (GetSimId() == -1) {
        return;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataAbilityHelper();
    if (dataShareHelper == nullptr) {
        TELEPHONY_LOGE("QueryAllApnInfo dataShareHelper is null");
        return;
    }
    std::vector<std::string> columns;
    DataShare::DataSharePredicates predicates;
    std::string opkey = GetOpKey(CoreManagerInner::GetInstance().GetDefaultCellularDataSlotId());
    predicates.EqualTo(Telephony::PdpProfileData::OPKEY, opkey);
    Uri cellularDataUri(std::string(CELLULAR_DATA_RDB_SELECTION) + "?simId=" + std::to_string(GetSimId()));
    std::shared_ptr<DataShare::DataShareResultSet> result =
        dataShareHelper->Query(cellularDataUri, predicates, columns);
    if (result == nullptr) {
        TELEPHONY_LOGE("QueryAllApnInfo error");
        dataShareHelper->Release();
        return;
    }
    int rowCnt = 0;
    result->GetRowCount(rowCnt);
    TELEPHONY_LOGI("QueryAllApnInfo rowCnt:%{public}d, simId:%{public}d,", rowCnt, GetSimId());
    for (int i = 0; i < rowCnt; ++i) {
        ApnInfo apnInfo;
        GetApnInfo(apnInfo, i, result);
        apnInfoList.push_back(apnInfo);
    }
    result->Close();
    dataShareHelper->Release();
}
} // namespace Telephony
} // namespace OHOS
