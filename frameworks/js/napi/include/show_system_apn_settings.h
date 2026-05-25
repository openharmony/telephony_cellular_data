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

#ifndef SHOW_SYSTEM_APN_SETTINGS_H
#define SHOW_SYSTEM_APN_SETTINGS_H

#include "napi/native_api.h"
#include "ability_context.h"
#include "ui_extension_context.h"

namespace OHOS {
namespace Telephony {

struct AppBaseContext {
    std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext = nullptr;
    std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> uiExtensionContext = nullptr;
};

class ModalUICallback {
public:
    explicit ModalUICallback(std::shared_ptr<AppBaseContext> baseContext);
    void OnRelease(int32_t releaseCode);
    void SetSessionId(int32_t sessionId);

private:
    int32_t sessionId_ = 0;
    std::shared_ptr<AppBaseContext> baseContext_ = nullptr;
    void CloseModalUI();
};

bool ParseAbilityContext(napi_env env, const napi_value &obj,
                         std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> &abilityContext,
                         std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> &uiExtensionContext);
OHOS::Ace::UIContent *GetUIContent(std::shared_ptr<AppBaseContext> &asyncContext);
napi_value ShowSystemApnSettings(napi_env env, napi_callback_info info);

} // namespace Telephony
} // namespace OHOS

#endif // SHOW_SYSTEM_APN_SETTINGS_H