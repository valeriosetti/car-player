#include <mpd/client.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ui_utils.h"
#include "screens.h"
#include "mpd_interface.h"
#include "utils.h"

static void clock_timer(lv_timer_t * timer)
{
    (void) timer;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    lv_label_set_text_fmt(objects.clock_label, "%02d:%02d",
                          tm->tm_hour, tm->tm_min);
}

void post_init_customizations(void)
{
    /* No line on the spectrogram */
    lv_obj_t* spectrogram_chart = objects.spectrogram_chart;
    lv_chart_set_div_line_count(spectrogram_chart, 0, 0);
    lv_obj_set_style_pad_all(spectrogram_chart, 0, LV_PART_MAIN);

    /* Start clock timer */
    lv_timer_create(clock_timer, 500, NULL);
}

void update_song_list(void)
{
    char *cur_path = mpd_if_get_current_path();
    unsigned int items_count;
    struct mpd_song **mpd_song_list = mpd_if_list_files(cur_path, &items_count);

    for (unsigned i = 0; i < items_count; i++) {
        const char *uri = mpd_song_get_uri(mpd_song_list[i]);
        LOG("adding song: %s", uri);
        lv_list_add_text(objects.song_list_table, uri);
    }
}

static void directory_clear_list(void)
{
    uint32_t child_count = lv_obj_get_child_count(objects.folder_list_table);
    struct mpd_directory *mpd_dir;

    TRACE();
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(objects.folder_list_table, i);
        mpd_dir = lv_obj_get_user_data(child);
        free(mpd_dir);
        lv_obj_delete(child);
    }
    TRACE();
}

static void directory_list_event(lv_event_t * e)
{
    TRACE();
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *obj = lv_event_get_target(e);
        struct mpd_directory *mpd_dir = lv_obj_get_user_data(obj);
        const char *uri = mpd_directory_get_path(mpd_dir);
        mpd_if_change_directory(uri);
    }
    TRACE();
}

void update_directory_list(void)
{
    char *cur_path = mpd_if_get_current_path();
    unsigned int items_count;
    lv_obj_t * list_btn;
    struct mpd_directory **mpd_dir_list = mpd_if_list_folders(cur_path, &items_count);

    TRACE();
    directory_clear_list();
    for (unsigned i = 0; i < items_count; i++) {
        const char *uri = mpd_directory_get_path(mpd_dir_list[i]);
        LOG("adding directory: %s", uri);
        list_btn = lv_list_add_button(objects.folder_list_table, NULL, uri);
        lv_obj_set_user_data(list_btn, mpd_dir_list[i]);
        lv_obj_add_event_cb(list_btn, directory_list_event,
                            LV_EVENT_CLICKED, NULL);
    }
    TRACE();
}
