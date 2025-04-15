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
#ifndef APN_ACTIVATE_REPORT_INFO_H
#define APN_ACTIVATE_REPORT_INFO_H

#include "parcel.h"
#include "apn_item.h"
namespace OHOS {
namespace Telephony {
struct ApnActivateReportInfoIpc : public Parcelable {
    uint32_t actTimes;
    uint32_t averDuration;
    uint32_t topReason;
    uint32_t actSuccTimes;

    ApnActivateReportInfoIpc()
    {
        actTimes = 0;
        averDuration = 0;
        topReason = 0;
        actSuccTimes = 0;
    }

    ApnActivateReportInfoIpc(const ApnActivateReportInfo& obj)
    {
        actTimes = obj.actTimes;
        averDuration = obj.averDuration;
        topReason = obj.topReason;
        actSuccTimes = obj.actSuccTimes;
    }

    static void transferToReportInfo(ApnActivateReportInfoIpc infoIpc, ApnActivateReportInfo &info);

    bool Marshalling(Parcel &parcel) const override;
    static ApnActivateReportInfoIpc* Unmarshalling(Parcel &parcel);
};
}
}
#endif //APN_ACTIVATE_REPORT_INFO_H