#include "lvgl.h"

#define SPECTRUM_BARS_COUNTS     128
#define SPECTRUM_MAX_VALUE      100

typedef struct {
    lv_table_t obj;
    int32_t bars_heigth[SPECTRUM_BARS_COUNTS];
    bool data_updated;
    lv_mutex_t lock;
    lv_timer_t *refresh_timer;
} spectrum_t;

lv_obj_t *spectrum_create(lv_obj_t *parent);
void spectrum_set_magnitudes(spectrum_t *spectrum, int32_t magnitudes[SPECTRUM_BARS_COUNTS]);
