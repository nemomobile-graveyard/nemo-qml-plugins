/*
 * Copyright (C) 2012 Jolla Ltd
 * Contact: Andrew den Exter <andrew.den.exter@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */


#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <QImage>
#include <QMutex>
#include <QWaitCondition>

#include <QtDebug>

namespace {

struct Thumbnailer
{
    Thumbnailer()
        : pipeline(0)
        , decodebin(0)
        , transform(0)
        , appsink(0)
        , prerollAvailable(false)
    {
    }

    ~Thumbnailer()
    {
        if (pipeline) {
            gst_element_set_state(pipeline, GST_STATE_NULL);
            gst_object_unref(GST_OBJECT(pipeline));
        } else {
            if (decodebin)
                gst_object_unref(GST_OBJECT(decodebin));
            if (transform)
                gst_object_unref(GST_OBJECT(transform));
            if (appsink)
                gst_object_unref(GST_OBJECT(appsink));
        }
    }

    QMutex mutex;
    QWaitCondition condition;

    GstElement *pipeline;
    GstElement *decodebin;
    GstElement *transform;
    GstElement *appsink;
    bool prerollAvailable;
};

static gboolean decodebin_autoplug_continue(GstElement *, GstPad *, GstCaps *caps, gpointer)
{
    // Short cut audio streams as soon as possible so they're not decoded.
    return qstrncmp(gst_structure_get_name(gst_caps_get_structure(caps, 0)), "audio/", 6) != 0;
}

static void decodebin_new_pad(GstElement *element, GstPad *pad, gpointer data)
{
    Q_UNUSED(element);
    Thumbnailer *thumbnailer = static_cast<Thumbnailer *>(data);

    GstCaps *caps = gst_pad_get_caps(pad);
    GstStructure *structure = gst_caps_get_structure(caps, 0);

    GstPad *sinkPad = gst_element_get_static_pad(thumbnailer->transform, "sink");

    bool isFakeSink = false;
    if (gst_pad_is_linked(sinkPad) || qstrncmp(gst_structure_get_name(structure), "video/x-raw-", 12) != 0) {
        // Create a fake sink for any non video streams so they don't stall the pipeline.
        GstElement *sink = gst_element_factory_make("fakesink", NULL);
        sinkPad = gst_element_get_static_pad(sink, "sink");

        gst_bin_add(GST_BIN(thumbnailer->pipeline), sink);
        gst_element_set_state(sink, GST_STATE_PAUSED);

        isFakeSink = true;
    }

    for (;;) {
        switch (gst_pad_link(pad, sinkPad)) {
        case GST_PAD_LINK_OK:
            return;
        default:
            break;
        }
        if (isFakeSink)
            return;

        GstElement *sink = gst_element_factory_make("fakesink", NULL);
        sinkPad = gst_element_get_static_pad(sink, "sink");

        gst_bin_add(GST_BIN(thumbnailer->pipeline), sink);
        gst_element_set_state(sink, GST_STATE_PAUSED);

        isFakeSink = true;
    }
}

static GstFlowReturn new_preroll(GstAppSink *appsink, gpointer data)
{
    Q_UNUSED(appsink);

    Thumbnailer *thumbnailer = static_cast<Thumbnailer *>(data);
    QMutexLocker locker(&thumbnailer->mutex);
    thumbnailer->prerollAvailable = true;
    thumbnailer->condition.wakeOne();

    return GST_FLOW_OK;
}

}

