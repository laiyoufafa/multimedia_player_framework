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

#include "gst_shmem_pool.h"
#include "gst/video/gstvideometa.h"
#include "gst_shmem_allocator.h"
#include "media_log.h"
#include "media_errors.h"
#include "buffer_type_meta.h"
#include "avsharedmemorybase.h"

#define GST_BUFFER_POOL_LOCK(pool) (g_mutex_lock(&pool->lock))
#define GST_BUFFER_POOL_UNLOCK(pool) (g_mutex_unlock(&pool->lock))
#define GST_BUFFER_POOL_WAIT(pool) (g_cond_wait(&pool->cond, &pool->lock))
#define GST_BUFFER_POOL_NOTIFY(pool) (g_cond_signal(&pool->cond))

#define gst_shmem_pool_parent_class parent_class
G_DEFINE_TYPE (GstShMemPool, gst_shmem_pool, GST_TYPE_BUFFER_POOL);

static void gst_shmem_pool_finalize(GObject *obj);
static const gchar **gst_shmem_pool_get_options (GstBufferPool *pool);
static gboolean gst_shmem_pool_set_config(GstBufferPool *pool, GstStructure *config);
static gboolean gst_shmem_pool_start(GstBufferPool *pool);
static gboolean gst_shmem_pool_stop(GstBufferPool *pool);
static GstFlowReturn gst_shmem_pool_alloc_buffer(GstBufferPool *pool,
    GstBuffer **buffer, GstBufferPoolAcquireParams *params);
static void gst_shmem_pool_free_buffer(GstBufferPool *pool, GstBuffer *buffer);
static GstFlowReturn gst_shmem_pool_acquire_buffer(GstBufferPool *pool,
    GstBuffer **buffer, GstBufferPoolAcquireParams *params);
static void gst_shmem_pool_release_buffer(GstBufferPool *pool, GstBuffer *buffer);
static void gst_shmem_pool_flush_start(GstBufferPool *pool);

GST_DEBUG_CATEGORY_STATIC(gst_shmem_pool_debug_category);
#define GST_CAT_DEFAULT gst_shmem_pool_debug_category

static void gst_shmem_pool_class_init (GstShMemPoolClass *klass)
{
    g_return_if_fail(klass != nullptr);
    GstBufferPoolClass *poolClass = GST_BUFFER_POOL_CLASS (klass);
    GObjectClass *gobjectClass = G_OBJECT_CLASS(klass);

    gobjectClass->finalize = gst_shmem_pool_finalize;
    poolClass->get_options = gst_shmem_pool_get_options;
    poolClass->set_config = gst_shmem_pool_set_config;
    poolClass->start = gst_shmem_pool_start;
    poolClass->stop = gst_shmem_pool_stop;
    poolClass->acquire_buffer = gst_shmem_pool_acquire_buffer;
    poolClass->alloc_buffer = gst_shmem_pool_alloc_buffer;
    poolClass->release_buffer = gst_shmem_pool_release_buffer;
    poolClass->free_buffer = gst_shmem_pool_free_buffer;
    poolClass->flush_start = gst_shmem_pool_flush_start;

    GST_DEBUG_CATEGORY_INIT(gst_shmem_pool_debug_category, "shmempool", 0, "shmempool class");
}

static void gst_shmem_pool_init (GstShMemPool *pool)
{
    g_return_if_fail(pool != nullptr);

    pool->started = FALSE;
    g_mutex_init(&pool->lock);
    g_cond_init(&pool->cond);
    pool->allocator = nullptr;
    pool->minBuffers = 0;
    pool->maxBuffers = 0;
    pool->avshmempool = nullptr;
    pool->addVideoMeta = FALSE;
    gst_video_info_init(&pool->info);
    pool->size = 0;
    pool->freeBufCnt = 0;
    pool->end = FALSE;
    gst_allocation_params_init(&pool->params);

    GST_DEBUG_OBJECT(pool, "init pool");
}

static void gst_shmem_pool_finalize(GObject *obj)
{
    g_return_if_fail(obj != nullptr);
    GstShMemPool *spool = GST_SHMEM_POOL_CAST(obj);
    g_return_if_fail(spool != nullptr);

    gst_object_unref(spool->allocator);
    spool->allocator = nullptr;
    g_mutex_clear(&spool->lock);
    g_cond_clear(&spool->cond);
    if (spool->avshmempool != nullptr) {
        spool->avshmempool->Reset();
        spool->avshmempool = nullptr;
    }
    spool->end = TRUE;

    GST_DEBUG_OBJECT(spool, "finalize pool");
    G_OBJECT_CLASS(parent_class)->finalize(obj);
}

