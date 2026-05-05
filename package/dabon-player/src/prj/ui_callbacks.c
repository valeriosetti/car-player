#include <stdio.h>
#include <lvgl.h>
#include "actions.h"
#include "screens.h"
#include "main.h"
#include "ui_utils.h"
#include "mpd_interface.h"

void action_show_change_folder(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_screen_load_anim(objects.change_folder_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT,
                            250, 0, 0);
        update_directory_list();
    }
}

void action_return_to_main_screen(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_screen_load_anim(objects.main_screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                            250, 0, 0);
        update_song_list();
    }
}

void action_exit_program(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        terminate_program();
    }
}