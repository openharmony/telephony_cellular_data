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

#ifndef APN_ITEM_H
#define APN_ITEM_H

#include <string>
#include <vector>

#include "refbase.h"

#include "pdp_profile_data.h"

#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
class ApnItem : public RefBase {
public:
    ApnItem();
    ~ApnItem();
    std::vector<std::string> GetApnTypes() const;
    bool CanDealWithType(const std::string &type) const;
    void MarkBadApn(bool badApn);
    bool IsBadApn() const;
    static sptr<ApnItem> MakeDefaultApn(const std::string &apnType);
    static sptr<ApnItem> MakeApn(const PdpProfile &apnData);

private:
    static sptr<ApnItem> BuildOtherApnAttributes(sptr<ApnItem> &apnItem, const PdpProfile &apnData);

public:
    constexpr static int ALL_APN_ITEM_CHAR_LENGTH = 256;
    struct Attribute {
        char types_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        char numeric_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        int32_t profileId_ = 0;
        char protocol_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        char roamingProtocol_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        int32_t authType_ = 0;
        char apn_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        char apnName_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        char user_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        char password_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        bool isRoamingApn_ = false;
        char homeUrl_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        char proxyIpAddress_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        char mmsIpAddress_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        bool isEdited_ = false;
    } attr_;

private:
    std::vector<std::string> apnTypes_;
    bool badApn_ = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // APN_ITEM_H
