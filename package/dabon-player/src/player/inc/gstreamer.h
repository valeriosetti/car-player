#ifndef GSTREAMER_H
#define GSTREAMER_H

#include <gst/gst.h>
#include "lvgl.h"

#define GST_MAGNITUDES_MIN_VAL       (-80) // dB
#define GST_MAGNITUDES_MAX_VAL       (0) // dB

typedef enum {
    GSTREAMER_TYPE_FILE_PLAYER,
    GSTREAMER_TYPE_RADIO_PLAYER,
} gstreamer_type_t;

typedef enum {
    GSTREAMER_STOPPED_PAUSED = 0,
    GSTREAMER_PLAYING,
} gstreamer_state_t;

// typedef void (*gstreamer_eos_cb_t)(void);
typedef void (*gstreamer_spectrum_cb_t)(void *ctx, int *magnitudes);

typedef struct {
    GstElement *pipeline,
               *filesrc, *decodebin, *audioconvert, *spectrum, *autoaudiosink,
               *alsasrc, *alsasink;
    GstBus *bus;
    // gstreamer_eos_cb_t *eos_cb;
    gstreamer_state_t state;

    gstreamer_spectrum_cb_t spectrum_cb;
    void *spectrum_cb_ctx;
    int32_t *magnitudes;
} gstreamer_t;

gstreamer_t *gstreamer_init(gstreamer_type_t type, int spectrum_bands);
void gstreamer_set_spectrum_bands(gstreamer_t *gst, int count);
int gstreamer_play(gstreamer_t *gst);
int gstreamer_play_file(gstreamer_t *gst, char* path);
int gstreamer_stop(gstreamer_t *gst);
gstreamer_state_t gstreamer_get_state(gstreamer_t *gst);
int gstreamer_get_progress(gstreamer_t *gst, int *curr_sec, int *total_sec);
// void gstreamer_register_eos_cb(gstreamer_t *gst, gstreamer_eos_cb_t *cb);
void gstreamer_register_spectrum_cb(gstreamer_t *gst, gstreamer_spectrum_cb_t func, void* cb_ctx);

#endif /* GSTREAMER_H */
