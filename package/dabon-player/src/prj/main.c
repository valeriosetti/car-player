#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl.h"
#include "ui.h"
#include "screens.h"
#include "ui_utils.h"
#include "mpd_async.h"
#include "mpd_interface.h"

#define TIME_MS     1000 /* 1ms, to be used with usleep() */
#define TIME_S     (1000 * TIME_MS) /* 1s, to be used with usleep() */

#define MONITOR_H_RES       800
#define MONITOR_V_RES       480

static volatile int exit_program = 0;

static void hal_init(void)
{
    lv_display_t *display = lv_linux_drm_create();
    lv_linux_drm_set_file(display, "/dev/dri/card1", -1);

    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);

    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    lv_indev_set_group(touch, lv_group_get_default());
    lv_indev_set_display(touch, display);
}

static void *tick_thread(void *data) {
    (void) data;

    while(!exit_program) {
        usleep(5 * TIME_MS);
        lv_tick_inc(5);
    }

    return NULL;
}

void terminate_program(void)
{
    exit_program = 1;
}

int main(int argc, char **argv)
{
    uint32_t sleep_time;
    int ret;
    (void)argc;
    (void)argv;

    lv_init();
    hal_init();

    ret = mpd_if_connect_to_mpd();
    if (ret != 0) {
        LV_LOG_ERROR("Error: unable to connect to MPD");
        return ret;
    }

    pthread_t tick_pthread;

    ret = pthread_create(&tick_pthread, NULL, tick_thread, NULL);
    if (ret != 0) {
        LV_LOG_ERROR("Error: cannot create tick_pthread\n");
        return ret;
    }

    ui_init();
    post_init_customizations();

    mpd_async_start(0);

    while(!exit_program) {
        sleep_time = lv_timer_handler();
        usleep(sleep_time * TIME_MS);
    }

    mpd_async_stop();

    return 0;
}
