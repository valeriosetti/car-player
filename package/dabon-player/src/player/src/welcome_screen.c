#include <unistd.h>
#include "lvgl.h"
#include "welcome_screen.h"
#include "utils.h"
#include "main_selection_screen.h"

#if !defined(SKIP_WELCOME_SCREEN)

static lv_anim_t anim_zoom, anim_rotate;

static void zoom_anim_callback(void *data, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *) data;
    lv_image_set_scale(obj, v);
}

static void rotate_anim_callback(void *data, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *) data;
    lv_image_set_rotation(obj, v);
}
static void timer_cb(lv_timer_t *timer)
{
    (void) timer;
    main_selection_screen_start();
}

static void animation_completed(lv_anim_t *anim)
{
    (void) anim;

    /* One-shot timer to transition to the selection screen. */
    lv_timer_t *timer = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_set_repeat_count(timer, 1);
}
#endif /* !SKIP_WELCOME_SCREEN */

void welcome_screen_start()
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_screen_load(screen);

    lv_obj_t *img_obj = lv_image_create(screen);
    lv_image_set_src(img_obj, "A:DABon_symbol.png");
    lv_obj_align(img_obj, LV_ALIGN_OUT_LEFT_TOP, 0, 0);

#if !defined(SKIP_WELCOME_SCREEN)
    /* Zoom-in animation */
    lv_anim_init(&anim_zoom);
    lv_anim_set_var(&anim_zoom, img_obj);
    lv_anim_set_values(&anim_zoom, 0, 256);
    lv_anim_set_path_cb(&anim_zoom, lv_anim_path_linear);
    lv_anim_set_time(&anim_zoom, 1000);
    lv_anim_set_exec_cb(&anim_zoom, zoom_anim_callback);

    /* Rotation animation */
    lv_anim_init(&anim_rotate);
    lv_anim_set_var(&anim_rotate, img_obj);
    lv_anim_set_values(&anim_rotate, 0, 7200);
    lv_anim_set_path_cb(&anim_rotate, lv_anim_path_overshoot);
    lv_anim_set_time(&anim_rotate, 1000);
    lv_anim_set_exec_cb(&anim_rotate, rotate_anim_callback);
    lv_anim_set_deleted_cb(&anim_rotate, animation_completed);

    lv_anim_start(&anim_zoom);
    lv_anim_start(&anim_rotate);
#else
    main_selection_screen_start();
#endif
}
