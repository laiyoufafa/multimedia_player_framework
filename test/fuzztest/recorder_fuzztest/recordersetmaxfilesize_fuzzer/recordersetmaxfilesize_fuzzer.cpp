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

#include "recordersetmaxfilesize_fuzzer.h"
#include <iostream>
#include "aw_common.h"
#include "string_ex.h"
#include "media_errors.h"
#include "directory_ex.h"
#include "recorder.h"

using namespace std;
using namespace OHOS;
using namespace Media;
using namespace PlayerTestParam;
using namespace RecorderTestParam;

namespace OHOS {
namespace Media {
RecorderSetMaxFileSizeFuzzer::RecorderSetMaxFileSizeFuzzer()
{
}

RecorderSetMaxFileSizeFuzzer::~RecorderSetMaxFileSizeFuzzer()
{
}

bool RecorderSetMaxFileSizeFuzzer::FuzzRecorderSetMaxFileSize(uint8_t *data, size_t size)
{
	constexpr uint32_t recorderRate = 30;
    RETURN_IF(TestRecorder::CreateRecorder(), false);

    static VideoRecorderConfig_ g_videoRecorderConfig;
    g_videoRecorderConfig.vSource = VIDEO_SOURCE_SURFACE_YUV;
    g_videoRecorderConfig.videoFormat = MPEG4;
    g_videoRecorderConfig.outputFd = open("/data/test/media/recorder_SetMaxFileSize.mp4", O_RDWR);

    if (g_videoRecorderConfig.outputFd > 0) {
        RETURN_IF(TestRecorder::SetVideoSource(g_videoRecorderConfig), false);
        RETURN_IF(TestRecorder::SetOutputFormat(g_videoRecorderConfig), false);
        RETURN_IF(TestRecorder::CameraServicesForVideo(g_videoRecorderConfig), false);
        RETURN_IF(TestRecorder::SetCaptureRate(g_videoRecorderConfig, recorderRate), false);

        int64_t sizeValue = *reinterpret_cast<int64_t *>(data);

        RETURN_IF(TestRecorder::SetMaxFileSize(sizeValue, g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::SetNextOutputFile(g_videoRecorderConfig), true);
    }
    close(g_videoRecorderConfig.outputFd);
    return false;
}
}

bool FuzzTestRecorderSetMaxFileSize(uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return true;
    }

    if (size < sizeof(int64_t)) {
        return true;
    }
    RecorderSetMaxFileSizeFuzzer testRecorder;
    return testRecorder.FuzzRecorderSetMaxFileSize(data, size);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzTestRecorderSetMaxFileSize(data, size);
    return 0;
}
