/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef APN_ATTRIBUTE_H
#define APN_ATTRIBUTE_H

#include "apn_item.h"

namespace OHOS {
namespace Telephony {
constexpr static int ALL_APN_ITEM_CHAR_LENGTH = 256;
struct ApnAttribute final : public Parcelable {
    std::string types_;
    std::string numeric_;
    int32_t profileId_ = 0;
    std::string protocol_;
    std::string roamingProtocol_;
    int32_t authType_ = 0;
    std::string apn_;
    std::string apnName_;
    std::string user_;
    std::string password_;
    bool isRoamingApn_ = false;
    std::string homeUrl_;
    std::string proxyIpAddress_;
    std::string mmsIpAddress_;
    bool isEdited_ = false;

    bool Marshalling(Parcel &parcel) const override;
    static ApnAttribute* Unmarshalling(Parcel &parcel);

    static void TransferApnAttributeBeforeIpc(ApnItem::Attribute &apnAttr, ApnAttribute &apnAfterTrans);
    static void TransferApnAttributeAfterIpc(ApnItem::Attribute &apnAttr, ApnAttribute &apnAfterTrans);
};
}
}
#endif //APN_ATTRIBUTE_H