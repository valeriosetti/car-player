#include <stdlib.h>
#include "lvgl.h"
#include "spectrum.h"
#include "gstreamer.h"

#define REFRESH_RATE_MS        100

static void spectrum_event_cb(const lv_obj_class_t * class_p, lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    (void) class_p;

    if (code == LV_EVENT_DRAW_MAIN_BEGIN) {
        spectrum_t *spectrum = (spectrum_t *) obj;
        lv_layer_t *layer = lv_event_get_layer(e);
        lv_draw_line_dsc_t draw_dsc;

        lv_draw_line_dsc_init(&draw_dsc);
        draw_dsc.opa = LV_OPA_COVER;

        lv_area_t obj_coords;
        lv_point_t bottom_left;
        lv_obj_get_coords(obj, &obj_coords);
        bottom_left.x = obj_coords.x1;
        bottom_left.y = obj_coords.y2;
        int32_t obj_width = lv_obj_get_width(obj);
        int32_t element_width = obj_width / SPECTRUM_BARS_COUNTS;
        int32_t obj_height = lv_obj_get_height(obj);
        uint8_t color_mix;
        draw_dsc.width = element_width;

        for (int i = 0; i < SPECTRUM_BARS_COUNTS; i++) {
            color_mix = (uint8_t) ((spectrum->bars_heigth[i] * 255) / obj_height);
            draw_dsc.color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE),
                                             lv_palette_main(LV_PALETTE_GREEN),
                                             color_mix);
            draw_dsc.p1.x = (i * element_width) + bottom_left.x + element_width / 2;
            draw_dsc.p1.y = bottom_left.y;
            draw_dsc.p2.x = (i * element_width) + bottom_left.x + element_width / 2;
            draw_dsc.p2.y = bottom_left.y - spectrum->bars_heigth[i];
            lv_draw_line(layer, &draw_dsc);
        }
    }
}

static void refresh_timer_cb(lv_timer_t *timer)
{
    spectrum_t *spectrum = lv_timer_get_user_data(timer);

    lv_mutex_lock(&spectrum->lock);
    if (spectrum->data_updated) {
        spectrum->data_updated = false;
    } else {
        for (int i = 0; i < SPECTRUM_BARS_COUNTS; i++) {
            spectrum->bars_heigth[i] = GST_MAGNITUDES_MIN_VAL;
        }
    }
    lv_obj_invalidate((lv_obj_t *) spectrum);
    lv_mutex_unlock(&spectrum->lock);
}

static void spectrum_constructor_cb(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    (void) class_p;
    spectrum_t *spectrum = (spectrum_t *) obj;

    lv_mutex_init(&spectrum->lock);
    spectrum->refresh_timer = lv_timer_create(refresh_timer_cb, REFRESH_RATE_MS, spectrum);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
}

const lv_obj_class_t spectrum_class = {
    .base_class = &lv_obj_class,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(spectrum_t),
    .constructor_cb = spectrum_constructor_cb,
    .event_cb = spectrum_event_cb,
    .theme_inheritable = LV_OBJ_CLASS_THEME_INHERITABLE_TRUE,
    .name = "spectrum",
};

void spectrum_set_magnitudes(spectrum_t *spectrum, int32_t magnitudes[SPECTRUM_BARS_COUNTS])
{
    int32_t spectrum_height = lv_obj_get_height((lv_obj_t *) spectrum);

    lv_mutex_lock(&spectrum->lock);
    for (int i = 0; i < SPECTRUM_BARS_COUNTS; i++) {
        spectrum->bars_heigth[i] = lv_map(magnitudes[i],
                                          GST_MAGNITUDES_MIN_VAL, GST_MAGNITUDES_MAX_VAL,
                                          0, spectrum_height);
    }
    spectrum->data_updated = true;
    lv_mutex_unlock(&spectrum->lock);

    lv_timer_ready(spectrum->refresh_timer);
}

lv_obj_t *spectrum_create(lv_obj_t *parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(&spectrum_class, parent);
    lv_obj_class_init_obj(obj);

    return obj;
}
