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

#ifndef NETWORK_SEARCH_UTILS_H
#define NETWORK_SEARCH_UTILS_H

#include "event_handler.h"

#include "inner_event.h"
#include "i_net_conn_service.h"

#include "core_manager.h"

namespace OHOS {
namespace Telephony {
class NetworkSearchUtils {
public:
    /**
     * Gets the roaming status of the specified SIM card
     *
     * @param slotId specified sim card
     * @return return true if roaming else false
     */
    static bool GetRoamingState(int32_t slotId);

    /**
     * Gets the attachment status of the data service
     *
     * @param slotId specified sim card
     * @return Data as defined by the RegServiceState.
     */
    static bool GetAttachedState(int32_t slotId);

    /**
     * Technology for acquiring access points
     *
     * @param slotId specified sim card
     * @return Data as defined by the RadioTech.
     */
    static int32_t GetPsRadioTech(int32_t slotId);

    /**
     * Set the radio status
     *
     * @param slotId specified sim card
     * @param fun setting the status of the modem is required
     * @param rst set more parameters
     * @param response Response after sending the request
     */
    static void SetRadioState(int32_t slotId, int fun, int rst, const AppExecFwk::InnerEvent::Pointer &response);

    /**
     * Get the operator display name
     *
     * @param slotId specified sim card
     * @return operator display name
     */
    static std::string GetOperatorNumeric(int32_t slotId);

    static void DcPhysicalLinkActiveUpdate(int32_t slotId, bool isActive);

    /**
     * Conversion corresponding access point name according to the access point technology
     *
     * @param radioTech Access point technology
     * @return access technology name
     */
    static std::string ConvertRadioTechToRadioName(const int32_t radioTech);

private:
    NetworkSearchUtils() = default;

    ~NetworkSearchUtils() = default;
};
} // namespace Telephony
} // namespace OHOS
#endif // NETWORK_SEARCH_UTILS_H
