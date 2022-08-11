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

#ifndef TEST_PARAMS_CONFIG_H
#define TEST_PARAMS_CONFIG_H

#include <cstdint>
#include <string>
#include "avmetadatahelper.h"
#include "recorder.h"

namespace OHOS {
namespace Media {
namespace PlayerTestParam {
inline constexpr int32_t SEEK_TIME_5_SEC = 5000;
inline constexpr int32_t SEEK_TIME_2_SEC = 2000;
inline constexpr int32_t WAITSECOND = 6;
inline constexpr int32_t DELTA_TIME = 1000;
const std::string MEDIA_ROOT = "file:///data/test/";
const std::string VIDEO_FILE1 = MEDIA_ROOT + "H264_AAC.mp4";
} // namespace PlayerTestParam
namespace AVMetadataTestParam {
inline constexpr int32_t PARA_MAX_LEN = 256;
#define AVMETA_KEY_TO_STRING_MAP_ITEM(key) { key, #key }
static const std::unordered_map<int32_t, std::string_view> AVMETA_KEY_TO_STRING_MAP = {
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_ALBUM),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_ALBUM_ARTIST),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_DATE_TIME),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_ARTIST),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_AUTHOR),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_COMPOSER),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_DURATION),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_GENRE),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_HAS_AUDIO),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_HAS_VIDEO),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_MIME_TYPE),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_NUM_TRACKS),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_SAMPLE_RATE),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_TITLE),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_VIDEO_HEIGHT),
    AVMETA_KEY_TO_STRING_MAP_ITEM(AV_KEY_VIDEO_WIDTH),
};
} // namespace PlAVMetadataTestParamayerTestParam
namespace RecorderTestParam {
    constexpr uint32_t STUB_STREAM_SIZE = 602;
    constexpr uint32_t FRAME_RATE = 30000;
    constexpr uint32_t CODEC_BUFFER_WIDTH = 1024;
    constexpr uint32_t CODEC_BUFFER_HEIGHT = 25;
    constexpr uint32_t YUV_BUFFER_WIDTH = 1280;
    constexpr uint32_t YUV_BUFFER_HEIGHT = 720;
    constexpr uint32_t STRIDE_ALIGN = 8;
    constexpr uint32_t FRAME_DURATION = 40000000;
    constexpr uint32_t RECORDER_TIME = 5;
    constexpr uint32_t YUV_BUFFER_SIZE = YUV_BUFFER_WIDTH * YUV_BUFFER_HEIGHT * 3 / 2;
    constexpr uint32_t SEC_TO_NS = 1000000000;
    const std::string RECORDER_ROOT = "/data/test/media/";
    const std::string PURE_VIDEO = "video";
    const std::string PURE_AUDIO = "audio";
    const std::string AUDIO_VIDEO = "av";
    // this array contains each buffer size of the stub stream
    const uint32_t HIGH_VIDEO_FRAME_SIZE[STUB_STREAM_SIZE] = {
        13571, 321, 72, 472, 68, 76, 79, 509, 90, 677, 88, 956, 99, 347, 77, 452, 681, 81, 1263, 94, 106, 97, 998,
        97, 797, 93, 1343, 150, 116, 117, 926, 1198, 128, 110, 78, 1582, 158, 135, 112, 1588, 165, 132, 128, 1697,
        168, 149, 117, 1938, 170, 141, 142, 1830, 106, 161, 122, 1623, 160, 154, 156, 1998, 230, 177, 139, 1650,
        186, 128, 134, 1214, 122, 1411, 120, 1184, 128, 1591, 195, 145, 105, 1587, 169, 140, 118, 1952, 177, 150,
        161, 1437, 159, 123, 1758, 180, 165, 144, 1936, 214, 191, 175, 2122, 180, 179, 160, 1927, 161, 184, 119,
        1973, 218, 210, 129, 1962, 196, 127, 154, 2308, 173, 127, 1572, 142, 122, 2065, 262, 159, 206, 2251, 269,
        179, 170, 2056, 308, 168, 191, 2090, 303, 191, 110, 1932, 272, 162, 122, 1877, 245, 167, 141, 1908, 294,
        162, 118, 1493, 132, 1782, 273, 184, 133, 1958, 274, 180, 149, 2070, 216, 169, 143, 1882, 224, 149, 139,
        1749, 277, 184, 139, 2141, 197, 170, 140, 2002, 269, 162, 140, 1862, 202, 179, 131, 1868, 214, 164, 140,
        1546, 226, 150, 130, 1707, 162, 146, 1824, 181, 147, 130, 1898, 209, 143, 131, 1805, 180, 148, 106, 1776,
        147, 141, 1572, 177, 130, 105, 1776, 178, 144, 122, 1557, 142, 124, 114, 1436, 143, 126, 1326, 127, 1755,
        169, 127, 105, 1807, 177, 131, 134, 1613, 187, 137, 136, 1314, 134, 118, 2005, 194, 129, 147, 1566, 185,
        132, 131, 1236, 174, 137, 106, 11049, 574, 126, 1242, 188, 130, 119, 1450, 187, 137, 141, 1116, 124, 1848,
        138, 122, 1605, 186, 127, 140, 1798, 170, 124, 121, 1666, 157, 128, 130, 1678, 135, 118, 1804, 169, 135,
        125, 1837, 168, 124, 124, 2049, 180, 122, 128, 1334, 143, 128, 1379, 116, 1884, 149, 122, 150, 1962, 176,
        122, 122, 1197, 139, 1853, 184, 151, 148, 1692, 209, 129, 126, 1736, 149, 135, 104, 1775, 165, 160, 121,
        1653, 163, 123, 112, 1907, 181, 129, 107, 1808, 177, 125, 111, 2405, 166, 144, 114, 1833, 198, 136, 113,
        1960, 206, 139, 116, 1791, 175, 130, 129, 1909, 194, 138, 119, 1807, 160, 156, 124, 1998, 184, 173, 114,
        2069, 181, 127, 139, 2212, 182, 138, 146, 1993, 214, 135, 139, 2286, 194, 137, 120, 2090, 196, 159, 132,
        2294, 194, 148, 137, 2312, 183, 163, 106, 2118, 201, 158, 127, 2291, 187, 144, 116, 2413, 139, 115, 2148,
        178, 122, 103, 2370, 207, 161, 117, 2291, 213, 159, 129, 2244, 243, 157, 133, 2418, 255, 171, 127, 2316,
        185, 160, 132, 2405, 220, 165, 155, 2539, 219, 172, 128, 2433, 199, 154, 119, 1681, 140, 1960, 143, 2682,
        202, 153, 127, 2794, 239, 158, 155, 2643, 229, 172, 125, 2526, 201, 181, 159, 2554, 233, 167, 125, 2809,
        205, 164, 117, 2707, 221, 156, 138, 2922, 240, 160, 146, 2952, 267, 177, 149, 3339, 271, 175, 136, 3006,
        242, 168, 141, 3068, 232, 194, 149, 2760, 214, 208, 143, 2811, 218, 184, 149, 137, 15486, 2116, 235, 167,
        157, 2894, 305, 184, 139, 3090, 345, 179, 155, 3226, 347, 160, 164, 3275, 321, 184, 174, 3240, 269, 166,
        170, 3773, 265, 169, 155, 3023, 301, 188, 161, 3343, 275, 174, 155, 3526, 309, 177, 173, 3546, 307, 183,
        149, 3648, 295, 213, 170, 3568, 305, 198, 166, 3641, 297, 172, 148, 3608, 301, 200, 159, 3693, 322, 209,
        166, 3453, 318, 206, 162, 3696, 341, 200, 176, 3386, 320, 192, 176, 3903, 373, 207, 187, 3305, 361, 200,
        202, 3110, 367, 220, 197, 2357, 332, 196, 201, 1827, 377, 187, 199, 860, 472, 173, 223, 238};

