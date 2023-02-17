/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef MEDIA_DATA_SOURCE_CALLBACK_H_
#define MEDIA_DATA_SOURCE_CALLBACK_H_

#include <mutex>
#include <uv.h>
#include "media_data_source.h"
#include "common_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Media {
const std::string READAT_CALLBACK_NAME = "readAt";

class MediaDataSourceCallback : public IMediaDataSource, public NoCopyable {
public:
    MediaDataSourceCallback(napi_env env, int64_t fileSize);
    virtual ~MediaDataSourceCallback();
    int32_t ReadAt(const std::shared_ptr<AVSharedMemory> &mem, uint32_t length, int64_t pos = -1) override;
    int32_t GetSize(int64_t &size) override;
    void SaveCallbackReference(const std::string &name, std::shared_ptr<AutoRef> ref);
    void ClearCallbackReference();
    static bool AddNapiValueProp(napi_env env, napi_value obj, const std::string &key, napi_value value);

    // This interface has been deprecated
    int32_t ReadAt(int64_t pos, uint32_t length, const std::shared_ptr<AVSharedMemory> &mem) override;
    // This interface has been deprecated
    int32_t ReadAt(uint32_t length, const std::shared_ptr<AVSharedMemory> &mem) override;
private:
    int32_t uv_work(uv_loop_s *loop, uv_work_t *work);
    struct MediaDataSourceJsCallback {
        MediaDataSourceJsCallback(const std::string &callbackName, const std::shared_ptr<AVSharedMemory> &mem,
            uint32_t length, int64_t pos)
            :callbackName_(callbackName), memory_(mem), length_(length), pos_(pos) {
        }
        ~MediaDataSourceJsCallback()
        {
            isExit_ = true;
            if (memory_ != nullptr) {
                memory_ = nullptr;
            }
        }
        std::weak_ptr<AutoRef> callback_;
        std::string callbackName_;
        std::shared_ptr<AVSharedMemory> memory_;
        uint32_t length_;
        int64_t pos_;
        int32_t readSize_;
        std::mutex mutexCond_;
        std::condition_variable cond_;
        bool setResult_ = false;
        bool isExit_ = false;
        void WaitResult()
        {
            std::unique_lock<std::mutex> lock(mutexCond_);
            if (!setResult_) {
                static constexpr int32_t timeout = 3;
                cond_.wait_for(lock, std::chrono::seconds(timeout), [this]() { return setResult_ || isExit_; });
                if (!(setResult_ || isExit_)) {
                    readSize_ = 0;
                }
            }
            setResult_ = false;
        }
    };
    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::map<std::string, std::shared_ptr<AutoRef>> refMap_;
    int64_t size_ = -1;
};
} // namespace Media
} // namespace OHOS
#endif // MEDIA_DATA_SOURCE_CALLBACK_H_
