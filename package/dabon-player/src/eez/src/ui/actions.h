#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_show_change_folder(lv_event_t * e);
extern void action_return_to_main_screen(lv_event_t * e);
extern void action_exit_program(lv_event_t * e);
extern void action_select_folder(lv_event_t * e);
extern void action_folder_list_event(lv_event_t * e);
extern void action_select_btn_event(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/