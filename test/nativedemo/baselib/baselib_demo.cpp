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

#include "baselib_demo.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "string_ex.h"
#include "securec.h"
using namespace OHOS;
using namespace OHOS::Media;
using namespace std;

namespace {
    constexpr uint32_t MIN_MALLOC_SIZE = 1;
    constexpr uint32_t MAX_MALLOC_SIZE = 10240;
    constexpr uint32_t PRINT_FREQ = 100;
}

void BaseLibDemo::RunCase()
{
    cout << "Please set run mode:" << endl;
    cout << "0: RunMultiThreadMem" << endl;
    cout << "1: RunMalloc" << endl;
    cout << "2: RunMemset_s" << endl;
    cout << "3: RunMemcpy_s" << endl;
    string mode;
    (void)getline(cin, mode);
    cout << "Please set run epoch (default is 2000) :" << endl;
    string str;
    (void)getline(cin, str);
    if (StrToInt(str, epoch_)) {
        cout << "set epoch success: " << epoch_ << endl;
    }

    cout << "Please set run times in every epoch (default is 1000) :" << endl;
    (void)getline(cin, str);
    if (StrToInt(str, runTimesPerEpoch_)) {
        cout << "set set run times in every epoch as: " << runTimesPerEpoch_ << endl;
    }

    if (mode == "0") {
        RunMultiThreadMem();
    } else if (mode == "1") {
        RunMalloc();
    } else if (mode == "2") {
        RunMemset_s();
    } else if (mode == "3") {
        RunMemcpy_s();
    }
    cout << "Please collect media_demo process information, enter any key to exit" << endl;
    (void)getline(cin, str);
}

void BaseLibDemo::GenerateRandList(int32_t *r, const int32_t &len, const int32_t &left, const int32_t &right)
{
    (void)srand(static_cast<int32_t>(time(nullptr)));
    for (int32_t i = 0; i < len; i++) {
        r[i] = rand() % (right - left + 1) + left;
    }
}

void BaseLibDemo::RunMultiThreadMem()
{
    cout << "===RunMultiThreadMem epoch: " << epoch_ << "===" << endl;
    memsetsThread_ = make_unique<thread>(&BaseLibDemo::RunMemset_s, this);
    memcpysThread_ = make_unique<thread>(&BaseLibDemo::RunMemcpy_s, this);
    mallocThread_ = make_unique<thread>(&BaseLibDemo::RunMalloc, this);

    memsetsThread_->join();
    memcpysThread_->join();
    mallocThread_->join();
    cout << "RunMultiThreadMem end" << endl;
}

void BaseLibDemo::RunMalloc()
{
    cout << "---RunMalloc epoch: " << epoch_ << "---" << endl;
    void *ptrs[runTimesPerEpoch_];
    int32_t randSize[runTimesPerEpoch_];
    GenerateRandList(randSize, runTimesPerEpoch_, MIN_MALLOC_SIZE, MAX_MALLOC_SIZE);
    for (int32_t epo = 0; epo < epoch_; epo++) {
        for (int32_t i = 0; i < runTimesPerEpoch_; i++) {
            ptrs[i] = malloc(randSize[i]);
        }
        for (int32_t i = 0; i < runTimesPerEpoch_; i++) {
            free(ptrs[i]);
        }
        if (epo % PRINT_FREQ== 0) {
            cout << "RunMalloc epoch now is:" << epo << endl;
        }
    }
}

void BaseLibDemo::RunMemset_s()
{
    cout << "---RunMemset_s epoch: " << epoch_ << "---" << endl;
    void *ptrs[runTimesPerEpoch_];
    int32_t randSize[runTimesPerEpoch_];
    GenerateRandList(randSize, runTimesPerEpoch_, MIN_MALLOC_SIZE, MAX_MALLOC_SIZE);
    for (int32_t epo = 0; epo < epoch_; epo++) {
        for (int32_t i = 0; i < runTimesPerEpoch_; i++) {
            ptrs[i] = malloc(randSize[i]);
            memset_s(ptrs[i], randSize[i], 0, randSize[i]);
        }
        for (int32_t i = 0; i < runTimesPerEpoch_; i++) {
            free(ptrs[i]);
        }
        if (epo % PRINT_FREQ== 0) {
        cout << "RunMemset_s epoch now is:" << epo << endl;
        }
    }
}

void BaseLibDemo::RunMemcpy_s()
{
    cout << "---RunMemset epoch: " << epoch_ << "---" << endl;
    void *ptrSrc[runTimesPerEpoch_];
    void *ptrDst[runTimesPerEpoch_];

    int32_t randSize[runTimesPerEpoch_];
    GenerateRandList(randSize, runTimesPerEpoch_, MIN_MALLOC_SIZE, MAX_MALLOC_SIZE);
    for (int32_t epo = 0; epo < epoch_; epo++) {
        for (int32_t i = 0; i < runTimesPerEpoch_; i++) {
            ptrSrc[i] = malloc(randSize[i]);
            ptrDst[i] = malloc(randSize[i]);
            memset_s(ptrSrc[i], randSize[i], 0, randSize[i]);
            memset_s(ptrDst[i], randSize[i], 1, randSize[i]);

            memcpy_s(ptrDst[i], randSize[i], ptrSrc[i], randSize[i]);
        }
        for (int32_t i = 0; i < runTimesPerEpoch_; i++) {
            free(ptrSrc[i]);
            free(ptrDst[i]);
        }
        if (epo % PRINT_FREQ== 0) {
            cout << "RunMemcpy_s epoch now is:" << epo << endl;
        }
    }
}

