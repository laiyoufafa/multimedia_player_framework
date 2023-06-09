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

#include "media_capability_vcaps_napi.h"
#include <climits>
#include "media_log.h"
#include "media_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MediaCapsVideoCapsNapi"};
}

namespace OHOS {
namespace Media {
thread_local napi_ref MediaCapabilityVCapsNapi::constructor_ = nullptr;
const std::string CLASS_NAME = "VideoCaps";

MediaCapabilityVCapsNapi::MediaCapabilityVCapsNapi()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MediaCapabilityVCapsNapi::~MediaCapabilityVCapsNapi()
{
    if (wrap_ != nullptr) {
        napi_delete_reference(env_, wrap_);
    }
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

napi_value MediaCapabilityVCapsNapi::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("isSizeSupported", IsSizeSupported),
        DECLARE_NAPI_FUNCTION("getSupportedFrameRate", GetSupportedFrameRate),
        DECLARE_NAPI_FUNCTION("getPreferredFrameRate", GetPreferredFrameRate),

        DECLARE_NAPI_GETTER("codecInfo", GetCodecInfo),
        DECLARE_NAPI_GETTER("supportedBitrate", SupportedBitrate),
        DECLARE_NAPI_GETTER("supportedFormats", SupportedFormats),
        DECLARE_NAPI_GETTER("supportedHeightAlignment", SupportedHeightAlignment),
        DECLARE_NAPI_GETTER("supportedWidthAlignment", SupportedWidthAlignment),
        DECLARE_NAPI_GETTER("supportedWidth", SupportedWidth),
        DECLARE_NAPI_GETTER("supportedHeight", SupportedHeight),
        DECLARE_NAPI_GETTER("supportedProfiles", SupportedProfiles),
        DECLARE_NAPI_GETTER("supportedLevels", SupportedLevels),
        DECLARE_NAPI_GETTER("supportedBitrateMode", SupportedBitrateMode),
        DECLARE_NAPI_GETTER("supportedQuality", SupportedQuality),
        DECLARE_NAPI_GETTER("supportedComplexity", SupportedComplexity),
    };

    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "Failed to define VideoCaps class");

    status = napi_create_reference(env, constructor, 1, &constructor_);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "Failed to create reference of constructor");

    status = napi_set_named_property(env, exports, CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "Failed to set constructor");

    MEDIA_LOGD("Init success");
    return exports;
}

napi_value MediaCapabilityVCapsNapi::Create(napi_env env, std::shared_ptr<VideoCaps> caps)
{
    napi_value constructor = nullptr;
    napi_status status = napi_get_reference_value(env, constructor_, &constructor);
    CHECK_AND_RETURN_RET(status == napi_ok && constructor != nullptr, nullptr);
    CapsWrap *capsWrap = new(std::nothrow) CapsWrap(caps);

    napi_value args[1] = {nullptr};
    status = napi_create_external_arraybuffer(env, capsWrap, sizeof(CapsWrap),
        [] (napi_env env, void *data, void *hint) {
            (void)env;
            (void)data;
            CapsWrap *capsWrap = reinterpret_cast<CapsWrap *>(hint);
            delete capsWrap;
        }, reinterpret_cast<void*>(capsWrap), &args[0]);
    if (status != napi_ok) {
        delete capsWrap;
        MEDIA_LOGE("create capsWrap val failed");
        return nullptr;
    }
    napi_value result = nullptr;
    CHECK_AND_RETURN_RET(napi_new_instance(env, constructor, 1, args, &result) == napi_ok, nullptr);
    return result;
}

napi_value MediaCapabilityVCapsNapi::Constructor(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_value jsThis = nullptr;
    napi_value args[1] = {nullptr};
    size_t argCount = 1;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && args[0] != nullptr, result);

    MediaCapabilityVCapsNapi *napi = new(std::nothrow) MediaCapabilityVCapsNapi();
    CHECK_AND_RETURN_RET(napi != nullptr, result);

    napi->env_ = env;
    void *data;
    size_t byte_length;
    (void)napi_get_arraybuffer_info(env, args[0], &data, &byte_length);
    CapsWrap *capsWrap = reinterpret_cast<CapsWrap*>(data);
    napi->caps_ = capsWrap->caps_;

    status = napi_wrap(env, jsThis, reinterpret_cast<void *>(napi),
        MediaCapabilityVCapsNapi::Destructor, nullptr, &(napi->wrap_));
    if (status != napi_ok) {
        delete napi;
        MEDIA_LOGE("Failed to wrap native instance");
        return result;
    }

    MEDIA_LOGD("Constructor success");
    return jsThis;
}

