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

#include "config.h"
#include "gst_surface_video_src.h"
#include <gst/gst.h>
#include "media_errors.h"
#include "video_capture_factory.h"
#include "display_type.h"

static GstStaticPadTemplate gst_video_src_template =
GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

namespace {
    constexpr VideoStreamType DEFAULT_STREAM_TYPE = VIDEO_STREAM_TYPE_UNKNOWN;
    constexpr gint DEFAULT_FRAME_RATE = 30;
}

enum {
    PROP_0,
    PROP_STREAM_TYPE,
    PROP_SURFACE_WIDTH,
    PROP_SURFACE_HEIGHT,
    PROP_SURFACE,
    PROP_FRAME_RATE,
};

using namespace OHOS::Media;

#define gst_surface_video_src_parent_class parent_class
G_DEFINE_TYPE(GstSurfaceVideoSrc, gst_surface_video_src, GST_TYPE_PUSH_SRC);

static void gst_surface_video_src_finalize(GObject *object);
static void gst_surface_video_src_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void gst_surface_video_src_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static GstStateChangeReturn gst_surface_video_src_change_state(GstElement *element, GstStateChange transition);
static GstFlowReturn gst_surface_video_src_create(GstPushSrc *psrc, GstBuffer **outbuf);
static void gst_surface_video_src_set_stream_type(GstSurfaceVideoSrc *src, gint stream_type);
static gboolean gst_surface_video_src_negotiate(GstBaseSrc *basesrc);
static gboolean gst_surface_video_src_is_seekable(GstBaseSrc *basesrc);
static gboolean gst_surface_video_src_send_event(GstElement *element, GstEvent *event);

#define GST_TYPE_SURFACE_VIDEO_SRC_STREAM_TYPE (gst_surface_video_src_stream_type_get_type())
static GType gst_surface_video_src_stream_type_get_type(void)
{
    static GType surface_video_src_stream_type = 0;
    static const GEnumValue stream_types[] = {
        {VIDEO_STREAM_TYPE_UNKNOWN, "UNKNOWN", "UNKNOWN"},
        {VIDEO_STREAM_TYPE_ES_AVC, "ES_AVC", "ES_AVC"},
        {VIDEO_STREAM_TYPE_YUV_420, "YUV_420", "YUV_420"},
        {0, nullptr, nullptr}
    };
    if (!surface_video_src_stream_type) {
        surface_video_src_stream_type = g_enum_register_static("VideoStreamType", stream_types);
    }
    return surface_video_src_stream_type;
}

