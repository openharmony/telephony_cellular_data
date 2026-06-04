/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "show_system_apn_settings.h"

#include "cellular_data_client.h"
#include "core_service_client.h"
#include "napi_base_context.h"
#include "napi_util.h"
#include "ui_content.h"
#include "telephony_log_wrapper.h"
#include "telephony_permission.h"

namespace OHOS {
namespace Telephony {

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

#define ARGS_ONE 1
#define ARGS_TWO 2
#define PARAM0 0
#define PARAM1 1

struct AsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callbackRef;
    int32_t errorCode = 0;
};

napi_value WrapVoidToJs(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

OHOS::Ace::UIContent *GetUIContent(std::shared_ptr<AppBaseContext> &asyncContext)
{
    if (!asyncContext) return nullptr;
    OHOS::Ace::UIContent *uiContent = nullptr;
    if (asyncContext->abilityContext != nullptr) {
        uiContent = asyncContext->abilityContext->GetUIContent();
    } else if (asyncContext->uiExtensionContext != nullptr) {
        uiContent = asyncContext->uiExtensionContext->GetUIContent();
    }
    return uiContent;
}

ModalUICallback::ModalUICallback(std::shared_ptr<AppBaseContext> baseContext)
{
    baseContext_ = baseContext;
}

void ModalUICallback::CloseModalUI()
{
    auto uiContent = GetUIContent(baseContext_);
    if (uiContent == nullptr) {
        return;
    }
    uiContent->CloseModalUIExtension(sessionId_);
}

void ModalUICallback::SetSessionId(int32_t sessionId)
{
    sessionId_ = sessionId;
}

void ModalUICallback::OnRelease(int32_t releaseCode)
{
    CloseModalUI();
}

bool ParseAbilityContext(napi_env env, const napi_value &obj,
                         std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> &abilityContext,
                         std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> &uiExtensionContext)
{
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, obj, stageMode);
    if (status != napi_ok || !stageMode) {
        return false;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, obj);
    if (context == nullptr) {
        return false;
    }
    abilityContext = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(context);
    if (abilityContext != nullptr) {
        return true;
    }
    uiExtensionContext = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::UIExtensionContext>(context);
    if (uiExtensionContext == nullptr) {
        return false;
    }
    return true;
}

bool StartUiExtensionAbility(OHOS::AAFwk::Want &request, std::shared_ptr<AppBaseContext> &asyncContext)
{
    TELEPHONY_LOGI("StartUiExtensionAbility");
    auto uiContent = GetUIContent(asyncContext);
    if (uiContent == nullptr) {
        TELEPHONY_LOGE("UIContent is nullptr");
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
        TELEPHONY_LOGE("CreateModalUIExtension failed");
        return false;
    }
    callback->SetSessionId(sessionId);
    TELEPHONY_LOGI("StartUiExtensionAbility success, sessionId=%{public}d", sessionId);
    return true;
}

bool CheckParam(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, napi_callback_info info, size_t &argc,
                napi_value *argv)
{
    napi_valuetype valueType;
    napi_status ret = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (ret != napi_ok) {
        return false;
    }
    if (argc == ARGS_ONE) {
        ret = napi_typeof(env, argv[PARAM0], &valueType);
        if (ret != napi_ok || valueType != napi_object) {
            return false;
        }
    } else if (argc == ARGS_TWO) {
        ret = napi_typeof(env, argv[PARAM0], &valueType);
        if (ret != napi_ok || valueType != napi_object) {
            return false;
        }
        ret = napi_typeof(env, argv[PARAM1], &valueType);
        if (ret != napi_ok || valueType != napi_function) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

napi_value SettingsCompletePromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, napi_value result)
{
    TELEPHONY_LOGI("settings complete promise.");
    if (!asyncCallbackInfo) {
        return nullptr;
    }
    napi_value promise;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;
    napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
    return promise;
}

napi_value SettingsInCompletePromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, napi_value result)
{
    TELEPHONY_LOGI("settings incomplete promise.");
    if (!asyncCallbackInfo) {
        return nullptr;
    }
    napi_value promise;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;
    napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
    return promise;
}

napi_value ShowSystemApnSettings(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGI("ShowSystemApnSettings enter.");
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    std::shared_ptr<AsyncCallbackInfo> asyncCallbackInfo = std::make_shared<AsyncCallbackInfo>();
    bool isInvalid = CheckParam(env, asyncCallbackInfo.get(), info, argc, argv);
    if (!isInvalid) {
        napi_value result = WrapVoidToJs(env);
        napi_value promise = SettingsInCompletePromise(env, asyncCallbackInfo.get(), result);
        TELEPHONY_LOGE("ShowSystemApnSettings param is invalid.");
        return promise;
    }
    auto loadProductContext = std::make_shared<AppBaseContext>();
    if (!ParseAbilityContext(env, argv[PARAM0], loadProductContext->abilityContext,
                             loadProductContext->uiExtensionContext)) {
        napi_value result = WrapVoidToJs(env);
        napi_value promise = SettingsInCompletePromise(env, asyncCallbackInfo.get(), result);
        TELEPHONY_LOGE("ShowSystemApnSettings param is invalid.");
        return promise;
    }
    int32_t slotId = CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    bool isSimActive = DelayedRefSingleton<CoreServiceClient>::GetInstance().IsSimActive(slotId);
    TELEPHONY_LOGI("ShowSystemApnSettings slotId %{public}d isSimActive %{public}d", slotId, isSimActive);
    if (!isSimActive) {
        napi_value result = WrapVoidToJs(env);
        napi_value promise = SettingsInCompletePromise(env, asyncCallbackInfo.get(), result);
        return promise;
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
        napi_value result = WrapVoidToJs(env);
        napi_value promise = SettingsInCompletePromise(env, asyncCallbackInfo.get(), result);
        TELEPHONY_LOGE("ShowSystemApnSettings system internal error.");
        return promise;
    }
    napi_value result = WrapVoidToJs(env);
    napi_value promise = SettingsCompletePromise(env, asyncCallbackInfo.get(), result);
    return promise;
}

} // namespace Telephony
} // namespace OHOS
