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

#include "napi_cellular_data.h"

#include <memory>

#include "cellular_data_client.h"
#include "cellular_data_types.h"
#include "cstddef"
#include "iosfwd"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "napi/native_common.h"
#include "napi_parameter_util.h"
#include "napi_util.h"
#include "node_api.h"
#include "string"
#include "telephony_log_wrapper.h"
#include "telephony_napi_common_error.h"

namespace OHOS {
namespace Telephony {
static constexpr int32_t DEFAULT_REF_COUNT = 1;
static constexpr const char *SET_TELEPHONY_STATE = "ohos.permission.SET_TELEPHONY_STATE";
static constexpr const char *GET_NETWORK_INFO = "ohos.permission.GET_NETWORK_INFO";
static constexpr const char *MANAGE_APN_SETTING = "ohos.permission.MANAGE_APN_SETTING";

struct AsyncPara {
    std::string funcName = "";
    napi_env env = nullptr;
    napi_callback_info info = nullptr;
    napi_async_execute_callback execute = nullptr;
    napi_async_complete_callback complete = nullptr;
};

static bool IsCellularDataManagerInited()
{
    return CellularDataClient::GetInstance().IsConnect();
}

static inline bool IsValidSlotId(int32_t slotId)
{
    return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT));
}

static inline bool IsValidSlotIdEx(int32_t slotId)
{
    // One more slot for VSim.
    return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT + 1));
}

static bool MatchCellularDataParameters(napi_env env, const napi_value parameters[], const size_t parameterCount)
{
    switch (parameterCount) {
        case 0:
            return true;
        case 1:
            return NapiUtil::MatchParameters(env, parameters, {napi_function});
        default:
            return false;
    }
}

static bool MatchEnableCellularDataRoamingParameters(
    napi_env env, const napi_value parameters[], const size_t parameterCount)
{
    const size_t paramLimitOne = 1;
    const size_t paramLimitTwo = 2;
    switch (parameterCount) {
        case paramLimitOne:
            return NapiUtil::MatchParameters(env, parameters, {napi_number});
        case paramLimitTwo:
            return NapiUtil::MatchParameters(env, parameters, {napi_number, napi_function});
        default:
            return false;
    }
}

static int32_t WrapCellularDataType(const int32_t cellularDataType)
{
    switch (cellularDataType) {
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_DISCONNECTED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_DISCONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTING): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTING);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED): {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_SUSPENDED);
        }
        default: {
            return static_cast<int32_t>(DataConnectState::DATA_STATE_UNKNOWN);
        }
    }
}

static int32_t WrapGetCellularDataFlowTypeType(const int32_t cellularDataType)
{
    switch (cellularDataType) {
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE): {
            return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN): {
            return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP): {
            return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN): {
            return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT): {
            return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
        }
        default: {
            return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE);
        }
    }
}

static void NativeGetCellularDataState(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        return;
    }
    if (IsCellularDataManagerInited()) {
        int32_t result = CellularDataClient::GetInstance().GetCellularDataState();
        if (result == TELEPHONY_ERR_PERMISSION_ERR) {
            asyncContext->errorCode = result;
        } else {
            asyncContext->resolved = true;
            asyncContext->result = WrapCellularDataType(result);
        }
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

static void GetCellularDataStateCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncContext->result, &callbackValue));
    } else {
        if (asyncContext->errorCode == ERROR_SERVICE_UNAVAILABLE) {
            callbackValue =
                NapiUtil::CreateErrorMessage(env, "cellular data service unavailable", ERROR_SERVICE_UNAVAILABLE);
        } else {
            JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "GetCellularDataState", GET_NETWORK_INFO);
            callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
        }
    }
    NapiUtil::Handle2ValueCallback(env, asyncContext.release(), callbackValue);
}

static napi_value GetCellularDataState(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[1];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data));
    NAPI_ASSERT(env, MatchCellularDataParameters(env, parameters, parameterCount), "type mismatch");
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        std::string errorCode = std::to_string(napi_generic_failure);
        std::string errorMessage = "error at baseContext is nullptr";
        NAPI_CALL(env, napi_throw_error(env, errorCode.c_str(), errorMessage.c_str()));
        return nullptr;
    }
    if (parameterCount == 1) {
        NAPI_CALL(env, napi_create_reference(env, parameters[0], DEFAULT_REF_COUNT, &asyncContext->callbackRef));
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "GetCellularDataState",
        NativeGetCellularDataState, GetCellularDataStateCallback);
}

static void NativeIsCellularDataEnabled(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (IsCellularDataManagerInited()) {
        bool enabled = false;
        asyncContext->errorCode = CellularDataClient::GetInstance().IsCellularDataEnabled(enabled);
        if (asyncContext->errorCode == TELEPHONY_SUCCESS) {
            asyncContext->resolved = true;
        }
        asyncContext->result = enabled;
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

static void IsCellularDataEnabledCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        napi_status status = napi_get_boolean(env,
            asyncContext->result == static_cast<int32_t>(DataSwitchCode::CELLULAR_DATA_ENABLED), &callbackValue);
        NAPI_CALL_RETURN_VOID(env, status);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "IsCellularDataEnabled", GET_NETWORK_INFO);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle2ValueCallback(env, asyncContext.release(), callbackValue);
}

