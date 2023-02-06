/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "gst_video_capture_pool.h"
#include <gst/gst.h>
#include "buffer_type_meta.h"
#include "surface.h"
#include "scope_guard.h"
#include "securec.h"
#include "media_log.h"
#include "media_dfx.h"
#include "media_errors.h"
using namespace OHOS;

#define gst_video_capture_pool_parent_class parent_class

GST_DEBUG_CATEGORY_STATIC(gst_video_capture_pool_debug_category);
#define GST_CAT_DEFAULT gst_video_capture_pool_debug_category

enum {
    PROP_0,
    PROP_cached_data,
    PROP_BUFFER_COUNT,
};

G_DEFINE_TYPE(GstVideoCapturePool, gst_video_capture_pool, GST_TYPE_CONSUMER_SURFACE_POOL);

static void gst_video_capture_pool_set_property(GObject *object, guint id, const GValue *value, GParamSpec *pspec);
static void gst_video_capture_pool_get_property(GObject *object, guint id, GValue *value, GParamSpec *pspec);
static GstFlowReturn gst_video_capture_pool_buffer_available(GstConsumerSurfacePool *surfacepool);
static GstFlowReturn gst_video_capture_pool_alloc_buffer(GstBufferPool *pool, GstBuffer **buffer);
static GstFlowReturn gst_video_capture_pool_get_buffer(GstConsumerSurfacePool *surfacepool, GstBuffer **buffer);

