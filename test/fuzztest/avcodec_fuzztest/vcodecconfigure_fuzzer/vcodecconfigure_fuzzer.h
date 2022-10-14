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

#ifndef VCODECCONFIGURE_FUZZER_H
#define VCODECCONFIGURE_FUZZER_H

#define FUZZ_PROJECT_NAME "vcodecconfigure_fuzzer"
#include "vdec_mock.h"
#include "venc_mock.h"

namespace OHOS {
namespace Media {
bool FuzzVCodecConfigure(uint8_t *data, size_t size);

class VCodecConfigureFuzzer : public NoCopyable {
public:
    VCodecConfigureFuzzer();
    ~VCodecConfigureFuzzer();
    bool FuzzVideoConfigure(uint8_t *data, size_t size);
protected:
    std::shared_ptr<VDecMock> videoDec_ = nullptr;
    std::shared_ptr<VDecCallbackTest> vdecCallback_ = nullptr;
    std::shared_ptr<VEncMock> videoEnc_ = nullptr;
    std::shared_ptr<VEncCallbackTest> vencCallback_ = nullptr;
};
}
}
#endif // VCODECCONFIGURE_FUZZER_H