static napi_value IsCellularDataEnabled(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[1];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data));
    if (!MatchCellularDataParameters(env, parameters, parameterCount)) {
        TELEPHONY_LOGE("IsCellularDataEnabled MatchCellularDataParameters failed.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        TELEPHONY_LOGE("IsCellularDataEnabled asyncContext is nullptr.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    if (parameterCount == 1) {
        NAPI_CALL(env, napi_create_reference(env, parameters[0], DEFAULT_REF_COUNT, &(asyncContext->callbackRef)));
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "IsCellularDataEnabled",
        NativeIsCellularDataEnabled, IsCellularDataEnabledCallback);
}

static napi_value IsCellularDataEnabledSync(napi_env env, napi_callback_info info)
{
    bool isEnabled = false;
    napi_value value = nullptr;
    if (IsCellularDataManagerInited()) {
        auto errorCode = CellularDataClient::GetInstance().IsCellularDataEnabled(isEnabled);
        if (errorCode != TELEPHONY_SUCCESS) {
            JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
                errorCode, "isCellularDataEnabledSync", GET_NETWORK_INFO);
            NapiUtil::ThrowError(env, error.errorCode, error.errorMessage);
            return value;
        }
    } else {
        return value;
    }
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &value));
    return value;
}

static void NativeEnableCellularData(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (IsCellularDataManagerInited()) {
        asyncContext->result = CellularDataClient::GetInstance().EnableCellularData(true);
        if (asyncContext->result == TELEPHONY_ERR_SUCCESS) {
            asyncContext->resolved = true;
        } else {
            asyncContext->resolved = false;
            asyncContext->errorCode = asyncContext->result;
        }
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

static void EnableCellularDataCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        callbackValue = NapiUtil::CreateUndefined(env);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "enableCellularData", SET_TELEPHONY_STATE);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle1ValueCallback(env, asyncContext.release(), callbackValue);
    TELEPHONY_LOGI("EnableCellularDataCallback end");
}

static void DisableCellularDataCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        callbackValue = NapiUtil::CreateUndefined(env);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "disableCellularData", SET_TELEPHONY_STATE);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle1ValueCallback(env, asyncContext.release(), callbackValue);
    TELEPHONY_LOGI("DisableCellularDataCallback end");
}

static void EnableCellularDataRoamingCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        callbackValue = NapiUtil::CreateUndefined(env);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "enableCellularDataRoaming", SET_TELEPHONY_STATE);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle1ValueCallback(env, asyncContext.release(), callbackValue);
    TELEPHONY_LOGI("EnableCellularDataRoamingCallback end");
}

static void DisableCellularDataRoamingCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        callbackValue = NapiUtil::CreateUndefined(env);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "disableCellularDataRoaming", SET_TELEPHONY_STATE);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle1ValueCallback(env, asyncContext.release(), callbackValue);
    TELEPHONY_LOGI("DisableCellularDataRoamingCallback end");
}

static napi_value EnableCellularData(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[1];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data));
    if (!MatchCellularDataParameters(env, parameters, parameterCount)) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    if (parameterCount == 1) {
        NAPI_CALL(env, napi_create_reference(env, parameters[0], DEFAULT_REF_COUNT, &(asyncContext->callbackRef)));
    }
    return NapiUtil::HandleAsyncWork(
        env, asyncContext.release(), "EnableCellularData", NativeEnableCellularData, EnableCellularDataCallback);
}

static void NativeDisableCellularData(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (IsCellularDataManagerInited()) {
        asyncContext->result = CellularDataClient::GetInstance().EnableCellularData(false);
        if (asyncContext->result == TELEPHONY_ERR_SUCCESS) {
            asyncContext->resolved = true;
        } else {
            asyncContext->resolved = false;
            asyncContext->errorCode = asyncContext->result;
        }
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

static napi_value DisableCellularData(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[1];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data));
    if (!MatchCellularDataParameters(env, parameters, parameterCount)) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    if (parameterCount == 1) {
        NAPI_CALL(env, napi_create_reference(env, parameters[0], DEFAULT_REF_COUNT, &(asyncContext->callbackRef)));
    }
    return NapiUtil::HandleAsyncWork(
        env, asyncContext.release(), "DisableCellularData", NativeDisableCellularData, DisableCellularDataCallback);
}

static void NativeEnableCellularDataRoaming(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (!IsValidSlotId(asyncContext->slotId)) {
        TELEPHONY_LOGE("NativeEnableCellularDataRoaming slotId is invalid");
        asyncContext->errorCode = ERROR_SLOT_ID_INVALID;
        return;
    }
    if (IsCellularDataManagerInited()) {
        asyncContext->result = CellularDataClient::GetInstance().EnableCellularDataRoaming(asyncContext->slotId, true);
        if (asyncContext->result == TELEPHONY_ERR_SUCCESS) {
            asyncContext->resolved = true;
        } else {
            asyncContext->resolved = false;
            asyncContext->errorCode = asyncContext->result;
        }
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
    TELEPHONY_LOGI("NativeEnableCellularDataRoaming end");
}

static napi_value EnableCellularDataRoaming(napi_env env, napi_callback_info info)
{
    const size_t paramLimitTwo = 2;
    size_t parameterCount = paramLimitTwo;
    napi_value parameters[paramLimitTwo] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data));
    if (!MatchEnableCellularDataRoamingParameters(env, parameters, parameterCount)) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_int32(env, parameters[0], &asyncContext->slotId));
    if (parameterCount == paramLimitTwo) {
        NAPI_CALL(env, napi_create_reference(env, parameters[1], DEFAULT_REF_COUNT, &asyncContext->callbackRef));
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "EnableCellularDataRoaming",
        NativeEnableCellularDataRoaming, EnableCellularDataRoamingCallback);
}

static void NativeDisableCellularDataRoaming(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (!IsValidSlotId(asyncContext->slotId)) {
        TELEPHONY_LOGE("NativeDisableCellularDataRoaming slotId is invalid");
        asyncContext->errorCode = ERROR_SLOT_ID_INVALID;
        return;
    }
    if (IsCellularDataManagerInited()) {
        asyncContext->result = CellularDataClient::GetInstance().EnableCellularDataRoaming(asyncContext->slotId, false);
        if (asyncContext->result == TELEPHONY_ERR_SUCCESS) {
            asyncContext->resolved = true;
        } else {
            asyncContext->resolved = false;
            asyncContext->errorCode = asyncContext->result;
        }
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

static napi_value DisableCellularDataRoaming(napi_env env, napi_callback_info info)
{
    const size_t paramLimitTwo = 2;
    size_t parameterCount = paramLimitTwo;
    napi_value parameters[paramLimitTwo] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data));
    if (!MatchEnableCellularDataRoamingParameters(env, parameters, parameterCount)) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    NAPI_CALL(env, napi_get_value_int32(env, parameters[0], &asyncContext->slotId));
    if (parameterCount == paramLimitTwo) {
        NAPI_CALL(env, napi_create_reference(env, parameters[1], DEFAULT_REF_COUNT, &asyncContext->callbackRef));
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "DisableCellularDataRoaming",
        NativeDisableCellularDataRoaming, DisableCellularDataRoamingCallback);
}

