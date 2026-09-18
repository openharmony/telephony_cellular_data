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

#ifndef ANI_CELLULAR_DATA_H
#define ANI_CELLULAR_DATA_H

#include <cstdint>
#include "cxx.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AAFwk {
class Want;
}
namespace AbilityRuntime {
class Context;
class AbilityContext;
class UIExtensionContext;
}
namespace CellularDataAni {
struct ArktsError;
struct ApnInfo;
struct AniEnv;
struct AniObject;

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

ArktsError isCellularDataEnabled(bool &dataEnabled);
ArktsError enableCellularDataSync();
ArktsError disableCellularDataSync();
int32_t getDefaultCellularDataSlotIdSync();
ArktsError getCellularDataState(int32_t &CellularDataState);
ArktsError disableCellularDataRoamingSync(int32_t slotId);
ArktsError enableCellularDataRoamingSync(int32_t slotId);
ArktsError isCellularDataRoamingEnabledSync(int32_t slotId, bool &dataEnabled);
ArktsError setDefaultCellularDataSlotIdSyn(int32_t slotId);
int32_t getCellularDataFlowTypeSyn();
ArktsError setPreferredApnSyn(int32_t apnId, bool &ret);
int32_t getDefaultCellularDataSimIdSyn();
ArktsError queryApnIdsSync(const ApnInfo &info, rust::vec<uint32_t> &ret);
ArktsError queryAllApnsSync(rust::vec<ApnInfo> &ret);
ArktsError getActiveApnNameSync(rust::String &apnName);
ArktsError showSystemApnSettingsSync(std::shared_ptr<AbilityRuntime::Context> context);

bool IsStageContext(AniEnv *env, AniObject *obj);
std::shared_ptr<AbilityRuntime::Context> GetStageModeContext(AniEnv **env, AniObject *obj);
bool ParseAbilityContext(std::shared_ptr<AbilityRuntime::Context> context,
    std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> &abilityContext,
    std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> &uiExtensionContext);
OHOS::Ace::UIContent *GetUIContent(std::shared_ptr<AppBaseContext> &asyncContext);
bool StartUiExtensionAbility(OHOS::AAFwk::Want &request, std::shared_ptr<AppBaseContext> &asyncContext);
} // namespace CellularDataAni
} // namespace OHOS
#endif
