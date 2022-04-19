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

#ifndef TEST_PLAYER_SETVOLUME_FUZZER_H
#define TEST_PLAYER_SETVOLUME_FUZZER_H

#define FUZZ_PROJECT_NAME "test_player_setvolume_fuzzer"
#include "test_player.h"

namespace OHOS {
namespace Media {
bool FuzzPlayerSetVolume(const uint8_t* data, size_t size);

class TestPlayerSetVolumeFuzz : public TestPlayer {
public:
    TestPlayerSetVolumeFuzz();
    ~TestPlayerSetVolumeFuzz();
    bool FuzzSetVolume(const uint8_t* data, size_t size);
};
}
}
#endif