static void NativeIsCellularDataRoamingEnabled(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (!IsValidSlotId(asyncContext->slotId)) {
        TELEPHONY_LOGE("NativeIsCellularDataRoamingEnabled slotId is invalid");
        asyncContext->errorCode = ERROR_SLOT_ID_INVALID;
        return;
    }
    if (IsCellularDataManagerInited()) {
        auto &dataManager = CellularDataClient::GetInstance();
        bool enabled = false;
        asyncContext->errorCode = dataManager.IsCellularDataRoamingEnabled(asyncContext->slotId, enabled);
        if (asyncContext->errorCode == TELEPHONY_SUCCESS) {
            asyncContext->resolved = true;
        }
        asyncContext->result = enabled;
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

static void IsCellularDataRoamingEnabledCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        napi_status status = napi_get_boolean(env,
            asyncContext->result == static_cast<int32_t>(RoamingSwitchCode::CELLULAR_DATA_ROAMING_ENABLED),
            &callbackValue);
        NAPI_CALL_RETURN_VOID(env, status);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "isCellularDataRoamingEnabled", GET_NETWORK_INFO);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle2ValueCallback(env, asyncContext.release(), callbackValue);
}

static napi_value IsCellularDataRoamingEnabled(napi_env env, napi_callback_info info)
{
    const size_t paramLimitTwo = 2;
    size_t parameterCount = paramLimitTwo;
    napi_value parameters[paramLimitTwo] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (!MatchEnableCellularDataRoamingParameters(env, parameters, parameterCount)) {
        TELEPHONY_LOGE("IsCellularDataRoamingEnabled MatchEnableCellularDataRoamingParameters failed.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        TELEPHONY_LOGE("IsCellularDataRoamingEnabled asyncContext is nullptr.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    napi_get_value_int32(env, parameters[0], &asyncContext->slotId);
    if (parameterCount == paramLimitTwo) {
        napi_create_reference(env, parameters[1], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "IsCellularDataRoamingEnabled",
        NativeIsCellularDataRoamingEnabled, IsCellularDataRoamingEnabledCallback);
}

static napi_value IsCellularDataRoamingEnabledSync(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 0;
    napi_value parameters[] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    int32_t slotId;
    bool dataRoamingEnabled = false;
    napi_value value = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    napi_get_value_int32(env, parameters[0], &slotId);
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGE("IsCellularDataRoamingEnabledSync slotId is invalid");
        NapiUtil::ThrowParameterError(env);
        return value;
    }
    if (IsCellularDataManagerInited()) {
        auto &dataManager = CellularDataClient::GetInstance();
        auto errorCode = dataManager.IsCellularDataRoamingEnabled(slotId, dataRoamingEnabled);
        if (errorCode != TELEPHONY_SUCCESS) {
            JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
                errorCode, "isCellularDataRoamingEnabledSync", GET_NETWORK_INFO);
            NapiUtil::ThrowError(env, error.errorCode, error.errorMessage);
            return value;
        }
    } else {
        return value;
    }
    NAPI_CALL(env, napi_get_boolean(env, dataRoamingEnabled, &value));
    return value;
}

static bool MatchGetDefaultCellularDataSlotIdParameters(
    napi_env env, const napi_value parameters[], const size_t parameterCount)
{
    switch (parameterCount) {
        case 0:
            return true;
        case 1:
            return NapiUtil::MatchParameters(env, parameters, {napi_function});
        default:
            return false;
    }
}

static void NativeGetDefaultCellularDataSlotId(napi_env env, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        return;
    }
    int32_t result = CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    if (IsValidSlotIdEx(result) || result == DEFAULT_SIM_SLOT_ID_REMOVE) {
        context->slotId = result;
        context->resolved = true;
    } else {
        context->resolved = false;
        context->errorCode = ERROR_NATIVE_API_EXECUTE_FAIL;
    }
}

static void GetDefaultCellularDataSlotIdCallback(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        return;
    }
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncContext->slotId, &callbackValue));
    } else {
        if (asyncContext->errorCode == ERROR_NATIVE_API_EXECUTE_FAIL) {
            callbackValue = NapiUtil::CreateErrorMessage(
                env, "GetDefaultCellularDataSlotId api execute failed", ERROR_NATIVE_API_EXECUTE_FAIL);
        } else {
            callbackValue = NapiUtil::CreateErrorMessage(env, "get default cellular data slotId failed");
        }
    }
    NapiUtil::Handle2ValueCallback(env, asyncContext, callbackValue);
}

static napi_value GetDefaultCellularDataSlotId(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchGetDefaultCellularDataSlotIdParameters(env, parameters, parameterCount), "type mismatch");
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        std::string errorCode = std::to_string(napi_generic_failure);
        std::string errorMessage = "error at baseContext is nullptr";
        NAPI_CALL(env, napi_throw_error(env, errorCode.c_str(), errorMessage.c_str()));
        return nullptr;
    }
    if (parameterCount == 1) {
        napi_create_reference(env, parameters[0], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "GetDefaultCellularDataSlotId",
        NativeGetDefaultCellularDataSlotId, GetDefaultCellularDataSlotIdCallback);
}

