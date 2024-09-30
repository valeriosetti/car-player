#include <stdlib.h>
#include <math.h>
#include "gstreamer.h"
#include "lvgl.h"

#define REFRESH_RATE_MS        50

GstBusSyncReply gst_bus_sync_handler(GstBus * bus, GstMessage * message, gpointer user_data)
// gboolean gst_bus_callback(GstBus *bus, GstMessage *message, gpointer user_data)
{
    (void) bus;
    (void) user_data;
    gstreamer_t *gst = (gstreamer_t *) user_data;

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(message, &err, &debug);
            LV_LOG_ERROR("Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);
            break;
        }

        case GST_MESSAGE_ELEMENT: {
            const GstStructure *message_struct = gst_message_get_structure(message);
            const gchar *name = gst_structure_get_name(message_struct);
            if (strcmp(name, "spectrum") == 0) {
                const GValue *magnitudes, *mag;
                magnitudes = gst_structure_get_value(message_struct, "magnitude");
                for (guint i = 0; i < gst_value_list_get_size(magnitudes); i++) {
                    mag = gst_value_list_get_value (magnitudes, i);
                    gst->magnitudes[i] = (int) g_value_get_float(mag);
                }
                if (gst->spectrum_cb != NULL) {
                    gst->spectrum_cb(gst->spectrum_cb_ctx, gst->magnitudes);
                }
            }
            break;
        }

        case GST_MESSAGE_EOS:
            LV_LOG_WARN("EOS");
            gstreamer_stop(gst);
            break;

        default:
            // Unhandled message
            break;
    }

    return TRUE;
}

void pad_added_handler(GstElement *self, GstPad *new_pad, gpointer user_data)
{
    gstreamer_t *gst = (gstreamer_t *) user_data;
    GstPad *sink_pad = gst_element_get_static_pad(gst->audioconvert, "sink");
    GstPadLinkReturn ret;
    (void) self;

    if (gst_pad_is_linked(sink_pad)) {
        LV_LOG_ERROR("Sink pad is already linked. Not reconnecting");
        goto exit;
    }

    ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret)) {
        LV_LOG_ERROR("Pad link failed\n");
    }

exit:
    gst_object_unref(sink_pad);
}

void unknown_type_callback(GstElement * bin, GstPad * pad, GstCaps * caps, gpointer udata)
{
    (void) bin;
    (void) pad;
    (void) caps;
    (void) udata;

    LV_LOG_ERROR("Unknown file type");
}

#define GST_ELEMENT_FACTORY_MAKE(name, dst) \
    dst = gst_element_factory_make(name, name); \
    if (!dst) { \
        LV_LOG_ERROR("GStreamer cannot create %s", name); \
        goto error; \
    }