void MediaCapabilityVCapsNapi::Destructor(napi_env env, void *nativeObject, void *finalize)
{
    (void)env;
    (void)finalize;
    if (nativeObject != nullptr) {
        delete reinterpret_cast<MediaCapabilityVCapsNapi *>(nativeObject);
    }
    MEDIA_LOGD("Destructor success");
}

napi_value MediaCapabilityVCapsNapi::IsSizeSupported(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    auto asyncCtx = std::make_unique<MediaVideoCapsAsyncCtx>(env);

    napi_value jsThis = nullptr;
    napi_value args[3] = {nullptr};
    size_t argCount = 3;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        asyncCtx->SignError(MSERR_EXT_INVALID_VAL, "Failed to napi_get_cb_info");
    }

    napi_valuetype valueType = napi_undefined;
    if (args[0] != nullptr && napi_typeof(env, args[0], &valueType) == napi_ok && valueType == napi_number &&
        args[1] != nullptr && napi_typeof(env, args[1], &valueType) == napi_ok && valueType == napi_number) {
        (void)napi_get_value_int32(env, args[0], &asyncCtx->width_);
        (void)napi_get_value_int32(env, args[1], &asyncCtx->height_);
    } else {
        asyncCtx->SignError(MSERR_EXT_INVALID_VAL, "Illegal argument");
    }

    size_t argIndex = 2;
    asyncCtx->callbackRef = CommonNapi::CreateReference(env, args[argIndex]);
    asyncCtx->deferred = CommonNapi::CreatePromise(env, asyncCtx->callbackRef, result);

    (void)napi_unwrap(env, jsThis, reinterpret_cast<void **>(&asyncCtx->napi_));

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "IsSizeSupported", NAPI_AUTO_LENGTH, &resource);
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
            auto asyncCtx = reinterpret_cast<MediaVideoCapsAsyncCtx *>(data);
            if (asyncCtx == nullptr) {
                MEDIA_LOGE("Failed, asyncCtx is nullptr");
                return;
            } else if (asyncCtx->napi_ == nullptr || asyncCtx->napi_->caps_ == nullptr) {
                asyncCtx->SignError(MSERR_EXT_UNKNOWN, "nullptr");
                return;
            }
            bool supported = asyncCtx->napi_->caps_->IsSizeSupported(asyncCtx->width_, asyncCtx->height_);
            asyncCtx->JsResult = std::make_unique<MediaJsResultInt>(static_cast<int32_t>(supported));
        },
        MediaAsyncContext::CompleteCallback, static_cast<void *>(asyncCtx.get()), &asyncCtx->work));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCtx->work));
    asyncCtx.release();

    return result;
}

napi_value MediaCapabilityVCapsNapi::GetSupportedFrameRate(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    auto asyncCtx = std::make_unique<MediaVideoCapsAsyncCtx>(env);

    napi_value jsThis = nullptr;
    napi_value args[3] = {nullptr};
    size_t argCount = 3;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        asyncCtx->SignError(MSERR_EXT_INVALID_VAL, "Failed to napi_get_cb_info");
    }

    napi_valuetype valueType = napi_undefined;
    if (args[0] != nullptr && napi_typeof(env, args[0], &valueType) == napi_ok && valueType == napi_number &&
        args[1] != nullptr && napi_typeof(env, args[1], &valueType) == napi_ok && valueType == napi_number) {
        (void)napi_get_value_int32(env, args[0], &asyncCtx->width_);
        (void)napi_get_value_int32(env, args[1], &asyncCtx->height_);
    } else {
        asyncCtx->SignError(MSERR_EXT_INVALID_VAL, "Illegal argument");
    }

    size_t argIndex = 2;
    asyncCtx->callbackRef = CommonNapi::CreateReference(env, args[argIndex]);
    asyncCtx->deferred = CommonNapi::CreatePromise(env, asyncCtx->callbackRef, result);

    (void)napi_unwrap(env, jsThis, reinterpret_cast<void **>(&asyncCtx->napi_));

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "GetSupportedFrameRate", NAPI_AUTO_LENGTH, &resource);
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
            auto asyncCtx = reinterpret_cast<MediaVideoCapsAsyncCtx *>(data);
            if (asyncCtx == nullptr) {
                MEDIA_LOGE("Failed, asyncCtx is nullptr");
                return;
            } else if (asyncCtx->napi_ == nullptr || asyncCtx->napi_->caps_ == nullptr) {
                asyncCtx->SignError(MSERR_EXT_UNKNOWN, "nullptr");
                return;
            }
            Range range = asyncCtx->napi_->caps_->GetSupportedFrameRatesFor(asyncCtx->width_, asyncCtx->height_);
            asyncCtx->JsResult = std::make_unique<MediaJsResultRange>(range.minVal, range.maxVal);
        },
        MediaAsyncContext::CompleteCallback, static_cast<void *>(asyncCtx.get()), &asyncCtx->work));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCtx->work));
    asyncCtx.release();

    return result;
}