GstShMemPool *gst_shmem_pool_new()
{
    GstShMemPool *pool = GST_SHMEM_POOL_CAST(g_object_new(
        GST_TYPE_SHMEM_POOL, "name", "ShMemPool", nullptr));
    (void)gst_object_ref_sink(pool);

    return pool;
}

static const gchar **gst_shmem_pool_get_options (GstBufferPool *pool)
{
    // add buffer type meta option at here
    static const gchar *options[] = { GST_BUFFER_POOL_OPTION_VIDEO_META, nullptr };
    return options;
}

gboolean gst_shmem_pool_set_avshmempool(GstShMemPool *pool,
                                        std::shared_ptr<OHOS::Media::AVSharedMemoryPool> avshmempool)
{
    g_return_val_if_fail(pool != nullptr && avshmempool != nullptr, FALSE);

    GST_BUFFER_POOL_LOCK(pool);

    if (pool->started) {
        GST_ERROR("started already, reject to set avshmempool");
        GST_OBJECT_UNLOCK(pool);
        return FALSE;
    }

    if (pool->avshmempool != nullptr) {
        GST_ERROR("avshmempool has already been set");
        GST_OBJECT_UNLOCK(pool);
        return FALSE;
    }
    pool->avshmempool = avshmempool;

    GST_BUFFER_POOL_UNLOCK(pool);
    return TRUE;
}

static gboolean parse_caps_for_raw_video(GstShMemPool *spool, GstCaps *caps, guint *size)
{
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    g_return_val_if_fail(structure != nullptr, FALSE);
    if (gst_structure_has_name(structure, "video/x-raw")) {
        GstVideoInfo info;
        gboolean ret = gst_video_info_from_caps(&info, caps);
        g_return_val_if_fail(ret, FALSE);
        *size = info.size;
        GST_INFO("this is video raw scene");
        spool->info = info;
        spool->addVideoMeta = TRUE;
        return TRUE;
    }

    return FALSE;
}

static gboolean gst_shmem_pool_set_config(GstBufferPool *pool, GstStructure *config)
{
    GstShMemPool *spool = GST_SHMEM_POOL_CAST(pool);
    g_return_val_if_fail(spool != nullptr, FALSE);
    g_return_val_if_fail(config != nullptr, FALSE);

    GstCaps *caps = nullptr;
    guint size;
    guint minBuffers;
    guint maxBuffers;
    static const guint DEFAULT_MAX_BUFFERS = 10;
    if (!gst_buffer_pool_config_get_params(config, &caps, &size, &minBuffers, &maxBuffers)) {
        GST_ERROR("wrong config");
        return FALSE;
    }
    g_return_val_if_fail(minBuffers <= maxBuffers, FALSE);

    GST_BUFFER_POOL_LOCK(spool);

    spool->minBuffers = minBuffers;
    spool->maxBuffers = maxBuffers > 0 ? maxBuffers : DEFAULT_MAX_BUFFERS;
    parse_caps_for_raw_video(spool, caps, &size);
    spool->size = size;

    GstAllocator *allocator = nullptr;
    GstAllocationParams params;
    (void)gst_buffer_pool_config_get_allocator(config, &allocator, &params);
    if (allocator == nullptr) {
        GST_WARNING_OBJECT(pool, "allocator is null");
    }
    if (!(allocator != nullptr && GST_IS_SHMEM_ALLOCATOR(allocator))) {
        GST_WARNING_OBJECT(pool, "no valid allocator in pool");
        return FALSE;
    }

    GST_INFO("set config, size: %u, min_bufs: %u, max_bufs: %u", size, minBuffers, maxBuffers);

    spool->allocator = GST_SHMEM_ALLOCATOR_CAST(gst_object_ref(allocator));
    spool->params = params;

    GST_BUFFER_POOL_UNLOCK(spool);
    return TRUE;
}

static gboolean gst_shmem_pool_start(GstBufferPool *pool)
{
    g_return_val_if_fail(pool != nullptr, FALSE);
    GstShMemPool *spool = GST_SHMEM_POOL_CAST(pool);
    g_return_val_if_fail(spool != nullptr, FALSE);

    GST_DEBUG("pool start");

    GST_BUFFER_POOL_LOCK(spool);
    if (spool->avshmempool == nullptr) {
        GST_BUFFER_POOL_UNLOCK(spool);
        GST_ERROR("not set avshmempool");
        return FALSE;
    }

    // clear the configuration carried in the avshmempool
    spool->avshmempool->Reset();

    static const uint32_t alignBytes = 4;
    gsize alignedPrefix = (spool->params.prefix + alignBytes - 1) & ~(alignBytes - 1);
    OHOS::Media::AVSharedMemoryPool::InitializeOption option = {
        .preAllocMemCnt = spool->minBuffers,
        .memSize = spool->size + alignedPrefix,
        .maxMemCnt = spool->maxBuffers,
        .enableRemoteRefCnt = false
    };

    int32_t ret = spool->avshmempool->Init(option);
    if (ret != OHOS::Media::MSERR_OK) {
        GST_BUFFER_POOL_UNLOCK(spool);
        GST_ERROR("init avshmempool failed, ret = %d", ret);
        return FALSE;
    }

    gst_shmem_allocator_set_pool(spool->allocator, spool->avshmempool);

    spool->started = TRUE;
    spool->freeBufCnt = spool->maxBuffers;
    GST_BUFFER_POOL_UNLOCK(spool);
    return TRUE;
}

