/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include "monitor_client.h"
#include "i_media_service.h"
#include "media_log.h"
#include "media_errors.h"
#include "scope_guard.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MonitorClient"};
}

namespace OHOS {
namespace Media {
MonitorClient::MonitorClient()
{
    MEDIA_LOGI("Instances create");
}

MonitorClient::~MonitorClient()
{
    MEDIA_LOGI("Instances destroy");
    clientDestroy_ = true;
    std::unique_lock<std::mutex> lock(mutex_);
    if (clickThread_ != nullptr) {
        MEDIA_LOGI("clear monitor client thread");
        if (clickThread_->joinable()) {
            clickCond_.notify_all();
            lock.unlock();
            clickThread_->join();
        }
        clickThread_.reset();
        clickThread_ = nullptr;
    }
}

MonitorClient &MonitorClient::GetInstance()
{
    static MonitorClient monitor;
    return monitor;
}

bool MonitorClient::IsVaildProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (monitorProxy_ == nullptr || isVaildProxy_ == false) {
        monitorProxy_ = MediaServiceFactory::GetInstance().GetMonitorProxy();
        CHECK_AND_RETURN_RET_LOG(monitorProxy_ != nullptr, false, "monitorProxy_ is nullptr!");
        isVaildProxy_ = true;
    }

    return true;
}

int32_t MonitorClient::StartClick(MonitorClientObject *obj)
{
    MEDIA_LOGI("0x%{public}06" PRIXPTR " StartClick", FAKE_POINTER(obj));
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(objSet_.count(obj) == 0, MSERR_OK, "It has already been activated");

    objSet_.insert(obj);

    // The original thread has already exited. Need to recycle resources
    if (clickThread_ != nullptr && threadRunning_.load() == false) {
        if (clickThread_->joinable()) {
            clickThread_->join();
        }
        clickThread_.reset();
        clickThread_ = nullptr;
    }

    // Start Thread
    if (clickThread_ == nullptr) {
        threadRunning_ = true;
        clickThread_ = std::make_unique<std::thread>(&MonitorClient::ClickThread, this);
    }

    return MSERR_OK;
}

int32_t MonitorClient::StopClick(MonitorClientObject *obj)
{
    MEDIA_LOGI("0x%{public}06" PRIXPTR " StopClick", FAKE_POINTER(obj));
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(objSet_.count(obj), MSERR_OK, "Not started");

    objSet_.erase(obj);
    if (objSet_.empty()) {
        clickCond_.notify_all();
    }

    return MSERR_OK;
}

void MonitorClient::MediaServerDied()
{
    MEDIA_LOGI("MediaServerDied");
    std::lock_guard<std::mutex> lock(mutex_);
    objSet_.clear();
    isVaildProxy_ = false;
    clickCond_.notify_all();
}

void MonitorClient::ClickThread()
{
    pthread_setname_np(pthread_self(), "MonitorClick");
    MEDIA_LOGI("ClickThread start");
    static constexpr uint8_t timeInterval = 1; // Heartbeat once per second
    ON_SCOPE_EXIT(0) {
        threadRunning_ = false;
    };

    CHECK_AND_RETURN_LOG(IsVaildProxy(), "Proxy is invaild!");
    CHECK_AND_RETURN_LOG(monitorProxy_->EnableMonitor() == MSERR_OK, "failed to EnableMonitor");

    while (true) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            clickCond_.wait_for(lock, std::chrono::seconds(timeInterval), [this] {
                return objSet_.empty() || !isVaildProxy_ || clientDestroy_;
            });

            if (objSet_.empty()) {
                MEDIA_LOGI("objSet empty, clickThread Stop.");
                break;
            }

            if (clientDestroy_) {
                MEDIA_LOGI("monitor client destroy, clickThread Stop.");
                return;
            }
        }

        CHECK_AND_RETURN_LOG(IsVaildProxy(), "Proxy is invaild!");
        CHECK_AND_RETURN_LOG(monitorProxy_->Click() == MSERR_OK, "failed to Click");
    }

    CHECK_AND_RETURN_LOG(IsVaildProxy(), "Proxy is invaild!");
    CHECK_AND_RETURN_LOG(monitorProxy_->DisableMonitor() == MSERR_OK, "failed to DisableMonitor");
    MEDIA_LOGI("ClickThread End");
}
} // namespace Media
} // namespace OHOS
