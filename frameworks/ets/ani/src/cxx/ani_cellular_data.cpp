/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ani_cellular_data.h"
#include "ability_context.h"
#include "ani_base_context.h"
#include "cellular_data_client.h"
#include "core_service_client.h"
#include "cxx.h"
#include "modal_ui_extension_config.h"
#include "napi_util.h"
#include "telephony_types.h"
#include "ui_content.h"
#include "ui_extension_context.h"
#include "want.h"
#include "wrapper.rs.h"

namespace OHOS {
using namespace Telephony;
namespace CellularDataAni {
static constexpr const char *SET_TELEPHONY_STATE = "ohos.permission.SET_TELEPHONY_STATE";
static constexpr const char *GET_NETWORK_INFO = "ohos.permission.GET_NETWORK_INFO";
static constexpr const char *MANAGE_APN_SETTING = "ohos.permission.MANAGE_APN_SETTING";

static constexpr const char *SETTINGS_PACKAGE_NAME = "com.huawei.hmos.callsetting";
static constexpr const char *SETTINGS_ABILITY_NAME = "GeneralCallSettingDialogAbility";
static constexpr const char *UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
static constexpr const char *UIEXTENSION_TYPE_VALUE = "sysDialog/common";
static constexpr const char *DIALOG_REASON_KEY = "dialogReason";
static constexpr const char *DIALOG_REASON_VALUE = "SYSTEM_APN_SETTINGS";
static constexpr const char *SLOT_ID_KEY = "slotId";
static constexpr const char *CONTEXT_TYPE_KEY = "storeKit.ability.contextType";
static constexpr const char *UI_ABILITY_CONTEXT_VALUE = "uiAbility";
static constexpr const char *UI_EXTENSION_CONTEXT_VALUE = "uiExtension";

static bool IsCellularDataManagerInited()
{
    return CellularDataClient::GetInstance().IsConnect();
}

static inline bool IsValidSlotId(int32_t slotId)
{
    if (SIM_SLOT_COUNT == CELLDATA_SLOT_ID_3) {
        return slotId != CELLDATA_SLOT_ID_3 ? ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT - 1)) :
               DelayedRefSingleton<CoreServiceClient>::GetInstance().IsMultiSimsCapabilitySupported(slotId);
    }
    return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT));
}

static inline ArktsError ConvertArktsErrorWithPermission(int32_t errorCode, const std::string &funcName,
                                                         const std::string &permission)
{
    JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(errorCode, funcName, permission);

    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };
    return ArktsErr;
}

ArktsError isCellularDataEnabled(bool &dataEnabled)
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().IsCellularDataEnabled(dataEnabled);
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    return ConvertArktsErrorWithPermission(errorCode, "IsCellularDataEnabled", GET_NETWORK_INFO);
}

ArktsError enableCellularDataSync()
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularData(true);
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    return ConvertArktsErrorWithPermission(errorCode, "enableCellularData", SET_TELEPHONY_STATE);
}

ArktsError disableCellularDataSync()
{
    int32_t errorCode;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularData(false);
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    return ConvertArktsErrorWithPermission(errorCode, "disableCellularData", SET_TELEPHONY_STATE);
}

int32_t getDefaultCellularDataSlotIdSync()
{
    int32_t slotId = -1;
    slotId = CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    return slotId;
}

static int32_t WrapCellularDataType(const int32_t cellularDataType)
{
    switch (cellularDataType) {
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_DISCONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTING): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTING);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_SUSPENDED);
        }
        default: {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_UNKNOWN);
        }
    }
}

ArktsError getCellularDataState(int32_t &CellularDataState)
{
    int32_t errorCode;
    if (IsCellularDataManagerInited()) {
        int32_t dataState = CellularDataClient::GetInstance().GetCellularDataState();
        CellularDataState = WrapCellularDataType(dataState);
        errorCode = TELEPHONY_ERR_SUCCESS;
    } else {
        errorCode = ERROR_SERVICE_UNAVAILABLE;
    }

    JsError error = NapiUtil::ConverErrorMessageForJs(errorCode);
    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };
    return ArktsErr;
}

ArktsError disableCellularDataRoamingSync(int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "disableCellularDataRoaming",
                                               SET_TELEPHONY_STATE);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, false);
    }

    return ConvertArktsErrorWithPermission(errorCode, "disableCellularDataRoaming", SET_TELEPHONY_STATE);
}

ArktsError enableCellularDataRoamingSync(int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "enableCellularDataRoaming", SET_TELEPHONY_STATE);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().EnableCellularDataRoaming(slotId, true);
    }

    return ConvertArktsErrorWithPermission(errorCode, "enableCellularDataRoaming", SET_TELEPHONY_STATE);
}