extern "C" Q_DECL_EXPORT QImage createThumbnail(const QString &fileName, const QSize &requestedSize, bool crop)
{
    static bool initialized = false;
    if (!initialized) {
        gst_init(0, 0);
        initialized = true;
    }

    QImage image;
    Thumbnailer thumbnailer;

    thumbnailer.decodebin = gst_element_factory_make("uridecodebin", "decodebin");
    thumbnailer.transform = gst_element_factory_make("ffmpegcolorspace", "transform");
    thumbnailer.appsink = gst_element_factory_make("appsink", "sink");

    if (!thumbnailer.decodebin || !thumbnailer.transform || !thumbnailer.appsink)
        return image;

    GstAppSinkCallbacks callbacks;
    memset(&callbacks, 0, sizeof(GstAppSinkCallbacks));
    callbacks.new_preroll = new_preroll;
    gst_app_sink_set_callbacks(GST_APP_SINK(thumbnailer.appsink), &callbacks, &thumbnailer, 0);

    thumbnailer.pipeline = gst_pipeline_new(NULL);
    if (!thumbnailer.pipeline)
        return image;

    gst_bin_add_many(GST_BIN(thumbnailer.pipeline), thumbnailer.decodebin, thumbnailer.transform, thumbnailer.appsink, NULL);

    GstCaps *sinkCaps = gst_caps_new_simple(
                "video/x-raw-rgb",
                "bpp", G_TYPE_INT       , 32,
                "depth", G_TYPE_INT     , 24,
                "endianness", G_TYPE_INT, 4321,
                "red_mask", G_TYPE_INT  , 0x0000FF00,
                "green_mask", G_TYPE_INT, 0x00FF0000,
                "blue_mask", G_TYPE_INT , 0xFF000000,
                NULL);
    gst_app_sink_set_caps(GST_APP_SINK(thumbnailer.appsink), sinkCaps);

    gst_caps_unref(sinkCaps);

    gst_element_link_pads(thumbnailer.transform, "src", thumbnailer.appsink, "sink");
    g_signal_connect(thumbnailer.decodebin, "autoplug-continue", G_CALLBACK(decodebin_autoplug_continue), &thumbnailer);
    g_signal_connect(thumbnailer.decodebin, "pad-added", G_CALLBACK(decodebin_new_pad), &thumbnailer);

    g_object_set(G_OBJECT(thumbnailer.decodebin), "uri", (QLatin1String("file://") + fileName).toLocal8Bit().constData(), NULL);

    gst_element_set_state(thumbnailer.pipeline, GST_STATE_READY);

    GstState currentState;
    GstState pendingState;
    GstStateChangeReturn result = gst_element_get_state(
                thumbnailer.pipeline,
                &currentState,
                &pendingState,
                5 * GST_SECOND);
    if (result != GST_STATE_CHANGE_SUCCESS)
        return image;

    // Seek a little to hopefully capture something a little more meaningful than a fade
    // from black.  Seeking from the READY state is preferable because putting an element into the
    // paused state wil decode frames which will be thrown away on the seekb but not all decoders
    // support seaking in the READY state so we need to fallback to seeking after transitioning
    // into the paused state if this fails.
    const bool seekDone = gst_element_seek_simple(
                thumbnailer.pipeline,
                GST_FORMAT_TIME,
                GstSeekFlags(GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH),
                10 * GST_SECOND);

    // Transition to paused state, we should have received a new_preroll notification before this
    // returns if we were going to get one.
    gst_element_set_state(thumbnailer.pipeline, GST_STATE_PAUSED);
    result = gst_element_get_state(
                thumbnailer.pipeline,
                &currentState,
                &pendingState,
                5 * GST_SECOND);
    if (result == GST_STATE_CHANGE_SUCCESS) {
        bool prerollAvailable = false;
        if (!seekDone) {
            // Reset the preroll state that was set by pausing and attempt a seek.  We need to
            // ensure we get a preroll notification from gstreamer before calling
            // gst_app_sink_pull_preroll otherwise the call may never return.
            thumbnailer.prerollAvailable = false;
            gst_element_seek_simple(
                            thumbnailer.pipeline,
                            GST_FORMAT_TIME,
                            GstSeekFlags(GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH),
                            10 * GST_SECOND);
            QMutexLocker locker(&thumbnailer.mutex);
            if (!thumbnailer.prerollAvailable)
                thumbnailer.condition.wait(&thumbnailer.mutex, 5000);
            prerollAvailable = thumbnailer.prerollAvailable;
        } else {
            prerollAvailable = thumbnailer.prerollAvailable;
        }

        if (!prerollAvailable)
            return image;

        if (GstBuffer *buffer = gst_app_sink_pull_preroll(GST_APP_SINK(thumbnailer.appsink))) {
            GstStructure *structure = gst_caps_get_structure(GST_BUFFER_CAPS(buffer), 0);

            int width = 0;
            int height = 0;
            gst_structure_get_int(structure, "width", &width);
            gst_structure_get_int(structure, "height", &height);

            if (width > 0 && height > 0) {
                const int croppedWidth = crop ? height * requestedSize.width() / requestedSize.height() : width;
                const int croppedHeight = crop ? width * requestedSize.height() / requestedSize.width() : height;
                const int bytesPerLine = width * 4;
                QImage frame;

                if (croppedWidth < width) {
                    const uchar *data = GST_BUFFER_DATA(buffer) + (width - croppedWidth) * 2;
                    frame = QImage(data, croppedWidth, height, bytesPerLine, QImage::Format_RGB32);
                } else if (croppedHeight < height) {
                    const uchar *data = GST_BUFFER_DATA(buffer) + bytesPerLine * ((height - croppedHeight) / 2);
                    frame = QImage(data, width, croppedHeight, bytesPerLine, QImage::Format_RGB32);
                } else {
                    frame = QImage(GST_BUFFER_DATA(buffer), width, height, bytesPerLine, QImage::Format_RGB32);
                }

                image = frame.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                image.detach(); // Ensure a deep copy is made in the instance the image isn't scaled.
            }

            gst_buffer_unref(buffer);
        }
    }

    return image;
}