napi_value MediaCapabilityVCapsNapi::GetPreferredFrameRate(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    auto asyncCtx = std::make_unique<MediaVideoCapsAsyncCtx>(env);

    napi_value jsThis = nullptr;
    napi_value args[3] = {nullptr};
    size_t argCount = 3;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        asyncCtx->SignError(MSERR_EXT_INVALID_VAL, "Failed to napi_get_cb_info");
    }

    napi_valuetype valueType = napi_undefined;
    if (args[0] != nullptr && napi_typeof(env, args[0], &valueType) == napi_ok && valueType == napi_number &&
        args[1] != nullptr && napi_typeof(env, args[1], &valueType) == napi_ok && valueType == napi_number) {
        (void)napi_get_value_int32(env, args[0], &asyncCtx->width_);
        (void)napi_get_value_int32(env, args[1], &asyncCtx->height_);
    } else {
        asyncCtx->SignError(MSERR_EXT_INVALID_VAL, "Illegal argument");
    }

    size_t argIndex = 2;
    asyncCtx->callbackRef = CommonNapi::CreateReference(env, args[argIndex]);
    asyncCtx->deferred = CommonNapi::CreatePromise(env, asyncCtx->callbackRef, result);

    (void)napi_unwrap(env, jsThis, reinterpret_cast<void **>(&asyncCtx->napi_));

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "GetPreferredFrameRate", NAPI_AUTO_LENGTH, &resource);
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
            auto asyncCtx = reinterpret_cast<MediaVideoCapsAsyncCtx *>(data);
            if (asyncCtx == nullptr) {
                MEDIA_LOGE("Failed, asyncCtx is nullptr");
                return;
            } else if (asyncCtx->napi_ == nullptr || asyncCtx->napi_->caps_ == nullptr) {
                asyncCtx->SignError(MSERR_EXT_UNKNOWN, "nullptr");
                return;
            }
            Range range = asyncCtx->napi_->caps_->GetPreferredFrameRate(asyncCtx->width_, asyncCtx->height_);
            asyncCtx->JsResult = std::make_unique<MediaJsResultRange>(range.minVal, range.maxVal);
        },
        MediaAsyncContext::CompleteCallback, static_cast<void *>(asyncCtx.get()), &asyncCtx->work));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCtx->work));
    asyncCtx.release();

    return result;
}