static void gst_surface_video_src_class_init(GstSurfaceVideoSrcClass *klass)
{
    g_return_if_fail(klass != nullptr);
    GObjectClass *gobject_class = reinterpret_cast<GObjectClass *>(klass);
    GstElementClass *gstelement_class = reinterpret_cast<GstElementClass *>(klass);
    GstBaseSrcClass *gstbasesrc_class = reinterpret_cast<GstBaseSrcClass *>(klass);
    GstPushSrcClass *gstpushsrc_class = reinterpret_cast<GstPushSrcClass *>(klass);

    gobject_class->finalize = gst_surface_video_src_finalize;
    gobject_class->set_property = gst_surface_video_src_set_property;
    gobject_class->get_property = gst_surface_video_src_get_property;

    g_object_class_install_property(gobject_class, PROP_STREAM_TYPE,
        g_param_spec_enum("stream-type", "Stream type",
            "Stream type", GST_TYPE_SURFACE_VIDEO_SRC_STREAM_TYPE, DEFAULT_STREAM_TYPE,
            (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(gobject_class, PROP_SURFACE_WIDTH,
        g_param_spec_uint("surface-width", "Surface width",
            "Surface width", 0, G_MAXUINT32, 0,
            (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(gobject_class, PROP_SURFACE_HEIGHT,
        g_param_spec_uint("surface-height", "Surface height",
            "Surface width", 0, G_MAXUINT32, 0,
            (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(gobject_class, PROP_SURFACE,
        g_param_spec_pointer("surface", "Surface", "Surface for recording",
            (GParamFlags)(G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(gobject_class, PROP_FRAME_RATE,
        g_param_spec_uint("frame-rate", "Frame rate",
            "recorder frame rate", 0, G_MAXUINT32, DEFAULT_FRAME_RATE,
            (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    gst_element_class_set_static_metadata(gstelement_class,
        "Surface video source", "Source/Video",
        "Retrieve video frame from surface buffer queue", "OpenHarmony");

    gst_element_class_add_static_pad_template(gstelement_class, &gst_video_src_template);

    gstelement_class->change_state = gst_surface_video_src_change_state;
    gstelement_class->send_event = gst_surface_video_src_send_event;
    gstbasesrc_class->negotiate = gst_surface_video_src_negotiate;
    gstbasesrc_class->is_seekable = gst_surface_video_src_is_seekable;
    gstpushsrc_class->create = gst_surface_video_src_create;
}

static void gst_surface_video_src_init(GstSurfaceVideoSrc *src)
{
    g_return_if_fail(src != nullptr);
    gst_base_src_set_format(GST_BASE_SRC(src), GST_FORMAT_TIME);
    gst_base_src_set_live(GST_BASE_SRC(src), TRUE);
    src->stream_type = VIDEO_STREAM_TYPE_UNKNOWN;
    src->capture = nullptr;
    src->src_caps = nullptr;
    src->video_width = 0;
    src->video_height = 0;
    src->video_frame_rate = DEFAULT_FRAME_RATE;
    src->is_start = FALSE;
    src->need_codec_data = TRUE;
    src->is_eos = FALSE;
    src->is_flushing = FALSE;
    src->reset_caps = TRUE;
}

static void gst_surface_video_src_finalize(GObject *object)
{
    g_return_if_fail(object != nullptr);
    GstSurfaceVideoSrc *src = GST_SURFACE_VIDEO_SRC(object);
    g_return_if_fail(src != nullptr);

    src->need_codec_data = FALSE;

    if (src->src_caps != nullptr) {
        gst_caps_unref(src->src_caps);
        src->src_caps = nullptr;
    }
}

static void gst_surface_video_src_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
    (void)pspec;
    g_return_if_fail(object != nullptr);
    g_return_if_fail(value != nullptr);
    GstSurfaceVideoSrc *src = GST_SURFACE_VIDEO_SRC(object);
    g_return_if_fail(src != nullptr);
    switch (prop_id) {
        case PROP_STREAM_TYPE:
            gst_surface_video_src_set_stream_type(src, g_value_get_enum(value));
            break;
        case PROP_SURFACE_WIDTH:
            src->video_width = g_value_get_uint(value);
            break;
        case PROP_SURFACE_HEIGHT:
            src->video_height = g_value_get_uint(value);
            break;
        case PROP_FRAME_RATE:
            src->video_frame_rate = g_value_get_uint(value);
            break;
        default:
            break;
    }
}

static void gst_surface_video_src_set_stream_type(GstSurfaceVideoSrc *src, gint stream_type)
{
    switch (stream_type) {
        case VideoStreamType::VIDEO_STREAM_TYPE_ES_AVC:
            src->stream_type = VIDEO_STREAM_TYPE_ES_AVC;
            src->need_codec_data = TRUE;
            break;
        case VideoStreamType::VIDEO_STREAM_TYPE_YUV_420:
            src->stream_type = VIDEO_STREAM_TYPE_YUV_420;
            src->need_codec_data = FALSE;
            break;
        default:
            return;
    }
    g_return_if_fail(src != nullptr);
}

static void gst_surface_video_src_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    g_return_if_fail(object != nullptr);
    g_return_if_fail(value != nullptr);
    GstSurfaceVideoSrc *src = GST_SURFACE_VIDEO_SRC(object);
    g_return_if_fail(src != nullptr);
    (void)pspec;
    switch (prop_id) {
        case PROP_STREAM_TYPE:
            g_value_set_enum(value, src->stream_type);
            break;
        case PROP_SURFACE_WIDTH:
            g_value_set_uint(value, src->video_width);
            break;
        case PROP_SURFACE_HEIGHT:
            g_value_set_uint(value, src->video_height);
            break;
        case PROP_FRAME_RATE:
            g_value_set_uint(value, src->video_frame_rate);
            break;
        case PROP_SURFACE:
            g_return_if_fail(src->capture != nullptr);
            g_value_set_pointer(value, src->capture->GetSurface().GetRefPtr());
            break;
        default:
            break;
    }
}

static gboolean process_codec_data(GstSurfaceVideoSrc *src)
{
    g_return_val_if_fail(src != nullptr, FALSE);
    g_return_val_if_fail(src->capture != nullptr, FALSE);

    std::shared_ptr<EsAvcCodecBuffer> codec_buffer = src->capture->GetCodecBuffer();
    g_return_val_if_fail(codec_buffer != nullptr, FALSE);

    if (codec_buffer->gstCodecBuffer == nullptr) {
        return FALSE;
    }

    if (src->src_caps != nullptr) {
        gst_caps_unref(src->src_caps);
    }

    src->src_caps = gst_caps_new_simple("video/x-h264",
        "width", G_TYPE_INT, codec_buffer->width,
        "height", G_TYPE_INT, codec_buffer->height,
        "framerate", GST_TYPE_FRACTION, src->video_frame_rate, 1,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
        "level", G_TYPE_STRING, "2",
        "profile", G_TYPE_STRING, "high",
        "alignment", G_TYPE_STRING, "au",
        "stream-format", G_TYPE_STRING, "avc", nullptr);
    gst_caps_set_simple(src->src_caps, "codec_data", GST_TYPE_BUFFER, codec_buffer->gstCodecBuffer, nullptr);
    GstBaseSrc *basesrc = GST_BASE_SRC_CAST(src);
    basesrc->segment.start = codec_buffer->segmentStart;
    gst_buffer_unref(codec_buffer->gstCodecBuffer);
    codec_buffer->gstCodecBuffer = nullptr;
    return TRUE;
}

static gboolean set_fix_caps(GstSurfaceVideoSrc *src)
{
    g_return_val_if_fail(src != nullptr, FALSE);
    g_return_val_if_fail(src->capture != nullptr, FALSE);

    if (src->src_caps != nullptr) {
        gst_caps_unref(src->src_caps);
    }

    src->src_caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "I420",
        "framerate", GST_TYPE_FRACTION, src->video_frame_rate, 1,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
        "width", G_TYPE_INT, src->video_width,
        "height", G_TYPE_INT, src->video_height,
        nullptr);

    return TRUE;
}

static gboolean start_video_capture(GstSurfaceVideoSrc *src)
{
    g_return_val_if_fail(src != nullptr, FALSE);
    g_return_val_if_fail(src->capture != nullptr, FALSE);

    g_return_val_if_fail(src->capture->Start() == MSERR_OK, FALSE);
    if (src->need_codec_data) {
        gboolean ret = process_codec_data(src);
        return ret;
    }
    return TRUE;
}

static GstStateChangeReturn gst_state_change_forward_direction(GstSurfaceVideoSrc *src, GstStateChange transition)
{
    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
            src->capture = OHOS::Media::VideoCaptureFactory::CreateVideoCapture(src->stream_type);
            g_return_val_if_fail(src->capture != nullptr, GST_STATE_CHANGE_FAILURE);
            if (src->need_codec_data != TRUE) {
                g_return_val_if_fail(set_fix_caps(src) == TRUE, GST_STATE_CHANGE_FAILURE);
            }
            break;
        case GST_STATE_CHANGE_READY_TO_PAUSED:
            g_return_val_if_fail(src->capture != nullptr, GST_STATE_CHANGE_FAILURE);
            g_return_val_if_fail(src->capture->SetVideoHeight(src->video_height) == MSERR_OK,
                GST_STATE_CHANGE_FAILURE);
            g_return_val_if_fail(src->capture->SetVideoWidth(src->video_width) == MSERR_OK,
                GST_STATE_CHANGE_FAILURE);
            g_return_val_if_fail(src->capture->SetFrameRate(src->video_frame_rate) == MSERR_OK,
                GST_STATE_CHANGE_FAILURE);
            g_return_val_if_fail(src->capture->SetStreamType(src->stream_type) == MSERR_OK,
                GST_STATE_CHANGE_FAILURE);
            g_return_val_if_fail(src->capture->Prepare() == MSERR_OK, GST_STATE_CHANGE_FAILURE);
            break;
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
            g_return_val_if_fail(src->capture != nullptr, GST_STATE_CHANGE_FAILURE);
            if (src->is_start == FALSE) {
                g_return_val_if_fail(start_video_capture(src) == TRUE, GST_STATE_CHANGE_FAILURE);
                src->is_start = TRUE;
            } else {
                g_return_val_if_fail(src->capture->Resume() == MSERR_OK, GST_STATE_CHANGE_FAILURE);
            }
            break;
        default:
            break;
    }
    return GST_STATE_CHANGE_SUCCESS;
}

static GstStateChangeReturn gst_surface_video_src_change_state(GstElement *element, GstStateChange transition)
{
    g_return_val_if_fail(element != nullptr, GST_STATE_CHANGE_FAILURE);
    GstSurfaceVideoSrc *src = GST_SURFACE_VIDEO_SRC(element);
    g_return_val_if_fail(src != nullptr, GST_STATE_CHANGE_FAILURE);

    GstStateChangeReturn ret = gst_state_change_forward_direction(src, transition);
    g_return_val_if_fail(ret == GST_STATE_CHANGE_SUCCESS, GST_STATE_CHANGE_FAILURE);

    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);

    switch (transition) {
        case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
            g_return_val_if_fail(src->capture->Pause() == MSERR_OK, GST_STATE_CHANGE_FAILURE);
            break;
        case GST_STATE_CHANGE_PAUSED_TO_READY:
            src->is_start = FALSE;
            g_return_val_if_fail(src->capture != nullptr, GST_STATE_CHANGE_FAILURE);
            g_return_val_if_fail(src->capture->Stop() == MSERR_OK, GST_STATE_CHANGE_FAILURE);
            break;
        case GST_STATE_CHANGE_READY_TO_NULL:
            src->capture = nullptr;
            break;
        default:
            break;
    }

    return ret;
}

static gboolean gst_surface_video_src_is_seekable(GstBaseSrc *basesrc)
{
    (void)basesrc;
    return FALSE;
}

static gboolean gst_surface_video_src_negotiate(GstBaseSrc *basesrc)
{
    g_return_val_if_fail(basesrc != nullptr, FALSE);
    GstSurfaceVideoSrc *src = GST_SURFACE_VIDEO_SRC(basesrc);
    g_return_val_if_fail(src != nullptr, FALSE);

    // no need to wait playing when yuv source
    if (src->need_codec_data) {
        (void)gst_base_src_wait_playing(basesrc);
    }
    g_return_val_if_fail(src->src_caps != nullptr, FALSE);
    return gst_base_src_set_caps(basesrc, src->src_caps);
}

static gboolean reset_src_caps(GstSurfaceVideoSrc *src, uint32_t pixelFormat)
{
    g_return_val_if_fail(src != nullptr, FALSE);

    if (src->src_caps != nullptr) {
        gst_caps_unref(src->src_caps);
    }

    std::string format = "NV21";

    switch (pixelFormat) {
        case PIXEL_FMT_YCRCB_420_SP:
            GST_INFO("input pixel foramt is nv21");
            format = "NV21";
            break;
        case PIXEL_FMT_YCBCR_420_P:
            GST_INFO("input pixel foramt is I420");
            format = "I420";
            return TRUE;
        case PIXEL_FMT_YCBCR_420_SP:
            GST_INFO("input pixel foramt is nv12");
            format = "NV12";
            break;
        default:
            break;
    }

    src->src_caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, format.c_str(),
        "framerate", GST_TYPE_FRACTION, src->video_frame_rate, 1,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
        "width", G_TYPE_INT, src->video_width,
        "height", G_TYPE_INT, src->video_height,
        nullptr);

    gst_base_src_set_caps(GST_BASE_SRC(src), src->src_caps);
    return TRUE;
}

static GstFlowReturn gst_surface_video_src_create(GstPushSrc *psrc, GstBuffer **outbuf)
{
    g_return_val_if_fail(psrc != nullptr, GST_FLOW_ERROR);
    g_return_val_if_fail(outbuf != nullptr, GST_FLOW_ERROR);
    GstSurfaceVideoSrc *src = GST_SURFACE_VIDEO_SRC(psrc);
    g_return_val_if_fail(src != nullptr, GST_FLOW_ERROR);

    GST_DEBUG_OBJECT(src, "begin create...");

    if (src->is_start == FALSE) {
        return GST_FLOW_EOS;
    }
    g_return_val_if_fail(src->capture != nullptr, GST_FLOW_ERROR);

    std::shared_ptr<VideoFrameBuffer> frame_buffer = src->capture->GetFrameBuffer();
    if (src->is_eos) {
        GST_INFO_OBJECT(src, "eos...");
        return GST_FLOW_EOS;
    }
    if (src->is_flushing) {
        GST_INFO_OBJECT(src, "flushing...");
        return GST_FLOW_FLUSHING;
    }
    g_return_val_if_fail(frame_buffer != nullptr, GST_FLOW_ERROR);

    if (src->reset_caps && !src->need_codec_data) {
        (void)reset_src_caps(src, frame_buffer->pixelFormat);
        src->reset_caps = FALSE;
    }

    gst_base_src_set_blocksize(GST_BASE_SRC_CAST(src), static_cast<guint>(frame_buffer->size));

    *outbuf = frame_buffer->gstBuffer;
    GST_BUFFER_PTS(*outbuf) = frame_buffer->timeStamp;

    GST_DEBUG_OBJECT(src, "end create...");
    return GST_FLOW_OK;
}

static gboolean gst_surface_video_src_send_event(GstElement *element, GstEvent *event)
{
    g_return_val_if_fail(element != nullptr, FALSE);
    GstSurfaceVideoSrc *src = GST_SURFACE_VIDEO_SRC(element);
    g_return_val_if_fail(src != nullptr, FALSE);

    switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_FLUSH_START:
            g_return_val_if_fail(src->capture != nullptr, FALSE);
            src->is_eos = FALSE;
            src->is_flushing = TRUE;
            src->capture->UnLock(true);
            break;
        case GST_EVENT_FLUSH_STOP:
            g_return_val_if_fail(src->capture != nullptr, FALSE);
            src->is_flushing = FALSE;
            src->capture->UnLock(false);
            break;
        case GST_EVENT_EOS:
            g_return_val_if_fail(src->capture != nullptr, FALSE);
            src->is_eos = TRUE;
            src->capture->UnLock(true);
            break;
        default:
            break;
    }

    return GST_ELEMENT_CLASS(parent_class)->send_event(element, event);
}

static gboolean plugin_init(GstPlugin *plugin)
{
    g_return_val_if_fail(plugin != nullptr, FALSE);
    return gst_element_register(plugin, "surfacevideosrc", GST_RANK_PRIMARY, GST_TYPE_SURFACE_VIDEO_SRC);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    _surface_video_src,
    "GStreamer Surface Video Source",
    plugin_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