ArktsError isCellularDataRoamingEnabledSync(int32_t slotId, bool &dataEnabled)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "isCellularDataRoamingEnabled", GET_NETWORK_INFO);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().IsCellularDataRoamingEnabled(slotId, dataEnabled);
    }

    return ConvertArktsErrorWithPermission(errorCode, "isCellularDataRoamingEnabled", GET_NETWORK_INFO);
}

ArktsError setDefaultCellularDataSlotIdSyn(int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        return ConvertArktsErrorWithPermission(ERROR_SLOT_ID_INVALID, "setDefaultCellularDataSlotId",
                                               SET_TELEPHONY_STATE);
    }

    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;

    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(slotId);
    }

    return ConvertArktsErrorWithPermission(errorCode, "setDefaultCellularDataSlotId", SET_TELEPHONY_STATE);
}

int32_t getCellularDataFlowTypeSyn()
{
    return CellularDataClient::GetInstance().GetCellularDataFlowType();
}

ArktsError setPreferredApnSyn(int32_t apnId, bool &ret)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().SetPreferApn(apnId);
    }
    ret = errorCode == TELEPHONY_SUCCESS;
    return ConvertArktsErrorWithPermission(errorCode, "setPreferredApn", MANAGE_APN_SETTING);
}

int32_t getDefaultCellularDataSimIdSyn()
{
    int32_t simId = 0;
    return CellularDataClient::GetInstance().GetDefaultCellularDataSimId(simId);
}

std::u16string Utf8ToU16String(const std::string &str)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str);
}

std::string U16StringToUtf8(const std::u16string &str)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(str);
}

ArktsError queryApnIdsSync(const ApnInfo &info, rust::vec<uint32_t> &ret)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    if (IsCellularDataManagerInited()) {
        std::vector<uint32_t> apnIdList;
        OHOS::Telephony::ApnInfo apnInfo;
        apnInfo.apnName = Utf8ToU16String(std::string(info.apnName));
        apnInfo.apn = Utf8ToU16String(std::string(info.apn));
        apnInfo.mcc = Utf8ToU16String(std::string(info.mcc));
        apnInfo.mnc = Utf8ToU16String(std::string(info.mnc));
        apnInfo.user = Utf8ToU16String(std::string(info.user));
        apnInfo.type = Utf8ToU16String(std::string(info.type_));
        apnInfo.proxy = Utf8ToU16String(std::string(info.proxy));
        apnInfo.mmsproxy = Utf8ToU16String(std::string(info.mmsproxy));
        errorCode = CellularDataClient::GetInstance().QueryApnIds(apnInfo, apnIdList);
        if (errorCode == TELEPHONY_SUCCESS) {
            for (auto apnId : apnIdList) {
                ret.push_back(apnId);
            }
        }
    }
    return ConvertArktsErrorWithPermission(errorCode, "queryApnIds", MANAGE_APN_SETTING);
}

ArktsError queryAllApnsSync(rust::vec<ApnInfo> &ret)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    std::vector<OHOS::Telephony::ApnInfo> apnInfoList;
    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().QueryAllApnInfo(apnInfoList);
    }
    for (auto info : apnInfoList) {
        ret.push_back(ApnInfo{
            .apnName = rust::string(U16StringToUtf8(info.apnName)),
            .apn = rust::string(U16StringToUtf8(info.apn)),
            .mcc = rust::string(U16StringToUtf8(info.mcc)),
            .mnc = rust::string(U16StringToUtf8(info.mnc)),
            .user = rust::string(U16StringToUtf8(info.user)),
            .type_ = rust::string(U16StringToUtf8(info.type)),
            .proxy = rust::string(U16StringToUtf8(info.proxy)),
            .mmsproxy = rust::string(U16StringToUtf8(info.mmsproxy)),
        });
    }

    return ConvertArktsErrorWithPermission(errorCode, "queryAllApns", MANAGE_APN_SETTING);
}

ArktsError getActiveApnNameSync(rust::String &apnName)
{
    int32_t errorCode = ERROR_SERVICE_UNAVAILABLE;
    std::string apnNameStr;
    if (IsCellularDataManagerInited()) {
        errorCode = CellularDataClient::GetInstance().GetActiveApnName(apnNameStr);
    }
    apnName = rust::string(apnNameStr);
    return ConvertArktsErrorWithPermission(errorCode, "GetActiveApnName", GET_NETWORK_INFO);
}

ModalUICallback::ModalUICallback(std::shared_ptr<AppBaseContext> baseContext)
{
    baseContext_ = baseContext;
}

OHOS::Ace::UIContent *GetUIContent(std::shared_ptr<AppBaseContext> &asyncContext)
{
    if (!asyncContext) {
        return nullptr;
    }
    OHOS::Ace::UIContent *uiContent = nullptr;
    if (asyncContext->abilityContext != nullptr) {
        uiContent = asyncContext->abilityContext->GetUIContent();
    } else if (asyncContext->uiExtensionContext != nullptr) {
        uiContent = asyncContext->uiExtensionContext->GetUIContent();
    }
    return uiContent;
}

