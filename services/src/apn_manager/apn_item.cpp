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
#include "pdp_profile_data.h"

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
        if (type == DATA_CONTEXT_ROLE_INTERNAL_DEFAULT && apnType == DATA_CONTEXT_ROLE_DEFAULT) {
            return true;
        }
        if ((type != DATA_CONTEXT_ROLE_IA) && (apnType == DATA_CONTEXT_ROLE_ALL)) {
            return true;
        }
        if (type == DATA_CONTEXT_ROLE_BIP && apnType == DATA_CONTEXT_ROLE_DEFAULT) {
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
    Attribute attr = {"", "46002", DATA_PROFILE_DEFAULT, "IPV4V6", "IPV4V6",
        DEFAULT_AUTH_TYPE, "cmnet", "CMNET", "", "", false, "", "", "", false, "", 0, "", 0, 0};
    apnItem->apnTypes_ = CellularDataUtils::Split(apnType, ",");
    apnItem->attr_ = attr;
    if (strcpy_s(apnItem->attr_.types_, ALL_APN_ITEM_CHAR_LENGTH, apnType.c_str()) != EOK) {
        TELEPHONY_LOGE("types_ copy fail");
        return nullptr;
    }
    if (apnType == "mms") {
        std::string apn = "cmwap";
        if (strcpy_s(apnItem->attr_.apn_, ALL_APN_ITEM_CHAR_LENGTH, apn.c_str()) != EOK) {
            TELEPHONY_LOGE("types_ copy fail");
            return nullptr;
        }
    }
    TELEPHONY_LOGI("type = %{public}s", apnItem->attr_.types_);
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
    TELEPHONY_LOGI("MakeApn apnTypes_ = %{public}s", apnData.apnTypes.c_str());
    apnItem->attr_.profileId_ = apnData.profileId;
    apnItem->attr_.authType_ = apnData.authType;
    apnItem->attr_.isRoamingApn_ = apnData.isRoamingApn;
    apnItem->attr_.isEdited_ = apnData.edited;
    if (strcpy_s(apnItem->attr_.types_, ALL_APN_ITEM_CHAR_LENGTH, apnData.apnTypes.c_str()) != EOK) {
        TELEPHONY_LOGE("types_ copy fail");
        return nullptr;
    }
    std::string numeric = apnData.mcc + apnData.mnc;
    if (strcpy_s(apnItem->attr_.numeric_, ALL_APN_ITEM_CHAR_LENGTH, numeric.c_str()) != EOK) {
        TELEPHONY_LOGE("numeric_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.protocol_, ALL_APN_ITEM_CHAR_LENGTH, apnData.pdpProtocol.c_str()) != EOK) {
        TELEPHONY_LOGE("protocol_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.roamingProtocol_, ALL_APN_ITEM_CHAR_LENGTH,
        apnData.roamPdpProtocol.c_str()) != EOK) {
        TELEPHONY_LOGE("roamingProtocol_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.apn_, ALL_APN_ITEM_CHAR_LENGTH, apnData.apn.c_str()) != EOK) {
        TELEPHONY_LOGE("apn_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.apnName_, ALL_APN_ITEM_CHAR_LENGTH, apnData.profileName.c_str()) != EOK) {
        TELEPHONY_LOGE("apnName_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.user_, ALL_APN_ITEM_CHAR_LENGTH, apnData.authUser.c_str()) != EOK) {
        TELEPHONY_LOGE("user_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.password_, ALL_APN_ITEM_CHAR_LENGTH, apnData.authPwd.c_str()) != EOK) {
        TELEPHONY_LOGE("password_ copy fail");
        return nullptr;
    }
    return BuildOtherApnAttributes(apnItem, apnData);
}

sptr<ApnItem> ApnItem::BuildOtherApnAttributes(sptr<ApnItem> &apnItem, const PdpProfile &apnData)
{
    if (strcpy_s(apnItem->attr_.homeUrl_, ALL_APN_ITEM_CHAR_LENGTH, apnData.homeUrl.c_str()) != EOK) {
        TELEPHONY_LOGE("homeUrl_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.proxyIpAddress_, ALL_APN_ITEM_CHAR_LENGTH, apnData.proxyIpAddress.c_str()) != EOK) {
        TELEPHONY_LOGE("proxyIpAddress_ copy fail");
        return nullptr;
    }
    if (strcpy_s(apnItem->attr_.mmsIpAddress_, ALL_APN_ITEM_CHAR_LENGTH, apnData.mmsIpAddress.c_str()) != EOK) {
        TELEPHONY_LOGE("mmsIpAddress_ copy fail");
        return nullptr;
    }
    TELEPHONY_LOGI("The APN name is:%{public}s", apnItem->attr_.apnName_);
    return apnItem;
}

bool ApnItem::IsSimilarPdpProfile(const PdpProfile &newPdpProfile, const PdpProfile &oldPdpProfile)
{
    if ((newPdpProfile.apnTypes.find(DATA_CONTEXT_ROLE_DEFAULT) != std::string::npos) &&
        (oldPdpProfile.apnTypes.find(DATA_CONTEXT_ROLE_DEFAULT) != std::string::npos)) {
        return false;
    }
    return (newPdpProfile.apn == oldPdpProfile.apn) &&
        IsSimilarProperty(newPdpProfile.proxyIpAddress, oldPdpProfile.proxyIpAddress) &&
        IsSimilarProtocol(newPdpProfile.pdpProtocol, oldPdpProfile.pdpProtocol) &&
        IsSimilarProtocol(newPdpProfile.roamPdpProtocol, oldPdpProfile.roamPdpProtocol) &&
        (newPdpProfile.mvnoType == oldPdpProfile.mvnoType) &&
        (newPdpProfile.mvnoMatchData == oldPdpProfile.mvnoMatchData) &&
        IsSimilarProperty(newPdpProfile.homeUrl, oldPdpProfile.homeUrl) &&
        IsSimilarProperty(newPdpProfile.mmsIpAddress, oldPdpProfile.mmsIpAddress);
}

bool ApnItem::IsSimilarProtocol(const std::string &newProtocol, const std::string &oldProtocol)
{
    return (newProtocol == oldProtocol) ||
        (newProtocol == PROTOCOL_IPV4V6 && oldProtocol == PROTOCOL_IPV4) ||
        (newProtocol == PROTOCOL_IPV4V6 && oldProtocol == PROTOCOL_IPV6) ||
        (newProtocol == PROTOCOL_IPV4 && oldProtocol == PROTOCOL_IPV4V6) ||
        (newProtocol == PROTOCOL_IPV6 && oldProtocol == PROTOCOL_IPV4V6);
}
} // namespace Telephony
} // namespace OHOS