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

#include "apn_activate_report_info.h"

namespace OHOS {
namespace Telephony {
bool ApnActivateReportInfoIpc::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(actTimes)) {
        return false;
    }
    if (!parcel.WriteUint32(averDuration)) {
        return false;
    }
    if (!parcel.WriteUint32(topReason)) {
        return false;
    }
    if (!parcel.WriteUint32(actSuccTimes)) {
        return false;
    }
    return true;
}

ApnActivateReportInfoIpc* ApnActivateReportInfoIpc::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<ApnActivateReportInfoIpc> info = std::make_unique<ApnActivateReportInfoIpc>();
    if (info == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadUint32(info->actTimes)) {
        return nullptr;
    }
    if (!parcel.ReadUint32(info->averDuration)) {
        return nullptr;
    }
    if (!parcel.ReadUint32(info->topReason)) {
        return nullptr;
    }
    if (!parcel.ReadUint32(info->actSuccTimes)) {
        return nullptr;
    }
    return info.release();
}

void ApnActivateReportInfoIpc::transferToReportInfo(ApnActivateReportInfoIpc infoIpc, ApnActivateReportInfo &info)
{
    info.actTimes = infoIpc.actTimes;
    info.averDuration = infoIpc.averDuration;
    info.topReason = infoIpc.topReason;
    info.actSuccTimes = infoIpc.actSuccTimes;
}
}
}