void ModalUICallback::CloseModalUI()
{
    auto uiContent = GetUIContent(baseContext_);
    if (uiContent == nullptr) {
        return;
    }
    uiContent->CloseModalUIExtension(sessionId_);
}

void ModalUICallback::OnRelease(int32_t releaseCode)
{
    CloseModalUI();
}

void ModalUICallback::SetSessionId(int32_t sessionId)
{
    sessionId_ = sessionId;
}

bool StartUiExtensionAbility(OHOS::AAFwk::Want &request, std::shared_ptr<AppBaseContext> &asyncContext)
{
    auto uiContent = GetUIContent(asyncContext);
    if (uiContent == nullptr) {
        return false;
    }
    auto callback = std::make_shared<ModalUICallback>(asyncContext);
    OHOS::Ace::ModalUIExtensionCallbacks extensionCallbacks = {
        std::bind(&ModalUICallback::OnRelease, callback, std::placeholders::_1)
    };
    OHOS::Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = false;
    int32_t sessionId = uiContent->CreateModalUIExtension(request, extensionCallbacks, config);
    if (sessionId == 0) {
        return false;
    }
    callback->SetSessionId(sessionId);
    return true;
}

bool ParseAbilityContext(std::shared_ptr<AbilityRuntime::Context> context,
    std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
    std::shared_ptr<AbilityRuntime::UIExtensionContext> &uiExtensionContext)
{
    if (context == nullptr) {
        return false;
    }
    abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (abilityContext != nullptr) {
        return true;
    }
    uiExtensionContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(context);
    if (uiExtensionContext == nullptr) {
        return false;
    }
    return true;
}

ArktsError showSystemApnSettingsSync(std::shared_ptr<AbilityRuntime::Context> context)
{
    auto loadProductContext = std::make_shared<AppBaseContext>();
    if (!ParseAbilityContext(context, loadProductContext->abilityContext,
                             loadProductContext->uiExtensionContext)) {
        return ConvertArktsErrorWithPermission(
            ERROR_PARAMETER_TYPE_INVALID, "ShowSystemApnSettings", MANAGE_APN_SETTING);
    }
    int32_t slotId = CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    bool isSimActive = DelayedRefSingleton<CoreServiceClient>::GetInstance().IsSimActive(slotId);
    if (!isSimActive) {
        return ConvertArktsErrorWithPermission(TELEPHONY_ERR_NO_SIM_CARD, "ShowSystemApnSettings", MANAGE_APN_SETTING);
    }
    OHOS::AAFwk::Want want;
    want.SetElementName(std::string(SETTINGS_PACKAGE_NAME), std::string(SETTINGS_ABILITY_NAME));
    want.SetParam(std::string(UIEXTENSION_TYPE_KEY), std::string(UIEXTENSION_TYPE_VALUE));
    want.SetParam(std::string(DIALOG_REASON_KEY), std::string(DIALOG_REASON_VALUE));
    want.SetParam(std::string(SLOT_ID_KEY), slotId);
    want.SetParam(std::string(CONTEXT_TYPE_KEY),
        loadProductContext->uiExtensionContext != nullptr ?
        std::string(UI_EXTENSION_CONTEXT_VALUE) : std::string(UI_ABILITY_CONTEXT_VALUE));

    if (!StartUiExtensionAbility(want, loadProductContext)) {
        return ConvertArktsErrorWithPermission(ERROR_SERVICE_UNAVAILABLE, "ShowSystemApnSettings", MANAGE_APN_SETTING);
    }
    return ConvertArktsErrorWithPermission(TELEPHONY_ERR_SUCCESS, "ShowSystemApnSettings", MANAGE_APN_SETTING);
}

bool IsStageContext(AniEnv *env, AniObject *obj)
{
    if (env == nullptr || obj == nullptr) {
        return false;
    }
    ani_env *aniEnv = *reinterpret_cast<ani_env **>(env);
    ani_boolean stageMode;
    ani_status status = AbilityRuntime::IsStageContext(aniEnv,
        *reinterpret_cast<ani_object *>(obj), stageMode);
    if (status != ANI_OK) {
        return false;
    }
    return stageMode == 1;
}

std::shared_ptr<AbilityRuntime::Context> GetStageModeContext(AniEnv **env, AniObject *obj)
{
    if (env == nullptr || *env == nullptr || obj == nullptr) {
        return nullptr;
    }
    return AbilityRuntime::GetStageModeContext(reinterpret_cast<ani_env *>(*env),
                                               *reinterpret_cast<ani_object *>(obj));
}
} // namespace CellularDataAni
} // namespace OHOS