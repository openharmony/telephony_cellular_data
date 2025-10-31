/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef TEL_CELLULAR_DATA_UTILS_H
#define TEL_CELLULAR_DATA_UTILS_H

namespace OHOS {
namespace Telephony {

    enum CJErrorCode {
        /**
         * The input parameter value is out of range.
         */
        CJ_ERROR_TELEPHONY_ARGUMENT_ERROR = 8300001,

        /**
         * Operation failed. Cannot connect to service.
         */
        CJ_ERROR_TELEPHONY_SERVICE_ERROR = 8300002,

        /**
         * System internal error.
         */
        CJ_ERROR_TELEPHONY_SYSTEM_ERROR = 8300003,

        /**
         * Do not have sim card.
         */
        CJ_ERROR_TELEPHONY_NO_SIM_CARD = 8300004,

        /**
         * Airplane mode is on.
         */
        CJ_ERROR_TELEPHONY_AIRPLANE_MODE_ON = 8300005,

        /**
         * Network not in service.
         */
        CJ_ERROR_TELEPHONY_NETWORK_NOT_IN_SERVICE = 8300006,

        /**
         * Unknown error code.
         */
        CJ_ERROR_TELEPHONY_UNKNOW_ERROR = 8300999,

        /**
         * SIM card is not activated.
         */
        CJ_ERROR_SIM_CARD_IS_NOT_ACTIVE = 8301001,

        /**
         * SIM card operation error.
         */
        CJ_ERROR_SIM_CARD_OPERATION_ERROR = 8301002,

        /**
         * Operator config error.
         */
        CJ_ERROR_OPERATOR_CONFIG_ERROR = 8301003,

        /**
         * Permission verification failed, usually the result returned by VerifyAccessToken.
         */
        CJ_ERROR_TELEPHONY_PERMISSION_DENIED = 201,

        /**
         * Permission verification failed, application which is not a system application uses system API.
         */
        CJ_ERROR_ILLEGAL_USE_OF_SYSTEM_API = 202,
    };

}
}
#endif