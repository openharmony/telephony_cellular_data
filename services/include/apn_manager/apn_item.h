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

#include "cellular_data_constant.h"

namespace OHOS {
namespace Telephony {
struct PdpProfile;
struct ApnActivateInfo {
    uint64_t actSuccTime;
    uint32_t duration;
    uint32_t reason;
    uint32_t apnId;
};

struct ApnActivateReportInfo {
    uint32_t actTimes;
    uint32_t averDuration;
    uint32_t topReason;
    uint32_t actSuccTimes;
};
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
    static bool IsSimilarPdpProfile(const PdpProfile &newPdpProfile, const PdpProfile &oldPdpProfile);
    static inline bool IsSimilarProperty(const std::string &newProp, const std::string &oldProp)
    {
        return (newProp == oldProp) || (newProp.empty()) || (oldProp.empty());
    }

private:
    static sptr<ApnItem> BuildOtherApnAttributes(sptr<ApnItem> &apnItem, const PdpProfile &apnData);
    static bool IsSimilarProtocol(const std::string &newProtocol, const std::string &oldProtocol);

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
        /* For networkslice*/
        char snssai_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        uint8_t sscMode_ = 0;
        char dnn_[ALL_APN_ITEM_CHAR_LENGTH] = { 0 };
        int32_t PduSessionType_ = 0;
        uint8_t RouteBitmap_ = 0;
    } attr_;

private:
    std::vector<std::string> apnTypes_;
    bool badApn_ = false;
};
} // namespace Telephony
} // namespace OHOS
#endif // APN_ITEM_H
