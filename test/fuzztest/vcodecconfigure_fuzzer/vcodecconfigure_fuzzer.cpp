/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "vcodecconfigure_fuzzer.h"
#include <iostream>
#include "string_ex.h"
#include "media_errors.h"
#include "directory_ex.h"
#include "ui/rs_surface_node.h"
#include "window_option.h"
#include "aw_common.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::VCodecTestParam;

VCodecConfigureFuzzer::VCodecConfigureFuzzer()
{
}

VCodecConfigureFuzzer::~VCodecConfigureFuzzer()
{
}

bool VCodecConfigureFuzzer::FuzzVideoConfigure(uint8_t *data, size_t size)
{
    std::shared_ptr<VDecSignal> vdecSignal = std::make_shared<VDecSignal>();
    vdecCallback_ = std::make_shared<VDecCallbackTest>(vdecSignal);
    CHECK_INSTANCE_AND_RETURN_RET(vdecCallback_, false);
    videoDec_ = std::make_shared<VDecMock>(vdecSignal);
    CHECK_INSTANCE_AND_RETURN_RET(videoDec_, false);
    CHECK_BOOL_AND_RETURN_RET(videoDec_->CreateVideoDecMockByMime("video/avc"), false);
    CHECK_STATE_AND_RETURN_RET(videoDec_->SetCallback(vdecCallback_), false);

    std::shared_ptr<VEncSignal> vencSignal = std::make_shared<VEncSignal>();
    vencCallback_ = std::make_shared<VEncCallbackTest>(vencSignal);
    CHECK_INSTANCE_AND_RETURN_RET(vencCallback_, false);
    videoEnc_ = std::make_shared<VEncMock>(vencSignal);
    CHECK_INSTANCE_AND_RETURN_RET(videoEnc_, false);
    CHECK_BOOL_AND_RETURN_RET(videoEnc_->CreateVideoEncMockByMime("video/avc"), false);
    CHECK_STATE_AND_RETURN_RET(videoEnc_->SetCallback(vencCallback_), false);

    videoEnc_->SetOutPath("/data/test/media/avc_configurefuzz_out.es");
    std::shared_ptr<FormatMock> format = AVCodecMockFactory::CreateFormat();
    CHECK_INSTANCE_AND_RETURN_RET(format, false);
    int32_t data_ = *reinterpret_cast<int32_t *>(data);
    (void)format->PutIntValue("width", DEFAULT_WIDTH);
    (void)format->PutIntValue("height", DEFAULT_HEIGHT);
    (void)format->PutIntValue("pixel_format", NV12);
    (void)format->PutIntValue("frame_rate", data_);
    videoDec_->SetSource(H264_SRC_PATH, ES_H264, ES_LENGTH_H264);
    videoEnc_->Configure(format);
    videoDec_->Configure(format);
    std::shared_ptr<SurfaceMock> surface = videoEnc_->GetInputSurface();
    videoDec_->SetOutputSurface(surface);
    videoDec_->Prepare();
    videoEnc_->Prepare();
    videoDec_->Start();
    videoEnc_->Start();
    sleep(WAITTING_TIME);
    if (videoDec_ != nullptr) {
        videoDec_->Reset();
        videoDec_->Release();
    }
    if (videoEnc_ != nullptr) {
        videoEnc_->Reset();
        videoEnc_->Release();
    }
    return true;
}

bool OHOS::Media::FuzzVCodecConfigure(uint8_t *data, size_t size)
{
    auto codecfuzzer = std::make_unique<VCodecConfigureFuzzer>();
    if (codecfuzzer == nullptr) {
        cout << "codecfuzzer is null" << endl;
        return 0;
    }
    return codecfuzzer->FuzzVideoConfigure(data, size);
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::Media::FuzzVCodecConfigure(data, size);
    return 0;
}