napi_value MediaCapabilityVCapsNapi::GetCodecInfo(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    auto codecInfo = napi->caps_->GetCodecInfo();
    CHECK_AND_RETURN_RET(codecInfo != nullptr, undefined);

    CHECK_AND_RETURN_RET(napi_create_object(env, &jsResult) == napi_ok, undefined);
    (void)CommonNapi::SetPropertyString(env, jsResult, "name", codecInfo->GetName());
    (void)CommonNapi::SetPropertyInt32(env, jsResult, "type", static_cast<int32_t>(codecInfo->GetType()));
    (void)CommonNapi::SetPropertyString(env, jsResult, "mimeType", codecInfo->GetMimeType());
    (void)CommonNapi::SetPropertyInt32(env, jsResult, "isHardwareAccelerated",
        static_cast<int32_t>(codecInfo->IsHardwareAccelerated()));
    (void)CommonNapi::SetPropertyInt32(env, jsResult, "isSoftwareOnly",
        static_cast<int32_t>(codecInfo->IsSoftwareOnly()));
    (void)CommonNapi::SetPropertyInt32(env, jsResult, "isVendor", static_cast<int32_t>(codecInfo->IsVendor()));

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedBitrate(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    Range range = napi->caps_->GetSupportedBitrate();
    CHECK_AND_RETURN_RET(napi_create_object(env, &jsResult) == napi_ok, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "min", range.minVal) == true, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "max", range.maxVal) == true, undefined);

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedFormats(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    std::vector<int32_t> vec = napi->caps_->GetSupportedFormats();
    CHECK_AND_RETURN_RET(napi_create_array_with_length(env, vec.size(), &jsResult) == napi_ok, undefined);

    for (uint32_t i = 0; i < vec.size(); i++) {
        napi_value number = nullptr;
        (void)napi_create_int32(env, vec.at(i), &number);
        (void)napi_set_element(env, jsResult, i, number);
    }

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedHeightAlignment(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    status = napi_create_int32(env, napi->caps_->GetSupportedHeightAlignment(), &jsResult);
    CHECK_AND_RETURN_RET(status == napi_ok, undefined);

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedWidthAlignment(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    status = napi_create_int32(env, napi->caps_->GetSupportedWidthAlignment(), &jsResult);
    CHECK_AND_RETURN_RET(status == napi_ok, undefined);

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedWidth(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    Range range = napi->caps_->GetSupportedWidth();
    CHECK_AND_RETURN_RET(napi_create_object(env, &jsResult) == napi_ok, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "min", range.minVal) == true, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "max", range.maxVal) == true, undefined);

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedHeight(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    Range range = napi->caps_->GetSupportedHeight();
    CHECK_AND_RETURN_RET(napi_create_object(env, &jsResult) == napi_ok, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "min", range.minVal) == true, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "max", range.maxVal) == true, undefined);

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedProfiles(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    std::vector<int32_t> vec = napi->caps_->GetSupportedProfiles();
    CHECK_AND_RETURN_RET(napi_create_array_with_length(env, vec.size(), &jsResult) == napi_ok, undefined);

    for (uint32_t i = 0; i < vec.size(); i++) {
        napi_value number = nullptr;
        (void)napi_create_int32(env, vec.at(i), &number);
        (void)napi_set_element(env, jsResult, i, number);
    }

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedLevels(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    std::vector<int32_t> vec = napi->caps_->GetSupportedLevels();
    CHECK_AND_RETURN_RET(napi_create_array_with_length(env, vec.size(), &jsResult) == napi_ok, undefined);

    for (uint32_t i = 0; i < vec.size(); i++) {
        napi_value number = nullptr;
        (void)napi_create_int32(env, vec.at(i), &number);
        (void)napi_set_element(env, jsResult, i, number);
    }

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedBitrateMode(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    std::vector<int32_t> vec = napi->caps_->GetSupportedBitrateMode();
    CHECK_AND_RETURN_RET(napi_create_array_with_length(env, vec.size(), &jsResult) == napi_ok, undefined);

    for (uint32_t i = 0; i < vec.size(); i++) {
        napi_value number = nullptr;
        (void)napi_create_int32(env, vec.at(i), &number);
        (void)napi_set_element(env, jsResult, i, number);
    }

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedQuality(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    Range range = napi->caps_->GetSupportedQuality();
    CHECK_AND_RETURN_RET(napi_create_object(env, &jsResult) == napi_ok, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "min", range.minVal) == true, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "max", range.maxVal) == true, undefined);

    return jsResult;
}

napi_value MediaCapabilityVCapsNapi::SupportedComplexity(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value jsResult = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    CHECK_AND_RETURN_RET(status == napi_ok && jsThis != nullptr, undefined);

    MediaCapabilityVCapsNapi *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET(status == napi_ok && napi != nullptr && napi->caps_ != nullptr, undefined);

    Range range = napi->caps_->GetSupportedComplexity();
    CHECK_AND_RETURN_RET(napi_create_object(env, &jsResult) == napi_ok, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "min", range.minVal) == true, undefined);
    CHECK_AND_RETURN_RET(CommonNapi::SetPropertyInt32(env, jsResult, "max", range.maxVal) == true, undefined);

    return jsResult;
}
} // namespace Media
} // namespace OHOS
