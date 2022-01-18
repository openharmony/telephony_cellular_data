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

#include "apn_item.h"

#include <securec.h>

#include "telephony_log_wrapper.h"

#include "cellular_data_utils.h"

namespace OHOS {
namespace Telephony {
ApnItem::ApnItem() = default;

ApnItem::~ApnItem() = default;

std::vector<std::string> ApnItem::GetApnTypes() const
{
    return apnTypes_;
}

void ApnItem::MarkBadApn(bool badApn)
{
    badApn_ = badApn;
}

bool ApnItem::IsBadApn() const
{
    return badApn_;
}

bool ApnItem::CanDealWithType(const std::string &type) const
{
    for (std::string apnType : apnTypes_) {
        transform(apnType.begin(), apnType.end(), apnType.begin(), ::tolower);
        if (type == apnType) {
            return true;
        }
        if ((type != DATA_CONTEXT_ROLE_IA) && (apnType == DATA_CONTEXT_ROLE_ALL)) {
            return true;
        }
    }
    return false;
}

sptr<ApnItem> ApnItem::MakeDefaultApn(const std::string &apnType)
{
    sptr<ApnItem> apnItem = std::make_unique<ApnItem>().release();
    if (apnItem == nullptr) {
        TELEPHONY_LOGE("apn is null");
        return nullptr;
    }
    attribute attr = {"", "46002", DATA_PROFILE_DEFAULT, "IPV4V6", "IPV4V6",
        DEFAULT_AUTH_TYPE, "cmnet", "CMNET", "", ""};
    apnItem->apnTypes_ = CellularDataUtils::Split(apnType, ",");
    apnItem->attr_ = attr;
    if (strcpy_s(apnItem->attr_.types_, apnType.size() + 1, apnType.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeDefaultApn: types_ copy fail");
        return nullptr;
    }
    if (apnType == "mms") {
        std::string apn = "cmwap";
        if (strcpy_s(apnItem->attr_.apn_, apn.size() + 1, apn.c_str()) != EOK) {
            TELEPHONY_LOGE("MakeDefaultApn: types_ copy fail");
            return nullptr;
        }
    }
    TELEPHONY_LOGE("MakeDefaultApn: type = %{public}s", apnItem->attr_.types_);
    return apnItem;
}

sptr<ApnItem> ApnItem::MakeApn(const PdpProfile &apnData)
{
    sptr<ApnItem> apnItem = std::make_unique<ApnItem>().release();
    if (apnItem == nullptr) {
        TELEPHONY_LOGE("apn is null");
        return nullptr;
    }
    apnItem->apnTypes_ = CellularDataUtils::Split(apnData.apnTypes, ",");
    apnItem->attr_.profileId_ = apnData.profileId;
    apnItem->attr_.authType_ = apnData.authType;
    if (strcpy_s(apnItem->attr_.types_, apnData.apnTypes.size() + 1, apnData.apnTypes.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: types_ copy fail");
        return nullptr;
    }
    std::string numeric = apnData.mcc + apnData.mnc;
    if (strcpy_s(apnItem->attr_.numeric_, numeric.size() + 1, numeric.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: types_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.protocol_, apnData.pdpProtocol.size() + 1, apnData.pdpProtocol.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: protocol_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.roamingProtocol_, apnData.roamPdpProtocol.size() + 1,
        apnData.roamPdpProtocol.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: roamingProtocol_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.apn_, apnData.apn.size() + 1, apnData.apn.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: apn_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.apnName_, apnData.profileName.size() + 1, apnData.profileName.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: apnName_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.user_, apnData.authUser.size() + 1, apnData.authUser.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: user_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.password_, apnData.authPwd.size() + 1, apnData.authPwd.c_str()) != EOK) {
        TELEPHONY_LOGE("MakeApn: password_ copy fail");
        return nullptr;
    }
    TELEPHONY_LOGI("MakeApn: The APN name is:%{public}s", apnItem->attr_.apnName_);
    return apnItem;
}
} // namespace Telephony
} // namespace OHOS