static gboolean gst_shmem_pool_stop(GstBufferPool *pool)
{
    g_return_val_if_fail(pool != nullptr, FALSE);
    GstShMemPool *spool = GST_SHMEM_POOL_CAST(pool);
    if (spool->end) {
        return TRUE;
    }
    g_return_val_if_fail(spool != nullptr, FALSE);

    GST_DEBUG("pool stop");
    GST_BUFFER_POOL_LOCK(spool);
    spool->started = FALSE;
    if (spool->avshmempool != nullptr) {
        spool->avshmempool->Reset();
    }
    GST_BUFFER_POOL_NOTIFY(spool); // wakeup immediately
    gboolean ret = (spool->freeBufCnt == spool->maxBuffers);
    GST_INFO("stop pool, freeBufCnt: %u, maxBuffers: %u", spool->freeBufCnt, spool->maxBuffers);

    // leave all configuration unchanged.
    GST_BUFFER_POOL_UNLOCK(spool);
    return ret;
}

static void gst_shmem_pool_flush_start(GstBufferPool *pool)
{
    g_return_if_fail(pool != nullptr);
    GstShMemPool *spool = GST_SHMEM_POOL_CAST(pool);

    GST_DEBUG("pool flush");
    GST_BUFFER_POOL_LOCK(spool);
    GST_BUFFER_POOL_NOTIFY(spool);
    GST_BUFFER_POOL_UNLOCK(spool);
}

static GstFlowReturn do_alloc_memory_locked(GstShMemPool *spool,
    GstBufferPoolAcquireParams *params, GstShMemMemory **memory)
{
    GstBufferPool *pool = GST_BUFFER_POOL_CAST(spool);
    *memory = nullptr;
    GstFlowReturn ret = GST_FLOW_OK;

    while (TRUE) {
        // when pool is set flusing or set inactive, the flusing state is true, refer to GstBufferPool
        if (GST_BUFFER_POOL_IS_FLUSHING(pool)) {
            ret = GST_FLOW_FLUSHING;
            GST_INFO("pool is flushing");
            break;
        }

        /**
         * If the freeBufCnt is 0, it means the GstBuffer is not release back to GstPool, we can not
         * allow to acquire buffer from AVShMemPool to avoiding that two different GstBuffer hold
         * the same AVSharedMemory.
         */
        if (spool->freeBufCnt > 0) {
            if (spool->allocator == nullptr) {
                GST_ERROR("Allocator is null");
            }
            GstMemory *mem = gst_allocator_alloc(GST_ALLOCATOR_CAST(spool->allocator), spool->size, &spool->params);
            if (mem != nullptr) {
                *memory = reinterpret_cast<GstShMemMemory *>(mem);
            } else {
                ret = GST_FLOW_ERROR;
                GST_ERROR("alloc memory failed when the freeBufCnt is not zero");
            }
            break;
        }
        if ((params != nullptr) && (params->flags & GST_BUFFER_POOL_ACQUIRE_FLAG_DONTWAIT)) {
            ret = GST_FLOW_EOS;
            break;
        }

        if (!GST_BUFFER_POOL_IS_FLUSHING(pool)) {
            GST_DEBUG("before sleep currtime: %" G_GINT64_FORMAT, g_get_monotonic_time());
            GST_BUFFER_POOL_WAIT(spool);
            GST_DEBUG("after sleep currtime: %" G_GINT64_FORMAT, g_get_monotonic_time());
        }
    }

    return ret;
}