static napi_value GetDefaultCellularDataSlotIdSync(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    int32_t slotId = -1;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (parameterCount == 0) {
        slotId = CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    }
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_int32(env, slotId, &value));
    return value;
}

static napi_value GetDefaultCellularDataSimId(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    int32_t simId = 0;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (parameterCount == 0) {
        CellularDataClient::GetInstance().GetDefaultCellularDataSimId(simId);
    }
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_int32(env, simId, &value));
    return value;
}

static void NativeSetDefaultCellularDataSlotId(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (!IsValidSlotId(asyncContext->slotId)) {
        TELEPHONY_LOGE("NativeSetDefaultCellularDataSlotId slotId is invalid");
        asyncContext->errorCode = ERROR_SLOT_ID_INVALID;
        return;
    }
    if (IsCellularDataManagerInited()) {
        asyncContext->errorCode = CellularDataClient::GetInstance().SetDefaultCellularDataSlotId(asyncContext->slotId);
        if (asyncContext->errorCode == TELEPHONY_ERR_SUCCESS) {
            asyncContext->resolved = true;
        } else {
            asyncContext->resolved = false;
        }
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

static void SetDefaultCellularDataSlotIdCallback(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        callbackValue = NapiUtil::CreateUndefined(env);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "setDefaultCellularDataSlotId", SET_TELEPHONY_STATE);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle1ValueCallback(env, asyncContext, callbackValue);
}

static napi_value SetDefaultCellularDataSlotId(napi_env env, napi_callback_info info)
{
    const size_t paramLimitTwo = 2;
    size_t parameterCount = paramLimitTwo;
    napi_value parameters[] = { nullptr, nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (!MatchEnableCellularDataRoamingParameters(env, parameters, parameterCount)) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    napi_get_value_int32(env, parameters[0], &asyncContext->slotId);
    if (parameterCount == paramLimitTwo) {
        napi_create_reference(env, parameters[1], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "SetDefaultCellularDataSlotId",
        NativeSetDefaultCellularDataSlotId, SetDefaultCellularDataSlotIdCallback);
}

void NativeGetCellularDataFlowType(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        return;
    }
    if (IsCellularDataManagerInited()) {
        int32_t result = CellularDataClient::GetInstance().GetCellularDataFlowType();
        TELEPHONY_LOGI("result = %{public}d", result);
        if (result == TELEPHONY_ERR_PERMISSION_ERR) {
            asyncContext->errorCode = result;
        } else {
            asyncContext->resolved = true;
            asyncContext->result = WrapGetCellularDataFlowTypeType(result);
        }
    } else {
        asyncContext->resolved = false;
        asyncContext->errorCode = ERROR_SERVICE_UNAVAILABLE;
    }
}

void GetCellularDataFlowTypeCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncContext *>(data);
    if (context == nullptr) {
        return;
    }
    std::unique_ptr<AsyncContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncContext->result, &callbackValue));
    } else {
        if (asyncContext->errorCode == ERROR_SERVICE_UNAVAILABLE) {
            callbackValue =
                NapiUtil::CreateErrorMessage(env, "cellular data service unavailable", ERROR_SERVICE_UNAVAILABLE);
        } else {
            JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "GetCellularDataFlowType", GET_NETWORK_INFO);
            callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
        }
    }
    NapiUtil::Handle2ValueCallback(env, asyncContext.release(), callbackValue);
    TELEPHONY_LOGI("GetCellularDataFlowTypeCallback end");
}

static napi_value GetCellularDataFlowType(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    NAPI_ASSERT(env, MatchGetDefaultCellularDataSlotIdParameters(env, parameters, parameterCount), "type mismatch");
    auto asyncContext = std::make_unique<AsyncContext>();
    if (asyncContext == nullptr) {
        std::string errorCode = std::to_string(napi_generic_failure);
        std::string errorMessage = "error at baseContext is nullptr";
        NAPI_CALL(env, napi_throw_error(env, errorCode.c_str(), errorMessage.c_str()));
        return nullptr;
    }
    if (parameterCount == 1) {
        napi_create_reference(env, parameters[0], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    }
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "GetCellularDataFlowType",
        NativeGetCellularDataFlowType, GetCellularDataFlowTypeCallback);
}

static napi_value CreateEnumConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    return thisArg;
}

static napi_value InitEnumDataConnectState(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_UNKNOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_UNKNOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_DISCONNECTED",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_DISCONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_CONNECTING",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTING))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_CONNECTED",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_SUSPENDED",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_SUSPENDED))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_value CreateDataConnectState(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_UNKNOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_UNKNOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_DISCONNECTED",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_DISCONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_CONNECTING",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTING))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_CONNECTED",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_CONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_STATE_SUSPENDED",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataConnectState::DATA_STATE_SUSPENDED))),

    };
    napi_value result = nullptr;
    napi_define_class(env, "DataConnectState", NAPI_AUTO_LENGTH, CreateEnumConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "DataConnectState", result);
    return exports;
}

static napi_value InitEnumDataFlowType(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_NONE",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DORMANT",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_value CreateDataFlowType(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_NONE",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DORMANT",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "DataFlowType", NAPI_AUTO_LENGTH, CreateEnumConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "DataFlowType", result);
    return exports;
}

static napi_value EnableIntelligenceSwitch(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[1];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    bool enable = false;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    int32_t result = -1;
    napi_get_value_bool(env, parameters[0], &enable);
    if (parameterCount == 1) {
        result = CellularDataClient::GetInstance().EnableIntelligenceSwitch(enable);
    }
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &value));
    return value;
}

static napi_value GetIntelligenceSwitchState(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 1;
    napi_value parameters[] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    bool switchState = false;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (parameterCount == 0) {
        CellularDataClient::GetInstance().GetIntelligenceSwitchState(switchState);
    }
    napi_value value = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, switchState, &value));
    return value;
}