gstreamer_t *gstreamer_init(gstreamer_type_t type, int spectrum_bands)
{
    gstreamer_t *gst = calloc(1, sizeof(gstreamer_t));
    if (gst == NULL) {
        LV_LOG_ERROR("Memory allocation failed");
        return NULL;
    }

    gst_init(NULL, NULL);

    gst->pipeline = gst_pipeline_new("music-pipeline");
    if (!gst->pipeline) {
        LV_LOG_ERROR("Pipeline creation failed");
        goto error;
    }

    if (type == GSTREAMER_TYPE_FILE_PLAYER) {
        GST_ELEMENT_FACTORY_MAKE("filesrc", gst->filesrc);
        GST_ELEMENT_FACTORY_MAKE("decodebin", gst->decodebin);
        GST_ELEMENT_FACTORY_MAKE("audioconvert", gst->audioconvert);
        GST_ELEMENT_FACTORY_MAKE("spectrum", gst->spectrum);
        GST_ELEMENT_FACTORY_MAKE("alsasink", gst->alsasink);

        if ((gst_bin_add(GST_BIN(gst->pipeline), gst->filesrc) &&
            gst_bin_add(GST_BIN(gst->pipeline), gst->decodebin) &&
            gst_bin_add(GST_BIN(gst->pipeline), gst->audioconvert) &&
            gst_bin_add(GST_BIN(gst->pipeline), gst->spectrum) &&
            gst_bin_add(GST_BIN(gst->pipeline), gst->alsasink)) == 0) {
            LV_LOG_ERROR("Error adding elements to the pipeline");
            goto error;
        }
        if ((gst_element_link(gst->filesrc, gst->decodebin) &&
            /* Do not link decodebin->convert. This is done when pipeline starts */
            gst_element_link(gst->audioconvert, gst->spectrum) &&
            gst_element_link(gst->spectrum, gst->alsasink)) == 0) {
            gst_object_unref(gst->pipeline);
            LV_LOG_ERROR("Elements could not be linked");
        }

        g_object_set(G_OBJECT(gst->alsasink), "device", "i2sout", NULL);
        g_signal_connect(gst->decodebin, "pad-added", G_CALLBACK(pad_added_handler), gst);
        g_signal_connect(gst->decodebin, "unknown-type", G_CALLBACK(unknown_type_callback), gst);
    } else { /* GSTREAMER_TYPE_RADIO_PLAYER */
        GST_ELEMENT_FACTORY_MAKE("alsasrc", gst->alsasrc);
        GST_ELEMENT_FACTORY_MAKE("audioconvert", gst->audioconvert);
        GST_ELEMENT_FACTORY_MAKE("spectrum", gst->spectrum);
        GST_ELEMENT_FACTORY_MAKE("alsasink", gst->alsasink);

        if ((gst_bin_add(GST_BIN(gst->pipeline), gst->alsasrc) &&
            gst_bin_add(GST_BIN(gst->pipeline), gst->audioconvert) &&
            gst_bin_add(GST_BIN(gst->pipeline), gst->spectrum) &&
            gst_bin_add(GST_BIN(gst->pipeline), gst->alsasink)) == 0) {
            LV_LOG_ERROR("Error adding elements to the pipeline");
            goto error;
        }
        if ((gst_element_link(gst->alsasrc, gst->audioconvert) &&
            gst_element_link(gst->audioconvert, gst->spectrum) &&
            gst_element_link(gst->spectrum, gst->alsasink)) == 0) {
            gst_object_unref(gst->pipeline);
            LV_LOG_ERROR("Elements could not be linked");
        }

        g_object_set(G_OBJECT(gst->alsasrc), "device", "i2sin", NULL);
        g_object_set(G_OBJECT(gst->alsasink), "device", "i2sout", NULL);
    }

    gstreamer_set_spectrum_bands(gst, spectrum_bands);
    gst->magnitudes = calloc(spectrum_bands, sizeof(int32_t));
    if (!gst->magnitudes) {
        LV_LOG_ERROR("Memeory allocation failed");
        goto error;
    }

    gst->bus = gst_element_get_bus(gst->pipeline);
    gst_bus_set_sync_handler(gst->bus, gst_bus_sync_handler, gst, NULL);
    gst_object_unref(gst->bus);

    return gst;

error:
    if (gst) {
        free(gst);
    }
    return NULL;
}

void gstreamer_set_spectrum_bands(gstreamer_t *gst, int count)
{
    g_object_set(G_OBJECT(gst->spectrum), "bands", count,
                 "threshold", GST_MAGNITUDES_MIN_VAL,
                 "interval", REFRESH_RATE_MS * GST_MSECOND, NULL);
}

int gstreamer_play_file(gstreamer_t *gst, char* path)
{
    GstStateChangeReturn ret;

    if ((gstreamer_get_state(gst) != GSTREAMER_STOPPED_PAUSED)) {
        gstreamer_stop(gst);
    }

    g_object_set(gst->filesrc, "location", path, NULL);
    ret = gst_element_set_state(gst->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LV_LOG_ERROR("Unable to start the pipeline");
        return -EIO;
    }
    gst->state = GSTREAMER_PLAYING;

    return 0;
}

int gstreamer_play(gstreamer_t *gst)
{
    GstStateChangeReturn ret;

    if ((gstreamer_get_state(gst) != GSTREAMER_STOPPED_PAUSED)) {
        gstreamer_stop(gst);
    }

    ret = gst_element_set_state(gst->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LV_LOG_ERROR("Unable to start the pipeline");
        return -EIO;
    }
    gst->state = GSTREAMER_PLAYING;

    return 0;
}

int gstreamer_stop(gstreamer_t *gst)
{
    GstStateChangeReturn ret = gst_element_set_state(gst->pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LV_LOG_ERROR("Unable to stop pipeline");
        return -EIO;
    }
    gst->state = GSTREAMER_STOPPED_PAUSED;

    return 0;
}

gstreamer_state_t gstreamer_get_state(gstreamer_t *gst)
{
    return gst->state;
}

void gstreamer_register_spectrum_cb(gstreamer_t *gst, gstreamer_spectrum_cb_t func, void* cb_ctx)
{
    gst->spectrum_cb = func;
    gst->spectrum_cb_ctx = cb_ctx;
}

int gstreamer_get_progress(gstreamer_t *gst, int *curr_sec, int *total_sec)
{
    gint64 pos, len;

    if (gst_element_query_position(gst->pipeline, GST_FORMAT_TIME, &pos)
        && gst_element_query_duration(gst->pipeline, GST_FORMAT_TIME, &len)) {
        *curr_sec = (int)(pos/1000000000);
        *total_sec = (int)(len/1000000000);
    } else {
        return -EINVAL;
    }

    return 0;
}