static GstFlowReturn gst_shmem_pool_alloc_buffer(GstBufferPool *pool,
    GstBuffer **buffer, GstBufferPoolAcquireParams *params)
{
    g_return_val_if_fail(pool != nullptr && buffer != nullptr, GST_FLOW_ERROR);
    GstShMemPool *spool = GST_SHMEM_POOL_CAST(pool);
    g_return_val_if_fail(spool != nullptr, GST_FLOW_ERROR);

    GstShMemMemory *memory = nullptr;
    GstFlowReturn ret = do_alloc_memory_locked(spool, params, &memory);
    if (memory == nullptr) {
        GST_DEBUG("no memory");
        return ret;
    }

    *buffer = gst_buffer_new();
    if (*buffer == nullptr) {
        GST_ERROR("alloc gst buffer failed");
        gst_allocator_free(GST_ALLOCATOR_CAST(spool->allocator), reinterpret_cast<GstMemory *>(memory));
        return GST_FLOW_ERROR;
    }

    gst_buffer_append_memory(*buffer, reinterpret_cast<GstMemory *>(memory));

    if (spool->addVideoMeta) { // don't need the mutex
        GstVideoInfo *info = &spool->info;
        gst_buffer_add_video_meta(*buffer, GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_INFO_FORMAT(info),
            GST_VIDEO_INFO_WIDTH(info), GST_VIDEO_INFO_HEIGHT(info));
    }

    // add buffer type meta at here.
    std::shared_ptr<OHOS::Media::AVSharedMemory> mem = memory->mem;
    std::shared_ptr<OHOS::Media::AVSharedMemoryBase> baseMem =
            std::static_pointer_cast<OHOS::Media::AVSharedMemoryBase>(mem);
    if (baseMem == nullptr)  {
        GST_ERROR("invalid pointer");
        return GST_FLOW_ERROR;
    }

    int32_t fd = baseMem->GetFd();
    int32_t size = baseMem->GetSize();
    AVShmemFlags flag = FLAGS_READ_WRITE;
    if (baseMem->GetFlags() == OHOS::Media::AVSharedMemory::FLAGS_READ_ONLY) {
        flag = FLAGS_READ_ONLY;
    }
    GstBufferFdConfig config;
    config.offset = 0;
    config.length = size;
    config.totalSize = size;
    config.memFlag = flag;
    config.bufferFlag = 0;
    gst_buffer_add_buffer_fd_meta(*buffer, fd, config);

    GST_DEBUG("alloc buffer ok, 0x%06" PRIXPTR "", FAKE_POINTER(*buffer));
    return GST_FLOW_OK;
}

static void gst_shmem_pool_free_buffer(GstBufferPool *pool, GstBuffer *buffer)
{
    (void)pool;
    g_return_if_fail(buffer != nullptr);
    gst_buffer_unref(buffer);
}

static GstFlowReturn gst_shmem_pool_acquire_buffer(GstBufferPool *pool,
    GstBuffer **buffer, GstBufferPoolAcquireParams *params)
{
    g_return_val_if_fail(pool != nullptr && buffer != nullptr, GST_FLOW_ERROR);
    GstShMemPool *spool = GST_SHMEM_POOL_CAST(pool);
    g_return_val_if_fail(spool != nullptr, GST_FLOW_ERROR);
    GST_DEBUG("acquire buffer from pool 0x%06" PRIXPTR, FAKE_POINTER(pool));

    GST_BUFFER_POOL_LOCK(spool);
    // Rather than keeping a idlelist for surface buffer by ourself, we acquire buffer
    // from the AVShMemPool directly.
    GstFlowReturn ret = gst_shmem_pool_alloc_buffer(pool, buffer, params);
    if (ret == GST_FLOW_OK) {
        spool->freeBufCnt -= 1;
    }

    if (ret == GST_FLOW_EOS) {
        GST_DEBUG("no more buffers");
    }

    // The GstBufferPool will add the GstBuffer's pool ref to this pool.
    GST_BUFFER_POOL_UNLOCK(spool);
    return ret;
}

static void gst_shmem_pool_release_buffer(GstBufferPool *pool, GstBuffer *buffer)
{
    // The GstBufferPool has already cleared the GstBuffer's pool ref to this pool.

    GstShMemPool *spool = GST_SHMEM_POOL_CAST(pool);
    g_return_if_fail(spool != nullptr);
    GST_DEBUG("release buffer 0x%06" PRIXPTR " to pool 0x%06" PRIXPTR, FAKE_POINTER(buffer), FAKE_POINTER(pool));

    if (G_UNLIKELY(!gst_buffer_is_all_memory_writable(buffer))) {
        GST_WARNING("buffer is not writable, 0x%06" PRIXPTR, FAKE_POINTER(buffer));
        gst_shmem_pool_free_buffer(pool, buffer);
        // not add the freeBufCnt, we wait the internal avshmemory released back to the internal avshmmempool
        return;
    }

    // we dont queue the buffer to the idlelist. the memory rotation reuse feature
    // provided by the AVShMemPool itself.
    gst_shmem_pool_free_buffer(pool, buffer);

    GST_BUFFER_POOL_LOCK(spool);
    spool->freeBufCnt += 1;
    GST_BUFFER_POOL_NOTIFY(spool);
    GST_BUFFER_POOL_UNLOCK(spool);
}