template<typename T>
void NapiAsyncBaseCompleteCallback(
    napi_env env, const AsyncContext1<T> &asyncContext, JsError error, bool funcIgnoreReturnVal = false)
{
    const BaseContext &context = asyncContext.context;
    if (context.deferred != nullptr && !context.resolved) {
        napi_value errorMessage = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context.deferred, errorMessage));
        NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, context.work));
        TELEPHONY_LOGE("NapiAsyncBaseCompleteCallback deferred error and resolved is false");
        return;
    }

    if (context.deferred != nullptr && context.resolved) {
        napi_value res =
            (funcIgnoreReturnVal ? NapiUtil::CreateUndefined(env) : GetNapiValue(env, asyncContext.callbackVal));
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context.deferred, res));
        NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, context.work));
        TELEPHONY_LOGE("NapiAsyncBaseCompleteCallback deferred error and resolved is true");
        return;
    }

    napi_value res =
        (funcIgnoreReturnVal ? NapiUtil::CreateUndefined(env) : GetNapiValue(env, asyncContext.callbackVal));
    napi_value callbackValue[] { NapiUtil::CreateUndefined(env), res };
    if (!context.resolved) {
        callbackValue[0] = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
        callbackValue[1] = NapiUtil::CreateUndefined(env);
    }
    napi_value undefined = nullptr;
    napi_value callback = nullptr;
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, context.callbackRef, &callback));
    NAPI_CALL_RETURN_VOID(
        env, napi_call_function(env, undefined, callback, std::size(callbackValue), callbackValue, &result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, context.callbackRef));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, context.work));
}

template<typename T>
void NapiAsyncPermissionCompleteCallback(napi_env env, napi_status status, const AsyncContext1<T> &asyncContext,
    bool funcIgnoreReturnVal, PermissionPara permissionPara)
{
    if (status != napi_ok) {
        napi_throw_type_error(env, nullptr, "excute failed");
        return;
    }

    JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
        asyncContext.context.errorCode, permissionPara.func, permissionPara.permission);
    NapiAsyncBaseCompleteCallback(env, asyncContext, error, funcIgnoreReturnVal);
}

void ApnInfoAnalyzeExt(napi_env env, napi_value arg, ApnInfo &queryApnPara)
{
    napi_value proxy = NapiUtil::GetNamedProperty(env, arg, "proxy");
    if (proxy) {
        std::array<char, ARRAY_SIZE> proxyStr = {0};
        NapiValueToCppValue(env, proxy, napi_string, std::data(proxyStr));
        queryApnPara.proxy = NapiUtil::ToUtf16(proxyStr.data());
    } else {
        queryApnPara.proxy = NapiUtil::ToUtf16(NOT_FILLED_IN);
    }
    napi_value mmsproxy = NapiUtil::GetNamedProperty(env, arg, "mmsproxy");
    if (mmsproxy) {
        std::array<char, ARRAY_SIZE> mmsproxyStr = {0};
        NapiValueToCppValue(env, mmsproxy, napi_string, std::data(mmsproxyStr));
        queryApnPara.mmsproxy = NapiUtil::ToUtf16(mmsproxyStr.data());
    } else {
        queryApnPara.mmsproxy = NapiUtil::ToUtf16(NOT_FILLED_IN);
    }
}

void ApnInfoAnalyze(napi_env env, napi_value arg, ApnInfo &queryApnPara)
{
    napi_value apnName = NapiUtil::GetNamedProperty(env, arg, "apnName");
    if (apnName) {
        std::array<char, ARRAY_SIZE> apnNameStr = {0};
        NapiValueToCppValue(env, apnName, napi_string, std::data(apnNameStr));
        queryApnPara.apnName = NapiUtil::ToUtf16(apnNameStr.data());
    }
    napi_value apn = NapiUtil::GetNamedProperty(env, arg, "apn");
    if (apn) {
        std::array<char, ARRAY_SIZE> apnStr = {0};
        NapiValueToCppValue(env, apn, napi_string, std::data(apnStr));
        queryApnPara.apn = NapiUtil::ToUtf16(apnStr.data());
    }
    napi_value mcc = NapiUtil::GetNamedProperty(env, arg, "mcc");
    if (mcc) {
        std::array<char, ARRAY_SIZE> mccStr = {0};
        NapiValueToCppValue(env, mcc, napi_string, std::data(mccStr));
        queryApnPara.mcc = NapiUtil::ToUtf16(mccStr.data());
    }
    napi_value mnc = NapiUtil::GetNamedProperty(env, arg, "mnc");
    if (mnc) {
        std::array<char, ARRAY_SIZE> mncStr = {0};
        NapiValueToCppValue(env, mnc, napi_string, std::data(mncStr));
        queryApnPara.mnc = NapiUtil::ToUtf16(mncStr.data());
    }
    napi_value user = NapiUtil::GetNamedProperty(env, arg, "user");
    if (user) {
        std::array<char, ARRAY_SIZE> userStr = {0};
        NapiValueToCppValue(env, user, napi_string, std::data(userStr));
        queryApnPara.user = NapiUtil::ToUtf16(userStr.data());
    } else {
        queryApnPara.user = NapiUtil::ToUtf16(NOT_FILLED_IN);
    }
    napi_value type = NapiUtil::GetNamedProperty(env, arg, "type");
    if (type) {
        std::array<char, ARRAY_SIZE> typeStr = {0};
        NapiValueToCppValue(env, type, napi_string, std::data(typeStr));
        queryApnPara.type = NapiUtil::ToUtf16(typeStr.data());
    } else {
        queryApnPara.type = NapiUtil::ToUtf16(NOT_FILLED_IN);
    }
    ApnInfoAnalyzeExt(env, arg, queryApnPara);
}

