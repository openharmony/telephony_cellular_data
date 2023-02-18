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
#include "napi_cellular_data_types.h"
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

static bool IsCellularDataManagerInited()
{
    return CellularDataClient::GetInstance().IsConnect();
}

static inline bool IsValidSlotId(int32_t slotId)
{
    return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT));
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
            return static_cast<int32_t>(DataConnectionState::DATA_STATE_DISCONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTING): {
            return static_cast<int32_t>(DataConnectionState::DATA_STATE_CONNECTING);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_CONNECTED): {
            return static_cast<int32_t>(DataConnectionState::DATA_STATE_CONNECTED);
        }
        case static_cast<int32_t>(DataConnectionStatus::DATA_STATE_SUSPENDED): {
            return static_cast<int32_t>(DataConnectionState::DATA_STATE_SUSPENDED);
        }
        default: {
            return static_cast<int32_t>(DataConnectionState::DATA_STATE_UNKNOWN);
        }
    }
}

static int32_t WrapGetCellularDataFlowTypeType(const int32_t cellularDataType)
{
    switch (cellularDataType) {
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_NONE): {
            return static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_NONE);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DOWN): {
            return static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_DOWN);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP): {
            return static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_UP);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN): {
            return static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_UP_DOWN);
        }
        case static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT): {
            return static_cast<int32_t>(CellDataFlowType::DATA_FLOW_TYPE_DORMANT);
        }
        default: {
            return static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_NONE);
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
        int32_t dataState = CellularDataClient::GetInstance().GetCellularDataState();
        asyncContext->resolved = true;
        asyncContext->result = WrapCellularDataType(dataState);
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
            callbackValue = NapiUtil::CreateErrorMessage(env, "unkonwn error");
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
    if (IsValidSlotId(result) || result == DEFAULT_SIM_SLOT_ID_REMOVE) {
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
    napi_value parameters[] = {nullptr};
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

static void NativeSetDefaultCellularDataSlotId(napi_env env, void *data)
{
    auto asyncContext = static_cast<AsyncContext *>(data);
    if (asyncContext == nullptr) {
        NapiUtil::ThrowParameterError(env);
        return;
    }
    if (!IsValidSlotId(asyncContext->slotId) && (asyncContext->slotId != DEFAULT_SIM_SLOT_ID_REMOVE)) {
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
        int32_t dataState = CellularDataClient::GetInstance().GetCellularDataFlowType();
        TELEPHONY_LOGI("dataState = %{public}d", dataState);
        asyncContext->resolved = true;
        asyncContext->result = WrapGetCellularDataFlowTypeType(dataState);
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
            callbackValue = NapiUtil::CreateErrorMessage(env, "unkonwn error");
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
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_UNKNOWN", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_UNKNOWN))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_DISCONNECTED", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_DISCONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_CONNECTING", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_CONNECTING))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_CONNECTED", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_CONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_SUSPENDED", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_SUSPENDED))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_value CreateDataConnectState(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_UNKNOWN", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_UNKNOWN))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_DISCONNECTED", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_DISCONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_CONNECTING", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_CONNECTING))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_CONNECTED", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_CONNECTED))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "DATA_STATE_SUSPENDED", NapiUtil::ToInt32Value(env, static_cast<int32_t>(DATA_STATE_SUSPENDED))),

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
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_UP))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_UP_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DORMANT",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_DORMANT))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_value CreateDataFlowType(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_NONE",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_UP))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_UP_DOWN",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_UP_DOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("DATA_FLOW_TYPE_DORMANT",
            NapiUtil::ToInt32Value(env, static_cast<int32_t>(DataFlowType::DATA_FLOW_TYPE_DORMANT))),
    };
    napi_value result = nullptr;
    napi_define_class(env, "DataFlowType", NAPI_AUTO_LENGTH, CreateEnumConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "DataFlowType", result);
    return exports;
}

EXTERN_C_START
napi_value RegistCellularData(napi_env env, napi_value exports)
{
    auto cellularDataPoxy = CellularDataClient::GetInstance().GetProxy();
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getCellularDataState", GetCellularDataState),
        DECLARE_NAPI_FUNCTION("isCellularDataEnabled", IsCellularDataEnabled),
        DECLARE_NAPI_FUNCTION("enableCellularData", EnableCellularData),
        DECLARE_NAPI_FUNCTION("disableCellularData", DisableCellularData),
        DECLARE_NAPI_FUNCTION("enableCellularDataRoaming", EnableCellularDataRoaming),
        DECLARE_NAPI_FUNCTION("disableCellularDataRoaming", DisableCellularDataRoaming),
        DECLARE_NAPI_FUNCTION("isCellularDataRoamingEnabled", IsCellularDataRoamingEnabled),
        DECLARE_NAPI_FUNCTION("getDefaultCellularDataSlotId", GetDefaultCellularDataSlotId),
        DECLARE_NAPI_FUNCTION("getDefaultCellularDataSlotIdSync", GetDefaultCellularDataSlotIdSync),
        DECLARE_NAPI_FUNCTION("setDefaultCellularDataSlotId", SetDefaultCellularDataSlotId),
        DECLARE_NAPI_FUNCTION("getCellularDataFlowType", GetCellularDataFlowType),
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
