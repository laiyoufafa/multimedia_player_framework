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

#include "recordersetaudiosource_fuzzer.h"
#include <iostream>
#include <cmath>
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
RecorderSetAudioSourceFuzzer::RecorderSetAudioSourceFuzzer()
{
}
RecorderSetAudioSourceFuzzer::~RecorderSetAudioSourceFuzzer()
{
}
bool RecorderSetAudioSourceFuzzer::FuzzRecorderSetAudioSource(uint8_t *data, size_t size)
{
    constexpr int32_t audioSourceTypesList = 3;
    constexpr uint32_t recorderTime = 5;
    RETURN_IF(TestRecorder::CreateRecorder(), false);

    static VideoRecorderConfig_ g_videoRecorderConfig;
    g_videoRecorderConfig.outputFd = open("/data/test/media/recorder_audio_es.m4a", O_RDWR);

    AudioSourceType AudioSourceType[audioSourceTypesList] {
        AUDIO_SOURCE_INVALID,
        AUDIO_SOURCE_DEFAULT,
        AUDIO_MIC,
    };

    int32_t sourcesubscript = abs((ProduceRandomNumberCrypt()) % (audioSourceTypesList));
    int32_t sourceId = *reinterpret_cast<int32_t *>(data);

    g_videoRecorderConfig.aSource = AudioSourceType[sourcesubscript];
    g_videoRecorderConfig.audioSourceId = sourceId;
    
    if (g_videoRecorderConfig.outputFd > 0) {
        RETURN_IF(TestRecorder::SetAudioSource(g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::SetOutputFormat(g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::CameraServicesForAudio(g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::SetMaxDuration(g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::SetOutputFile(g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::SetRecorderCallback(g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::Prepare(g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::Start(g_videoRecorderConfig), true);
        sleep(recorderTime);
        RETURN_IF(TestRecorder::Stop(false, g_videoRecorderConfig), true);
        RETURN_IF(TestRecorder::Release(g_videoRecorderConfig), true);
    }
    close(g_videoRecorderConfig.outputFd);
    return true;
}
}
bool FuzzTestRecorderSetAudioSource(uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }

    if (size < sizeof(int32_t)) {
        return 0;
    }
    RecorderSetAudioSourceFuzzer testRecorder;
    return testRecorder.FuzzRecorderSetAudioSource(data, size);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzTestRecorderSetAudioSource(data, size);
    return 0;
}