template<typename AsyncContextType, typename... Ts>
napi_value NapiCreateAsyncWork2(const AsyncPara &para, AsyncContextType *asyncContext, std::tuple<Ts...> &theTuple)
{
    if (asyncContext == nullptr) {
        return nullptr;
    }

    napi_env env = para.env;
    BaseContext &context = asyncContext->asyncContext.context;

    size_t argc = sizeof...(Ts);
    napi_value argv[sizeof...(Ts)]{nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, para.info, &argc, argv, nullptr, nullptr));

    std::optional<NapiError> errCode = MatchParameters(env, argv, argc, theTuple);
    if (errCode.has_value()) {
        JsError error = NapiUtil::ConverErrorMessageForJs(errCode.value());
        NapiUtil::ThrowError(env, error.errorCode, error.errorMessage);
        delete asyncContext;
        asyncContext = nullptr;
        return nullptr;
    }

    napi_value result = nullptr;
    if (context.callbackRef == nullptr) {
        NAPI_CALL(env, napi_create_promise(env, &context.deferred, &result));
    } else {
        NAPI_CALL(env, napi_get_undefined(env, &result));
    }

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, para.funcName.c_str(), para.funcName.length(), &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, para.execute, para.complete,
        static_cast<void *>(asyncContext), &context.work));
    return result;
}

 void NativeQueryApnIds(napi_env env, void *data)
{
    if (data == nullptr) {
        return;
    }
    auto queryApnInfoContext = static_cast<AsyncQueryApnInfo *>(data);
    ApnInfo apnInfoStru;
    if (queryApnInfoContext->queryApnPara.apn.length() == 0) {
        TELEPHONY_LOGE("NativeQueryApnIds apn is null.");
    }
    apnInfoStru.apnName = queryApnInfoContext->queryApnPara.apnName;
    apnInfoStru.apn = queryApnInfoContext->queryApnPara.apn;
    apnInfoStru.mcc = queryApnInfoContext->queryApnPara.mcc;
    apnInfoStru.mnc = queryApnInfoContext->queryApnPara.mnc;
    apnInfoStru.user = queryApnInfoContext->queryApnPara.user;
    apnInfoStru.type = queryApnInfoContext->queryApnPara.type;
    apnInfoStru.proxy = queryApnInfoContext->queryApnPara.proxy;
    apnInfoStru.mmsproxy = queryApnInfoContext->queryApnPara.mmsproxy;
    std::vector<uint32_t> apnIdList;
    std::unique_lock<std::mutex> callbackLock(queryApnInfoContext->asyncContext.callbackMutex);
    int32_t errorCode = CellularDataClient::GetInstance().QueryApnIds(apnInfoStru, apnIdList);
    if (errorCode == TELEPHONY_SUCCESS) {
        queryApnInfoContext->apnIdList.swap(apnIdList);
        queryApnInfoContext->asyncContext.context.resolved = true;
    } else {
        TELEPHONY_LOGE("NAPI NativeQueryApnIds %{public}d", errorCode);
        queryApnInfoContext->asyncContext.context.resolved = false;
    }
    queryApnInfoContext->asyncContext.context.errorCode = errorCode;
}

void QueryApnIdsCallback(napi_env env, napi_status status, void *data)
{
    NAPI_CALL_RETURN_VOID(env, (data == nullptr ? napi_invalid_arg : napi_ok));
    std::unique_ptr<AsyncQueryApnInfo> info(static_cast<AsyncQueryApnInfo *>(data));
    AsyncContext1<napi_value> &asyncContext = info->asyncContext;
    asyncContext.callbackVal = nullptr;
    napi_create_array(env, &asyncContext.callbackVal);
    TELEPHONY_LOGI("QueryApnCallback info->apnIdList.size = %{public}zu", info->apnIdList.size());
    for (size_t i = 0; i < info->apnIdList.size(); i++) {
        napi_value val;
        napi_create_int32(env, info->apnIdList.at(i), &val);
        TELEPHONY_LOGI("QueryApnCallback info->apnIdList.size = %{public}d", info->apnIdList.at(i));
        napi_set_element(env, asyncContext.callbackVal, i, val);
    }
    NapiAsyncPermissionCompleteCallback(
        env, status, asyncContext, false, { "QueryApnIds", MANAGE_APN_SETTING });
}

static napi_value QueryApnIds(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGI("QueryApnIds enter!");
    auto queryApnInfo = std::make_unique<AsyncQueryApnInfo>();
    if (queryApnInfo == nullptr) {
        return nullptr;
    }
    BaseContext &context = queryApnInfo->asyncContext.context;

    napi_value object = NapiUtil::CreateUndefined(env);
    auto initPara = std::make_tuple(&object, &context.callbackRef);
    AsyncPara para {
        .funcName = "QueryApnIds",
        .env = env,
        .info = info,
        .execute = NativeQueryApnIds,
        .complete = QueryApnIdsCallback,
    };
    napi_value result = NapiCreateAsyncWork2<AsyncQueryApnInfo>(para, queryApnInfo.get(), initPara);
    if (result == nullptr) {
        TELEPHONY_LOGE("creat asyncwork failed!");
        return nullptr;
    }
    ApnInfoAnalyze(env, object, queryApnInfo->queryApnPara);
    if (napi_queue_async_work_with_qos(env, context.work, napi_qos_default) == napi_ok) {
        queryApnInfo.release();
    }
    return result;
}

static void NativeSetPreferApn(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncSetPreferApnContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    int32_t errcode = CellularDataClient::GetInstance().SetPreferApn(asyncContext->apnId);
    TELEPHONY_LOGI("NAPI NativeSetPreferApn apnId:%{public}d, error::%{public}d", asyncContext->apnId, errcode);
    asyncContext->errorCode = errcode;
    if (asyncContext->errorCode == TELEPHONY_SUCCESS) {
        asyncContext->resolved = true;
        asyncContext->result = true;
    } else if (asyncContext->errorCode == -1) {
        asyncContext->resolved = true;
        asyncContext->result = false;
    } else {
        asyncContext->resolved = false;
        asyncContext->result = false;
    }
}

