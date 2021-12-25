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
#ifndef AVMETADATAHELPER_IMPL_H
#define AVMETADATAHELPER_IMPL_H

#include "avmetadatahelper.h"
#include "nocopyable.h"
#include "i_avmetadatahelper_service.h"

namespace OHOS {
namespace Media {
class AVMetadataHelperImpl : public AVMetadataHelper {
public:
    AVMetadataHelperImpl();
    ~AVMetadataHelperImpl();
    DISALLOW_COPY_AND_MOVE(AVMetadataHelperImpl);

    int32_t SetSource(const std::string &uri, int32_t usage) override;
    std::string ResolveMetadata(int32_t key) override;
    std::unordered_map<int32_t, std::string> ResolveMetadata() override;
    std::shared_ptr<AVSharedMemory> FetchArtPicture() override;
    std::shared_ptr<PixelMap> FetchFrameAtTime(int64_t timeUs, int32_t option, const PixelMapParams &param) override;
    void Release() override;
    int32_t Init();
private:
    std::shared_ptr<IAVMetadataHelperService> avMetadataHelperService_ = nullptr;
};
} // Media
} // OHOS
#endif // AVMETADATAHELPER_IMPL_H
