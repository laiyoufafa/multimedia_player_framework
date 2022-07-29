/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NATIVE_AVCODEC_VIDEOENCODER_H
#define NATIVE_AVCODEC_VIDEOENCODER_H

#include <stdint.h>
#include <stdio.h>
#include "native_averrors.h"
#include "native_avformat.h"
#include "native_avmemory.h"
#include "native_avcodec_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a video encoder instance from the mime type, which is recommended in most cases.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param mime mime type description string, refer to {@link AVCODEC_MIME_TYPE}
 * @return Returns a pointer to an AVCodec instance
 * @since 9
 * @version 1.0
 */
AVCodec* OH_VideoEncoder_CreateByMime(const char *mime);

/**
 * @brief Create a video encoder instance through the video encoder name. The premise of using this interface is to
 * know the exact name of the encoder.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param name Video encoder name
 * @return Returns a pointer to an AVCodec instance
 * @since 9
 * @version 1.0
 */
AVCodec* OH_VideoEncoder_CreateByName(const char *name);

/**
 * @brief Clear the internal resources of the encoder and destroy the encoder instance
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_Destroy(AVCodec *codec);

/**
 * @brief Set the asynchronous callback function so that your application can respond to the events generated by the
 * video encoder. This interface must be called before Prepare is called
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @param callback A collection of all callback functions, see {@link AVCodecAsyncCallback}
 * @param userData User specific data
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_SetCallback(AVCodec *codec, AVCodecAsyncCallback callback, void *userData);

/**
 * @brief To configure the video encoder, typically, you need to configure the description information of the
 * encoded video track. This interface must be called before Prepare is called.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @param format A pointer to an AVFormat that gives the description of the video track to be encoded
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_Configure(AVCodec *codec, AVFormat *format);

/**
 * @brief To prepare the internal resources of the encoder, the Configure interface must be called before
 * calling this interface.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_Prepare(AVCodec *codec);

/**
 * @brief Start the encoder, this interface must be called after the Prepare is successful. After being
 * successfully started, the encoder will start reporting NeedInputData events.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_Start(AVCodec *codec);

/**
 * @brief Stop the encoder. After stopping, you can re-enter the Started state through Start.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_Stop(AVCodec *codec);

/**
 * @brief Clear the input and output data buffered in the encoder. After this interface is called, all the Buffer
 * indexes previously reported through the asynchronous callback will be invalidated, make sure not to access the
 * Buffers corresponding to these indexes.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_Flush(AVCodec *codec);

/**
 * @brief Reset the encoder. To continue coding, you need to call the Configure interface again to
 * configure the encoder instance.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_Reset(AVCodec *codec);

/**
 * @brief Get the description information of the output data of the encoder, refer to {@link AVFormat} for details.
 * It should be noted that the life cycle of the AVFormat instance pointed to by the return value will be invalid
 * when the interface is called next time or the AVCodec instance is destroyed. 
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns a pointer to an AVFormat instance
 * @since 9
 * @version 1.0
 */
AVFormat* OH_VideoEncoder_GetOutputDescription(AVCodec *codec);

/**
 * @brief Set dynamic parameters to the encoder. Note: This interface can only be called after the encoder is started.
 * At the same time, incorrect parameter settings may cause the encoding to fail.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @param format AVFormat handle pointer
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_SetParameter(AVCodec *codec, AVFormat *format);


/**
 * @brief Get the input Surface from the video encoder, this interface must be called before Prepare is called.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @param window A pointer to a NativeWindow instance, see {@link NativeWindow}
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_GetSurface(AVCodec *codec, NativeWindow **window);

/**
 * @brief Return the processed output Buffer to the encoder.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @param index The index value corresponding to the output Buffer
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_FreeOutputData(AVCodec *codec, uint32_t index);

/**
 * @brief Notifies the video encoder that the input stream has ended. It is recommended to use this interface to notify
 * the encoder of the end of the stream in surface mode, and it is recommended to use the
 * OH_AVCODEC_VideoEncoderPushInputData interface to notify the encoder of the end of the stream in bytebuffer mode.
 * @syscap SystemCapability.Multimedia.Media.VideoEncoder
 * @param codec pointer to an AVCodec instance
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link AVErrCode}
 * @since 9
 * @version 1.0
 */
AVErrCode OH_VideoEncoder_NotifyEndOfStream(AVCodec *codec);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVCODEC_VIDEOENCODER_H