static void SetPreferApnCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<AsyncSetPreferApnContext *>(data);
    if (context == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }

    std::unique_ptr<AsyncSetPreferApnContext> asyncContext(context);
    napi_value callbackValue = nullptr;
    if (asyncContext->resolved) {
        napi_status status = napi_get_boolean(env, asyncContext->result, &callbackValue);
        NAPI_CALL_RETURN_VOID(env, status);
    } else {
        JsError error = NapiUtil::ConverErrorMessageWithPermissionForJs(
            asyncContext->errorCode, "SetPreferredApn", MANAGE_APN_SETTING);
        callbackValue = NapiUtil::CreateErrorMessage(env, error.errorMessage, error.errorCode);
    }
    NapiUtil::Handle2ValueCallback(env, asyncContext.release(), callbackValue);
}

static napi_value SetPreferredApn(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGI("SetPreferredApn enter!");
    const size_t paramLimitTwo = 2;
    size_t parameterCount = paramLimitTwo;
    napi_value parameters[] = { nullptr, nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);

    if (!NapiUtil::MatchParameters(env, parameters, {napi_number})) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto asyncContext = std::make_unique<AsyncSetPreferApnContext>();
    if (asyncContext == nullptr) {
        TELEPHONY_LOGE("SetPreferredApn asyncContext is nullptr.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    napi_get_value_int32(env, parameters[0], &asyncContext->apnId);
    return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "SetPreferredApn",
        NativeSetPreferApn, SetPreferApnCallback);
}

napi_value ApnInfoConversion(napi_env env, const ApnInfo &asyncQueryApnPara)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    SetPropertyToNapiObject(env, val, "apnName", NapiUtil::ToUtf8(asyncQueryApnPara.apnName));
    SetPropertyToNapiObject(env, val, "apn", NapiUtil::ToUtf8(asyncQueryApnPara.apn));
    SetPropertyToNapiObject(env, val, "mcc", NapiUtil::ToUtf8(asyncQueryApnPara.mcc));
    SetPropertyToNapiObject(env, val, "mnc", NapiUtil::ToUtf8(asyncQueryApnPara.mnc));
    SetPropertyToNapiObject(env, val, "user", NapiUtil::ToUtf8(asyncQueryApnPara.user));
    SetPropertyToNapiObject(env, val, "type", NapiUtil::ToUtf8(asyncQueryApnPara.type));
    SetPropertyToNapiObject(env, val, "proxy", NapiUtil::ToUtf8(asyncQueryApnPara.proxy));
    SetPropertyToNapiObject(env, val, "mmsproxy", NapiUtil::ToUtf8(asyncQueryApnPara.mmsproxy));
    return val;
}

void NativeQueryAllApn(napi_env env, void *data)
{
    if (data == nullptr) {
        return;
    }
    auto queryAllApnInfoContext = static_cast<AsyncQueryAllApnInfo *>(data);

    std::vector<ApnInfo> allApnInfoList;
    std::unique_lock<std::mutex> callbackLock(queryAllApnInfoContext->asyncContext.callbackMutex);
    int32_t errorCode = CellularDataClient::GetInstance().QueryAllApnInfo(allApnInfoList);
    TELEPHONY_LOGI("NAPI NativeQueryAllApn %{public}d", errorCode);
    if (errorCode == TELEPHONY_SUCCESS) {
        queryAllApnInfoContext->allApnInfoList.swap(allApnInfoList);
        queryAllApnInfoContext->asyncContext.context.resolved = true;
    } else {
        queryAllApnInfoContext->asyncContext.context.resolved = false;
    }
    queryAllApnInfoContext->asyncContext.context.errorCode = errorCode;
}

void QueryAllApnCallback(napi_env env, napi_status status, void *data)
{
    NAPI_CALL_RETURN_VOID(env, (data == nullptr ? napi_invalid_arg : napi_ok));
    std::unique_ptr<AsyncQueryAllApnInfo> info(static_cast<AsyncQueryAllApnInfo *>(data));
    AsyncContext1<napi_value> &asyncContext = info->asyncContext;
    asyncContext.callbackVal = nullptr;
    napi_create_array(env, &asyncContext.callbackVal);
    TELEPHONY_LOGI("QueryAllApnCallback info->allApnInfoList.size = %{public}zu", info->allApnInfoList.size());
    for (size_t i = 0; i < info->allApnInfoList.size(); i++) {
        napi_value val = ApnInfoConversion(env, info->allApnInfoList.at(i));
        napi_set_element(env, asyncContext.callbackVal, i, val);
    }
    NapiAsyncPermissionCompleteCallback(
        env, status, asyncContext, false, { "QueryAllApns", MANAGE_APN_SETTING });
}

static napi_value QueryAllApns(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGI("QueryAllApns enter!");
    size_t parameterCount = 1;
    napi_value parameters[] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);

    if (parameterCount != 0) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }

    auto queryAllApnInfo = std::make_unique<AsyncQueryAllApnInfo>();
    if (queryAllApnInfo == nullptr) {
        return nullptr;
    }
    BaseContext &context = queryAllApnInfo->asyncContext.context;

    auto initPara = std::make_tuple(&context.callbackRef);
    AsyncPara para {
        .funcName = "QueryAllApns",
        .env = env,
        .info = info,
        .execute = NativeQueryAllApn,
        .complete = QueryAllApnCallback,
    };
    napi_value result = NapiCreateAsyncWork2<AsyncQueryAllApnInfo>(para, queryAllApnInfo.get(), initPara);
    if (result == nullptr) {
        TELEPHONY_LOGE("creat asyncwork failed!");
        return nullptr;
    }
    if (napi_queue_async_work_with_qos(env, context.work, napi_qos_default) == napi_ok) {
        queryAllApnInfo.release();
    }
    return result;
}