    struct VideoRecorderConfig {
        int32_t audioSourceId = 0;
        int32_t videoSourceId = 0;
        int32_t audioEncodingBitRate = 48000;
        int32_t channelCount = 2;
        int32_t duration = 60;
        int32_t width = YUV_BUFFER_WIDTH;
        int32_t height = YUV_BUFFER_HEIGHT;
        int32_t frameRate = 30;
        int32_t videoEncodingBitRate = 48000;
        int32_t sampleRate = 48000;
        double captureFps = 30;
        int32_t outputFd = 0;
        AudioCodecFormat audioFormat = AAC_LC;
        AudioSourceType aSource = AUDIO_MIC;
        OutputFormatType outPutFormat = FORMAT_MPEG_4;
        VideoSourceType vSource = VIDEO_SOURCE_SURFACE_ES;
        VideoCodecFormat videoFormat = MPEG4;
    };
    struct AudioRecorderConfig {
        int32_t outputFd = 0;
        int32_t audioSourceId = 0;
        int32_t audioEncodingBitRate = 48000;
        int32_t channelCount = 2;
        int32_t duration = 60;
        int32_t sampleRate = 48000;
        AudioCodecFormat audioFormat = AAC_LC;
        AudioSourceType inputSource = AUDIO_MIC;
        OutputFormatType outPutFormat = FORMAT_M4A;
    };
} // namespace RecorderTestParam
namespace VCodecTestParam {
constexpr uint32_t DEFAULT_WIDTH = 320;
constexpr uint32_t DEFAULT_HEIGHT = 240;
constexpr uint32_t DEFAULT_FRAME_RATE = 60;
constexpr uint32_t FRAME_DURATION_US = 16670;
const std::string MIME_TYPE = "video/mp4v-es";
constexpr bool NEED_DUMP = true;
const uint32_t ES[] = { // H264_FRAME_SIZE_240
    2106, 11465, 321, 72, 472, 68, 76, 79, 509, 90, 677, 88, 956, 99, 347, 77, 452, 681, 81, 1263, 94, 106, 97,
    998, 97, 797, 93, 1343, 150, 116, 117, 926, 1198, 128, 110, 78, 1582, 158, 135, 112, 1588, 165, 132,
    128, 1697, 168, 149, 117, 1938, 170, 141, 142, 1830, 106, 161, 122, 1623, 160, 154, 156, 1998, 230,
    177, 139, 1650, 186, 128, 134, 1214, 122, 1411, 120, 1184, 128, 1591, 195, 145, 105, 1587, 169, 140,
    118, 1952, 177, 150, 161, 1437, 159, 123, 1758, 180, 165, 144, 1936, 214, 191, 175, 2122, 180, 179,
    160, 1927, 161, 184, 119, 1973, 218, 210, 129, 1962, 196, 127, 154, 2308, 173, 127, 1572, 142, 122,
    2065, 262, 159, 206, 2251, 269, 179, 170, 2056, 308, 168, 191, 2090, 303, 191, 110, 1932, 272, 162,
    122, 1877, 245, 167, 141, 1908, 294, 162, 118, 1493, 132, 1782, 273, 184, 133, 1958, 274, 180, 149,
    2070, 216, 169, 143, 1882, 224, 149, 139, 1749, 277, 184, 139, 2141, 197, 170, 140, 2002, 269, 162,
    140, 1862, 202, 179, 131, 1868, 214, 164, 140, 1546, 226, 150, 130, 1707, 162, 146, 1824, 181, 147,
    130, 1898, 209, 143, 131, 1805, 180, 148, 106, 1776, 147, 141, 1572, 177, 130, 105, 1776, 178, 144,
    122, 1557, 142, 124, 114, 1436, 143, 126, 1326, 127, 1755, 169, 127, 105, 1807, 177, 131, 134, 1613,
    187, 137, 136, 1314, 134, 118, 2005, 194, 129, 147, 1566, 185, 132, 131, 1236, 174, 137, 106, 11049,
    574, 126, 1242, 188, 130, 119, 1450, 187, 137, 141, 1116, 124, 1848, 138, 122, 1605, 186, 127, 140,
    1798, 170, 124, 121, 1666, 157, 128, 130, 1678, 135, 118, 1804, 169, 135, 125, 1837, 168, 124, 124,
    2049, 180, 122, 128, 1334, 143, 128, 1379, 116, 1884, 149, 122, 150, 1962, 176, 122, 122, 1197, 139,
    1853, 184, 151, 148, 1692, 209, 129, 126, 1736, 149, 135, 104, 1775, 165, 160, 121, 1653, 163, 123,
    112, 1907, 181, 129, 107, 1808, 177, 125, 111, 2405, 166, 144, 114, 1833, 198, 136, 113, 1960, 206,
    139, 116, 1791, 175, 130, 129, 1909, 194, 138, 119, 1807, 160, 156, 124, 1998, 184, 173, 114, 2069, 181,
    127, 139, 2212, 182, 138, 146, 1993, 214, 135, 139, 2286, 194, 137, 120, 2090, 196, 159, 132, 2294, 194,
    148, 137, 2312, 183, 163, 106, 2118, 201, 158, 127, 2291, 187, 144, 116, 2413, 139, 115, 2148, 178, 122,
    103, 2370, 207, 161, 117, 2291, 213, 159, 129, 2244, 243, 157, 133, 2418, 255, 171, 127, 2316, 185, 160,
    132, 2405, 220, 165, 155, 2539, 219, 172, 128, 2433, 199, 154, 119, 1681, 140, 1960, 143, 2682, 202, 153,
    127, 2794, 239, 158, 155, 2643, 229, 172, 125, 2526, 201, 181, 159, 2554, 233, 167, 125, 2809, 205, 164,
    117, 2707, 221, 156, 138, 2922, 240, 160, 146, 2952, 267, 177, 149, 3339, 271, 175, 136, 3006, 242, 168,
    141, 3068, 232, 194, 149, 2760, 214, 208, 143, 2811, 218, 184, 149, 137, 15486, 2116, 235, 167, 157, 2894,
    305, 184, 139, 3090, 345, 179, 155, 3226, 347, 160, 164, 3275, 321, 184, 174, 3240, 269, 166, 170, 3773,
    265, 169, 155, 3023, 301, 188, 161, 3343, 275, 174, 155, 3526, 309, 177, 173, 3546, 307, 183, 149, 3648,
    295, 213, 170, 3568, 305, 198, 166, 3641, 297, 172, 148, 3608, 301, 200, 159, 3693, 322, 209, 166, 3453,
    318, 206, 162, 3696, 341, 200, 176, 3386, 320, 192, 176, 3903, 373, 207, 187, 3305, 361, 200, 202, 3110,
    367, 220, 197, 2357, 332, 196, 201, 1827, 377, 187, 199, 860, 472, 173, 223, 238};
constexpr uint32_t ES_LENGTH = sizeof(ES) / sizeof(uint32_t);
} // namespace VCodecTestParam
namespace ACodecTestParam {
constexpr uint32_t SAMPLE_DURATION_US = 23000;
constexpr bool NEED_DUMP = true;
const std::string MIME_TYPE = "audio/mp4a-latm";
constexpr uint32_t ES[] = {
    283, 336, 291, 405, 438, 411, 215, 215, 313, 270, 342, 641, 554, 545, 545, 546,
    541, 540, 542, 552, 537, 533, 498, 472, 445, 430, 445, 427, 414, 386, 413, 370, 380,
    401, 393, 369, 391, 367, 395, 396, 396, 385, 391, 384, 395, 392, 386, 388, 384, 379,
    376, 381, 375, 373, 349, 391, 357, 384, 395, 384, 380, 386, 372, 386, 383, 378, 385,
    385, 384, 342, 390, 379, 387, 386, 393, 397, 362, 393, 394, 391, 383, 385, 377, 379,
    381, 369, 375, 379, 346, 382, 356, 361, 366, 394, 393, 385, 362, 406, 399, 384, 377,
    385, 389, 375, 346, 396, 388, 381, 383, 352, 357, 397, 382, 395, 376, 388, 373, 374,
    353, 383, 384, 393, 379, 348, 364, 389, 380, 381, 388, 423, 392, 381, 368, 351, 391,
    355, 358, 395, 390, 385, 382, 383, 388, 388, 389, 376, 379, 376, 384, 369, 354, 390,
    389, 396, 393, 382, 385, 353, 383, 381, 377, 411, 387, 390, 377, 349, 381, 390, 378,
    373, 375, 381, 351, 392, 381, 380, 381, 378, 387, 379, 383, 348, 386, 364, 386, 371,
    399, 399, 385, 380, 355, 397, 395, 382, 380, 386, 352, 387, 390, 373, 372, 388, 378,
    385, 368, 385, 370, 378, 373, 383, 368, 373, 388, 351, 384, 391, 387, 389, 383, 355,
    361, 392, 386, 354, 394, 392, 397, 392, 352, 381, 395, 349, 383, 390, 392, 350, 393,
    393, 385, 389, 393, 382, 378, 384, 378, 375, 373, 375, 389, 377, 383, 387, 373, 344,
    388, 379, 391, 373, 384, 358, 361, 391, 394, 363, 350, 361, 395, 399, 389, 398, 375,
    398, 400, 381, 354, 363, 366, 400, 400, 356, 370, 400, 394, 398, 385, 378, 372, 354,
    359, 393, 381, 363, 396, 396, 355, 390, 356, 355, 371, 399, 367, 406, 375, 377, 405,
    401, 390, 393, 392, 384, 386, 374, 358, 397, 389, 393, 385, 345, 379, 357, 388, 356,
    381, 389, 367, 358, 391, 360, 394, 396, 357, 395, 388, 394, 383, 357, 383, 392, 394,
    376, 379, 356, 386, 395, 387, 377, 377, 389, 377, 385, 351, 387, 350, 388, 384, 345,
    358, 368, 399, 394, 385, 384, 395, 378, 387, 386, 386, 376, 375, 382, 351, 359, 356,
    401, 388, 363, 406, 363, 374, 435, 366, 400, 393, 392, 371, 391, 359, 359, 397, 388,
    390, 420, 411, 369, 384, 382, 383, 383, 375, 381, 361, 380, 348, 379, 386, 379, 379,
    386, 371, 352, 378, 378, 388, 384, 385, 352, 355, 387, 383, 379, 362, 386, 399, 376,
    390, 350, 387, 357, 403, 398, 397, 360, 351, 394, 400, 399, 393, 388, 395, 370, 377,
    395, 360, 346, 381, 370, 390, 380, 391, 387, 382, 384, 383, 354, 349, 394, 358, 387,
    400, 386, 402, 354, 396, 387, 391, 365, 377, 359, 361, 365, 395, 388, 388, 384, 388,
    378, 374, 382, 376, 377, 389, 378, 341, 390, 376, 381, 375, 414, 368, 369, 387, 411,
    396, 391, 378, 389, 349, 383, 344, 381, 387, 380, 353, 361, 391, 365, 390, 396, 382,
    386, 385, 385, 409, 387, 386, 378, 372, 372, 374, 349, 388, 389, 348, 395, 380, 382,
    388, 375, 347, 383, 359, 389, 368, 361, 405, 398, 393, 395, 359, 360, 395, 395, 362,
    354, 388, 348, 388, 386, 390, 350, 388, 356, 369, 364, 404, 404, 391, 394, 385, 439,
    432, 375, 366, 441, 362, 367, 382, 374, 346, 391, 371, 354, 376, 390, 373, 382, 385,
    389, 378, 377, 347, 414, 338, 348, 385, 352, 385, 386, 381, 388, 387, 364, 465, 405,
    443, 387, 339, 376, 337, 379, 387, 370, 374, 358, 354, 357, 393, 356, 381, 357, 407,
    361, 397, 362, 394, 394, 392, 394, 391, 381, 386, 379, 354, 351, 392, 408, 393, 389,
    388, 385, 375, 388, 375, 388, 375, 354, 384, 379, 386, 394, 383, 359, 405, 395, 352,
    345, 403, 427, 373, 380, 350, 415, 378, 434, 385, 388, 387, 400, 405, 329, 391, 356,
    419, 358, 359, 375, 367, 391, 359, 369, 361, 376, 378, 379, 348, 390, 345, 388, 390,
    406, 349, 368, 364, 391, 384, 401, 384, 391, 361, 399, 359, 386, 392, 382, 386, 380,
    383, 345, 376, 393, 400, 395, 343, 352, 354, 381, 388, 357, 393, 389, 384, 389, 388,
    384, 404, 372, 358, 381, 352, 355, 485, 393, 371, 376, 389, 377, 391, 387, 376, 342,
    390, 375, 379, 396, 376, 402, 353, 392, 382, 383, 387, 386, 372, 377, 382, 388, 381,
    387, 357, 393, 385, 346, 389, 388, 357, 362, 404, 398, 397, 402, 371, 351, 370, 362,
    350, 388, 399, 402, 406, 377, 396, 359, 372, 390, 392, 368, 383, 346, 384, 381, 379,
    367, 384, 389, 381, 371, 358, 422, 372, 382, 374, 444, 412, 369, 362, 373, 389, 401,
    383, 380, 366, 365, 361, 379, 372, 345, 382, 375, 376, 375, 382, 356, 395, 383, 384,
    391, 361, 396, 407, 365, 351, 385, 378, 403, 344, 352, 387, 397, 399, 377, 371, 381,
    415, 382, 388, 368, 383, 405, 390, 386, 384, 374, 375, 381, 371, 372, 374, 377, 346,
    358, 381, 377, 359, 385, 396, 385, 390, 389, 391, 375, 357, 389, 390, 377, 370, 379,
    351, 381, 381, 380, 371, 386, 389, 389, 383, 362, 393, 388, 355, 396, 383, 352, 384,
    352, 383, 362, 396, 385, 396, 357, 388, 382, 377, 373, 379, 383, 386, 350, 393, 355,
    380, 401, 392, 391, 402, 391, 427, 407, 394, 332, 398, 367, 373, 343, 381, 383, 386,
    382, 349, 353, 393, 378, 386, 375, 390, 356, 392, 384, 387, 380, 381, 385, 386, 383,
    378, 379, 359, 381, 382, 388, 357, 357, 397, 358, 424, 382, 352, 409, 374, 368, 365,
    399, 352, 393, 389, 385, 352, 380, 398, 389, 385, 387, 387, 353, 402, 396, 386, 357,
    395, 368, 369, 407, 394, 383, 362, 380, 385, 368, 375, 365, 379, 377, 388, 380, 346,
    383, 381, 399, 359, 386, 455, 368, 406, 377, 339, 381, 377, 373, 371, 338}; // replace of self frame length
constexpr uint32_t ES_LENGTH = sizeof(ES) / sizeof(uint32_t);
} // namespace ACodecTestParam
} // namespace Media
} // namespace OHOS

#endif