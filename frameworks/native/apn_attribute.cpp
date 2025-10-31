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

#include "apn_attribute.h"
 
namespace OHOS {
namespace Telephony {

bool ApnAttribute::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(types_)) {
        return false;
    }
    if (!parcel.WriteString(numeric_)) {
        return false;
    }
    if (!parcel.WriteInt32(profileId_)) {
        return false;
    }
    if (!parcel.WriteString(protocol_)) {
        return false;
    }
    if (!parcel.WriteString(roamingProtocol_)) {
        return false;
    }
    if (!parcel.WriteInt32(authType_)) {
        return false;
    }
    if (!parcel.WriteString(apn_)) {
        return false;
    }
    if (!parcel.WriteString(apnName_)) {
        return false;
    }
    if (!parcel.WriteString(user_)) {
        return false;
    }
    if (!parcel.WriteString(password_)) {
        return false;
    }
    if (!parcel.WriteBool(isRoamingApn_)) {
        return false;
    }
    if (!parcel.WriteString(homeUrl_)) {
        return false;
    }
    if (!parcel.WriteString(proxyIpAddress_)) {
        return false;
    }
    if (!parcel.WriteString(mmsIpAddress_)) {
        return false;
    }
    if (!parcel.WriteBool(isEdited_)) {
        return false;
    }
    return true;
}

static bool UnmarshallingExt(Parcel &parcel, ApnAttribute* attribute)
{
    if (!parcel.ReadString(attribute->apnName_)) {
        return false;
    }
    if (!parcel.ReadString(attribute->user_)) {
        return false;
    }
    if (!parcel.ReadString(attribute->password_)) {
        return false;
    }
    if (!parcel.ReadBool(attribute->isRoamingApn_)) {
        return false;
    }
    if (!parcel.ReadString(attribute->homeUrl_)) {
        return false;
    }
    if (!parcel.ReadString(attribute->proxyIpAddress_)) {
        return false;
    }
    if (!parcel.ReadString(attribute->mmsIpAddress_)) {
        return false;
    }
    if (!parcel.ReadBool(attribute->isEdited_)) {
        return false;
    }
    return true;
}

ApnAttribute* ApnAttribute::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<ApnAttribute> attribute = std::make_unique<ApnAttribute>();
    if (attribute == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadString(attribute->types_)) {
        return nullptr;
    }
    if (!parcel.ReadString(attribute->numeric_)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(attribute->profileId_)) {
        return nullptr;
    }
    if (!parcel.ReadString(attribute->protocol_)) {
        return nullptr;
    }
    if (!parcel.ReadString(attribute->roamingProtocol_)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(attribute->authType_)) {
        return nullptr;
    }
    if (!parcel.ReadString(attribute->apn_)) {
        return nullptr;
    }
    if (!UnmarshallingExt(parcel, attribute.get())) {
        return nullptr;
    }
    return attribute.release();
}

void ApnAttribute::TransferApnAttributeBeforeIpc(ApnItem::Attribute &apnAttr, ApnAttribute &apnAfterTrans)
{
    apnAfterTrans.types_ = apnAttr.types_;
    apnAfterTrans.numeric_ = apnAttr.numeric_;
    apnAfterTrans.profileId_ = apnAttr.profileId_;
    apnAfterTrans.protocol_ = apnAttr.protocol_;
    apnAfterTrans.roamingProtocol_ = apnAttr.roamingProtocol_;
    apnAfterTrans.authType_ = apnAttr.authType_;
    apnAfterTrans.apn_ = apnAttr.apn_;
    apnAfterTrans.apnName_ = apnAttr.apnName_;
    apnAfterTrans.user_ = apnAttr.user_;
    apnAfterTrans.password_ = apnAttr.password_;
    apnAfterTrans.isRoamingApn_ = apnAttr.isRoamingApn_;
    apnAfterTrans.homeUrl_ = apnAttr.homeUrl_;
    apnAfterTrans.proxyIpAddress_ = apnAttr.proxyIpAddress_;
    apnAfterTrans.mmsIpAddress_ = apnAttr.mmsIpAddress_;
    apnAfterTrans.isEdited_ = apnAttr.isEdited_;
}

void ApnAttribute::TransferApnAttributeAfterIpc(ApnItem::Attribute &apnAttr, ApnAttribute &apnAfterTrans)
{
    apnAfterTrans.types_.copy(apnAttr.types_, apnAfterTrans.types_.size(), 0);
    apnAfterTrans.numeric_.copy(apnAttr.numeric_, apnAfterTrans.numeric_.size(), 0);
    apnAttr.profileId_ = apnAfterTrans.profileId_;
    apnAfterTrans.protocol_.copy(apnAttr.protocol_, apnAfterTrans.protocol_.size(), 0);
    apnAfterTrans.roamingProtocol_.copy(apnAttr.roamingProtocol_, apnAfterTrans.roamingProtocol_.size(), 0);
    apnAttr.authType_ = apnAfterTrans.authType_;
    apnAfterTrans.apn_.copy(apnAttr.apn_, apnAfterTrans.apn_.size(), 0);
    apnAfterTrans.apnName_.copy(apnAttr.apnName_, apnAfterTrans.apnName_.size(), 0);
    apnAfterTrans.user_.copy(apnAttr.user_, apnAfterTrans.user_.size(), 0);
    apnAfterTrans.password_.copy(apnAttr.password_, apnAfterTrans.password_.size(), 0);
    apnAttr.isRoamingApn_ = apnAfterTrans.isRoamingApn_;
    apnAfterTrans.homeUrl_.copy(apnAttr.homeUrl_, apnAfterTrans.homeUrl_.size(), 0);
    apnAfterTrans.proxyIpAddress_.copy(apnAttr.proxyIpAddress_, apnAfterTrans.proxyIpAddress_.size(), 0);
    apnAfterTrans.mmsIpAddress_.copy(apnAttr.mmsIpAddress_, apnAfterTrans.mmsIpAddress_.size(), 0);
    apnAttr.isEdited_ = apnAfterTrans.isEdited_;
}
}
}