void NativeGetActiveApnName(napi_env env, void *data)
{
    if (data == nullptr) {
        return;
    }
    auto getActiveApnNameContext = static_cast<AsyncGetActiveApnName *>(data);

    std::string apnName;
    std::unique_lock<std::mutex> callbackLock(getActiveApnNameContext->asyncContext.callbackMutex);
    int32_t errorCode = CellularDataClient::GetInstance().GetActiveApnName(apnName);
    TELEPHONY_LOGI("NAPI NativeGetActiveApnName %{public}d", errorCode);
    if (errorCode == TELEPHONY_SUCCESS) {
        getActiveApnNameContext->apnName = apnName;
        getActiveApnNameContext->asyncContext.context.resolved = true;
    } else {
        getActiveApnNameContext->asyncContext.context.resolved = false;
    }
    getActiveApnNameContext->asyncContext.context.errorCode = errorCode;
}

void GetActiveApnNameCallback(napi_env env, napi_status status, void *data)
{
    NAPI_CALL_RETURN_VOID(env, (data == nullptr ? napi_invalid_arg : napi_ok));
    std::unique_ptr<AsyncGetActiveApnName> info(static_cast<AsyncGetActiveApnName *>(data));
    AsyncContext1<napi_value> &asyncContext = info->asyncContext;
    asyncContext.callbackVal = nullptr;

    std::string apnName = info->apnName;
    napi_create_string_utf8(env, apnName.c_str(), apnName.length(), &asyncContext.callbackVal);
    TELEPHONY_LOGI("GetActiveApnNameCallback apnName = %{public}s", apnName.c_str());
    NapiAsyncPermissionCompleteCallback(
        env, status, asyncContext, false, { "GetActiveApnName", GET_NETWORK_INFO });
}

static napi_value GetActiveApnName(napi_env env, napi_callback_info info)
{
    TELEPHONY_LOGI("GetActiveApnName enter!");
    size_t parameterCount = 1;
    napi_value parameters[] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);

    if (parameterCount != 0) {
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }

    auto asyncGetActiveApnName = std::make_unique<AsyncGetActiveApnName>();
    if (asyncGetActiveApnName == nullptr) {
        return nullptr;
    }
    BaseContext &context = asyncGetActiveApnName->asyncContext.context;

    auto initPara = std::make_tuple(&context.callbackRef);
    AsyncPara para {
        .funcName = "GetActiveApnName",
        .env = env,
        .info = info,
        .execute = NativeGetActiveApnName,
        .complete = GetActiveApnNameCallback,
    };
    napi_value result = NapiCreateAsyncWork2<AsyncGetActiveApnName>(para, asyncGetActiveApnName.get(), initPara);
    if (result == nullptr) {
        TELEPHONY_LOGE("creat asyncwork failed!");
        return nullptr;
    }
    if (napi_queue_async_work_with_qos(env, context.work, napi_qos_default) == napi_ok) {
        asyncGetActiveApnName.release();
    }
    return result;
}

EXTERN_C_START
napi_value RegistCellularData(napi_env env, napi_value exports)
{
    auto cellularDataPoxy = CellularDataClient::GetInstance().GetProxy();
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_WRITABLE_FUNCTION("getCellularDataState", GetCellularDataState),
        DECLARE_NAPI_WRITABLE_FUNCTION("isCellularDataEnabled", IsCellularDataEnabled),
        DECLARE_NAPI_WRITABLE_FUNCTION("isCellularDataEnabledSync", IsCellularDataEnabledSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("enableCellularData", EnableCellularData),
        DECLARE_NAPI_WRITABLE_FUNCTION("disableCellularData", DisableCellularData),
        DECLARE_NAPI_WRITABLE_FUNCTION("enableCellularDataRoaming", EnableCellularDataRoaming),
        DECLARE_NAPI_WRITABLE_FUNCTION("disableCellularDataRoaming", DisableCellularDataRoaming),
        DECLARE_NAPI_WRITABLE_FUNCTION("isCellularDataRoamingEnabled", IsCellularDataRoamingEnabled),
        DECLARE_NAPI_WRITABLE_FUNCTION("isCellularDataRoamingEnabledSync", IsCellularDataRoamingEnabledSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("getDefaultCellularDataSlotId", GetDefaultCellularDataSlotId),
        DECLARE_NAPI_WRITABLE_FUNCTION("getDefaultCellularDataSimId", GetDefaultCellularDataSimId),
        DECLARE_NAPI_WRITABLE_FUNCTION("getDefaultCellularDataSlotIdSync", GetDefaultCellularDataSlotIdSync),
        DECLARE_NAPI_WRITABLE_FUNCTION("setDefaultCellularDataSlotId", SetDefaultCellularDataSlotId),
        DECLARE_NAPI_WRITABLE_FUNCTION("getCellularDataFlowType", GetCellularDataFlowType),
        DECLARE_NAPI_WRITABLE_FUNCTION("enableIntelligenceSwitch", EnableIntelligenceSwitch),
        DECLARE_NAPI_WRITABLE_FUNCTION("getIntelligenceSwitchState", GetIntelligenceSwitchState),
        DECLARE_NAPI_WRITABLE_FUNCTION("queryApnIds", QueryApnIds),
        DECLARE_NAPI_WRITABLE_FUNCTION("setPreferredApn", SetPreferredApn),
        DECLARE_NAPI_WRITABLE_FUNCTION("queryAllApns", QueryAllApns),
        DECLARE_NAPI_WRITABLE_FUNCTION("getActiveApnName", GetActiveApnName),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    CreateDataConnectState(env, exports);
    InitEnumDataConnectState(env, exports);
    CreateDataFlowType(env, exports);
    InitEnumDataFlowType(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module _cellularDataModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RegistCellularData,
    .nm_modname = "telephony.data",
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void RegisterCellularDataModule(void)
{
    napi_module_register(&_cellularDataModule);
}
} // namespace Telephony
} // namespace OHOS