static void gst_video_capture_pool_class_init(GstVideoCapturePoolClass *klass)
{
    g_return_if_fail(klass != nullptr);
    GObjectClass *gobjectClass = G_OBJECT_CLASS(klass);
    GST_DEBUG_CATEGORY_INIT(gst_video_capture_pool_debug_category, "videocapturepool", 0,
        "video capture pool base class");

    gobjectClass->set_property = gst_video_capture_pool_set_property;
    gobjectClass->get_property = gst_video_capture_pool_get_property;
   
    g_object_class_install_property(gobjectClass, PROP_cached_data,
        g_param_spec_boolean("cached-data", "es pause", "es pause",
            FALSE, (GParamFlags)(G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS)));
    
    g_object_class_install_property(gobjectClass, PROP_BUFFER_COUNT,
        g_param_spec_uint("buffer-count", "buffer count", "buffer count",
            0, G_MAXUINT32, 0, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
}

static void gst_video_capture_pool_init(GstVideoCapturePool *pool)
{
    g_return_if_fail(pool != nullptr);
    GstConsumerSurfacePool *surfacepool = GST_CONSUMER_SURFACE_POOL(pool);
    surfacepool->buffer_available = gst_video_capture_pool_buffer_available;
    surfacepool->alloc_buffer = gst_video_capture_pool_alloc_buffer;

    pool->cached_data = false;
    pool->poolMgr = nullptr;
}

GstBufferPool *gst_video_capture_pool_new()
{
    GstBufferPool *pool = GST_BUFFER_POOL_CAST(g_object_new(
        GST_TYPE_VIDEO_CAPTURE_POOL, "name", "video_capture_pool", nullptr));
    (void)gst_object_ref_sink(pool);

    return pool;
}

static void gst_video_capture_pool_set_property(GObject *object, guint id, const GValue *value, GParamSpec *pspec)
{
    (void)pspec;
    GstVideoCapturePool *pool = GST_VIDEO_CAPTURE_POOL(object);
    g_return_if_fail(pool != nullptr && value != nullptr);

    g_mutex_lock(&pool->pool_lock);
    ON_SCOPE_EXIT(0) { g_mutex_unlock(&pool->pool_lock); };
    switch (id) {
        case PROP_cached_data:
            if (g_value_get_boolean(value) == true) {
                if (pool->poolMgr == nullptr) {
                    const uint32_t size = 6; // Save up to 6 buffer data.
                    pool->poolMgr = std::make_shared<OHOS::Media::VideoPoolManager>(size);
                    g_return_if_fail(pool->poolMgr != nullptr);
                }

                // If the queue is not empty, it is not supported to cache the suspended data again.
                if (pool->poolMgr->IsBufferQueEmpty()) {
                    pool->cached_data = true;
                }
            } else {
                pool->cached_data = false;
            }
            break;
        default:
            break;
    }
}

static void gst_video_capture_pool_get_property(GObject *object, guint id, GValue *value, GParamSpec *pspec)
{
    (void)pspec;
    GstVideoCapturePool *pool = GST_VIDEO_CAPTURE_POOL(object);
    g_return_if_fail(pool != nullptr && value != nullptr);

    g_mutex_lock(&pool->pool_lock);
    ON_SCOPE_EXIT(0) { g_mutex_unlock(&pool->pool_lock); };

    switch (id) {
        case PROP_BUFFER_COUNT:
            if (pool->poolMgr != nullptr) {
                g_value_set_uint(value, pool->poolMgr->GetBufferQueSize());
            }
            break;
        default:
            break;
    }
}

static GstFlowReturn gst_video_capture_pool_buffer_available(GstConsumerSurfacePool *surfacepool)
{
    g_return_val_if_fail(surfacepool != nullptr, GST_FLOW_ERROR);
    GstVideoCapturePool *videopool = GST_VIDEO_CAPTURE_POOL(surfacepool);

    g_mutex_lock(&videopool->pool_lock);
    ON_SCOPE_EXIT(0) { g_mutex_unlock(&videopool->pool_lock); };
    if (videopool->cached_data && videopool->poolMgr != nullptr &&
        videopool->poolMgr->IsBufferQueFull() == false) {
        GstBuffer *buf;
        if (gst_video_capture_pool_get_buffer(surfacepool, &buf) == GST_FLOW_OK) {
            (void)videopool->poolMgr->PushBuffer(buf);
            return GST_FLOW_OK;
        }
    }

    return GST_FLOW_ERROR;
}

static GstFlowReturn gst_video_capture_pool_alloc_buffer(GstBufferPool *pool, GstBuffer **buffer)
{
    GstVideoCapturePool *videopool = GST_VIDEO_CAPTURE_POOL(pool);
    g_return_val_if_fail(videopool != nullptr && buffer != nullptr, GST_FLOW_ERROR);
    
    g_mutex_lock(&videopool->pool_lock);
    ON_SCOPE_EXIT(0) { g_mutex_unlock(&videopool->pool_lock); };
    
    if (videopool->poolMgr != nullptr && videopool->poolMgr->GetBufferQueSize() > 0) {
        *buffer = videopool->poolMgr->PopBuffer();
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}

static GstFlowReturn gst_video_capture_pool_get_buffer(GstConsumerSurfacePool *surfacepool, GstBuffer **buffer)
{
    g_return_val_if_fail(surfacepool != nullptr && buffer != nullptr, GST_FLOW_ERROR);

    // Get surface buffer
    OHOS::sptr<OHOS::SurfaceBuffer> surface_buffer = nullptr;
    gint32 fencefd = -1;
    GstFlowReturn ret = gst_consumer_surface_pool_get_surface_buffer(surfacepool, surface_buffer, fencefd);
    g_return_val_if_fail(ret == GST_FLOW_OK, GST_FLOW_ERROR);    
    ON_SCOPE_EXIT(0) { gst_consumer_surface_pool_release_surface_buffer(surfacepool, surface_buffer, fencefd); };

    // Get surface buffer data.
    gint64 timestamp = 0;
    gint32 data_size = 0;
    gboolean end_of_stream = false;
    guint32 bufferFlag = 0;
    gint32 realsize = surface_buffer->GetSize();
    const OHOS::sptr<OHOS::BufferExtraData>& extraData = surface_buffer->GetExtraData();
    g_return_val_if_fail(extraData != nullptr, GST_FLOW_ERROR);    
    (void)extraData->ExtraGet("timeStamp", timestamp);
    (void)extraData->ExtraGet("dataSize", data_size);
    g_return_val_if_fail(realsize >= data_size, GST_FLOW_ERROR);
    (void)extraData->ExtraGet("endOfStream", end_of_stream);
    if (end_of_stream) {
        bufferFlag = BUFFER_FLAG_EOS;
    }

    // copy data
    GstBuffer *dts_buffer = gst_buffer_new_allocate(nullptr, data_size, nullptr);
    ON_SCOPE_EXIT(1) { gst_buffer_unref(dts_buffer); };
    g_return_val_if_fail(dts_buffer != nullptr, GST_FLOW_ERROR);

    GstMapInfo info = GST_MAP_INFO_INIT;
    g_return_val_if_fail(gst_buffer_map(dts_buffer, &info, GST_MAP_WRITE) == TRUE, GST_FLOW_ERROR);
    ON_SCOPE_EXIT(2) { gst_buffer_unmap(dts_buffer, &info); };

    uint8_t *src = static_cast<uint8_t *>(surface_buffer->GetVirAddr());
    errno_t rc = memcpy_s(info.data, info.size, src, static_cast<size_t>(data_size));
    if (rc != EOK) {
        GST_ERROR("memcpy failed");
        return GST_FLOW_ERROR;
    }

    // Meta 
    GstBufferTypeMeta *buffer_meta = NULL;
    buffer_meta = (GstBufferTypeMeta *)gst_buffer_add_meta(dts_buffer, GST_BUFFER_TYPE_META_INFO, NULL);
    g_return_val_if_fail(buffer_meta != NULL, GST_FLOW_ERROR);

    buffer_meta->type = BUFFER_TYPE_HANDLE;
    buffer_meta->bufLen = data_size;
    buffer_meta->length = data_size;
    buffer_meta->bufferFlag = bufferFlag;
    buffer_meta->pixelFormat = surface_buffer->GetFormat();
    buffer_meta->width = surface_buffer->GetWidth();
    buffer_meta->height = surface_buffer->GetHeight();

    GST_BUFFER_PTS(dts_buffer) = static_cast<uint64_t>(timestamp);
    GST_DEBUG_OBJECT(surfacepool, "BufferQue video capture buffer size is: %" G_GSIZE_FORMAT ", pts: %"
            G_GUINT64_FORMAT, gst_buffer_get_size(dts_buffer), timestamp);

    CANCEL_SCOPE_EXIT_GUARD(1);
    *buffer = dts_buffer;
    return GST_FLOW_OK;
}
